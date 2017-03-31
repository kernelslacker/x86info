/*
 * Copyright (c) 1996, by Steve Passe
 * All rights reserved.
 *
 * hacked to make it work in userspace Linux by Ingo Molnar, same copyright
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. The name of the developer may NOT be used to endorse or promote products
 *	derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "mptable.h"
#include <x86info.h>

/* EBDA is @ 40:0e in real-mode terms */
#define EBDA_POINTER			0x040e		  /* location of EBDA pointer */

/* CMOS 'top of mem' is @ 40:13 in real-mode terms */
#define TOPOFMEM_POINTER		0x0413		  /* BIOS: base memory size */

#define DEFAULT_TOPOFMEM		0xa0000

#define BIOS_BASE			0xf0000
#define BIOS_BASE2			0xe0000
#define BIOS_SIZE			0x10000
#define ONE_KBYTE			1024

#define GROPE_AREA1			0x80000
#define GROPE_AREA2			0x90000
#define GROPE_SIZE			0x10000

#define PROCENTRY_FLAG_EN	0x01
#define PROCENTRY_FLAG_BP	0x02

/* global data */
static int pfd;			/* physical /dev/mem fd */

static int	busses[16];
static int	apics[16];

static int	ncpu;
static int	nbus;
static int	napic;
static int	nintr;
static int	silent;

/* MP Floating Pointer Structure */
struct mpfps {
	char	signature[4];
	u32	pap;
	u8	length;
	u8	spec_rev;
	u8	checksum;
	u8	mpfb1;
	u8	mpfb2;
	u8	mpfb3;
	u8	mpfb4;
	u8	mpfb5;
};

/* MP Configuration Table Header */
struct mpcth {
	char	signature[4];
	u16	base_table_length;
	u8	spec_rev;
	u8	checksum;
	u8	oem_id[8];
	u8	product_id[12];
	u32	oem_table_pointer;
	u16	oem_table_size;
	u16	entry_count;
	u32	apic_address;
	u16	extended_table_length;
	u8	extended_table_checksum;
	u8	reserved;
};

struct ProcEntry {
	u8	type;
	u8	apicID;
	u8	apicVersion;
	u8	cpuFlags;
	u32	cpuSignature;
	u32	featureFlags;
	u32	reserved1;
	u32	reserved2;
};


static void seekEntry(unsigned long addr)
{
	if (lseek(pfd, (off_t)addr, SEEK_SET) < 0) {
		perror("/dev/mem seek");
		exit(EXIT_FAILURE);
	}
}

static int readEntry(void* entry, int size)
{
	if (read(pfd, entry, size) != size) {
		perror("readEntry");
		return -1;
	}
	return 0;
}

static int readType(void)
{
	unsigned char type;

	if (read(pfd, &type, sizeof(unsigned char)) != sizeof(unsigned char)) {
		perror("type read");
		exit(EXIT_FAILURE);
	}

	if (lseek(pfd, -1, SEEK_CUR) < 0) {
		perror("type seek");
		exit(EXIT_FAILURE);
	}

	return (int)type;
}

static void processorEntry(void)
{
	struct ProcEntry entry;

	/* read it into local memory */
	if (readEntry(&entry, sizeof(entry)) < 0) {
		printf("Error reading processor entry\n");
		exit(EXIT_FAILURE);
	}

	/* count it */
	++ncpu;

	if (!silent) {
		int t, family, model;

		printf("#\t%2d", (int) entry.apicID);
		printf("\t 0x%2x", (unsigned int) entry.apicVersion);

		printf("\t %s, %s",
			(entry.cpuFlags & PROCENTRY_FLAG_BP) ? "BSP" : "AP",
			(entry.cpuFlags & PROCENTRY_FLAG_EN) ? "usable" : "unusable");

		t = (int) entry.cpuSignature;
		family = (t >> 8) & 0xf;
		model = (t >> 4) & 0xf;
		if (family == 0xf) {
			family += (t >> 20) & 0xff;
			model += (t >> 12) & 0xf0;
		}

		printf("\t %d\t %d\t %d", family, model, t & 0xf);
		printf("\t 0x%04x\n", entry.featureFlags);
	}
}


static int MPConfigTableHeader(u32 pap)
{
	unsigned long paddr;
	struct mpcth cth;
	int x;
	int count, c;

	if (pap == 0) {
		printf("MP Configuration Table Header MISSING!\n");
		return SMP_NO;
	}

	/* convert physical address to virtual address */
	paddr = (unsigned long) pap;

	/* read in cth structure */
	seekEntry(paddr);
	if(readEntry(&cth, sizeof(cth))) {
		printf("error reading MP Config table header structure\n");
		exit(EXIT_FAILURE);
	}

	count = cth.entry_count;

	/* initialize tables */
	for (x = 0; x < 16; ++x)
		busses[ x ] = apics[ x ] = 0xff;

	ncpu = 0;
	nbus = 0;
	napic = 0;
	nintr = 0;

	/* process all the CPUs */
	if (!silent)
		printf("MP Table:\n#\tAPIC ID\tVersion\tState\t\tFamily\tModel\tStep\tFlags\n");
	for (c = count; c; c--) {
		if (readType() == 0)
			processorEntry();
	}
	if (!silent)
		printf("\n");

	return SMP_YES;
}



/*
 * set PHYSICAL address of MP floating pointer structure
 */
#define NEXT(X)		((X) += 4)
static int apic_probe(unsigned long* paddr)
{
	unsigned int x;
	u16 segment;
	unsigned long target;
	unsigned int buffer[BIOS_SIZE];
	const char MP_SIG[]="_MP_";

	/* search Extended Bios Data Area, if present */
	seekEntry((unsigned long)EBDA_POINTER);
	if (readEntry(&segment, 2)) {
		printf("error reading EBDA pointer\n");
		exit(EXIT_FAILURE);
	}

	if (segment == 0)
		return 0;

	if (debug)
		printf("\nEBDA points to: %x\n", segment);

	target = (unsigned long)segment << 4;
	seekEntry(target);
	if (debug)
		printf("EBDA segment ptr: %lx\n", target);
	if (readEntry(buffer, ONE_KBYTE)) {
		printf("error reading 1K from %p\n", (void *)target);
		exit(EXIT_FAILURE);
	}


	for (x = 0; x < ONE_KBYTE / 4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + target;
			return 1;
		}
	}

	/* read CMOS for real top of mem */
	seekEntry((unsigned long)TOPOFMEM_POINTER);
	if (readEntry(&segment, 2)) {
		printf("error reading CMOS for real top of mem (%p)\n", (void *) TOPOFMEM_POINTER);
		exit(EXIT_FAILURE);
	}

	--segment;				/* less ONE_KBYTE */
	target = segment * 1024;
	seekEntry(target);
	if (readEntry(buffer, ONE_KBYTE)) {
		printf("error reading 1KB from %p\n", (void *)target);
		exit(EXIT_FAILURE);
	}


	for (x = 0; x < ONE_KBYTE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + target;
			return 2;
		}
	}

	/* we don't necessarily believe CMOS, check base of the last 1K of 640K */
	if (target != (DEFAULT_TOPOFMEM - 1024)) {
		target = (DEFAULT_TOPOFMEM - 1024);
		seekEntry(target);
		if (readEntry(buffer, ONE_KBYTE)) {
			printf("error reading DEFAULT_TOPOFMEM - 1024 from %p\n", (void *) target);
			exit(EXIT_FAILURE);
		}

		for (x = 0; x < ONE_KBYTE/4; NEXT(x)) {
			if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
				*paddr = (x*4) + target;
				return 3;
			}
		}
	}

	/* search the BIOS */
	seekEntry(BIOS_BASE);
	if (readEntry(buffer, BIOS_SIZE)) {
		printf("error reading BIOS_BASE from %p\n", (void *)BIOS_BASE);
		exit(EXIT_FAILURE);
	}


	for (x = 0; x < BIOS_SIZE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + BIOS_BASE;
			return 4;
		}
	}

	/* search the extended BIOS */
	seekEntry(BIOS_BASE2);
	if (readEntry(buffer, BIOS_SIZE)) {
		printf("error reading BIOS_BASE2 from %p\n", (void *)BIOS_BASE2);
		exit(EXIT_FAILURE);
	}


	for (x = 0; x < BIOS_SIZE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + BIOS_BASE2;
			return 4;
		}
	}

	/* search additional memory */
	target = GROPE_AREA1;
	seekEntry(target);
	if (readEntry(buffer, GROPE_SIZE)) {
		printf("error reading GROPE_AREA1 from %p\n", (void *)target);
		exit(EXIT_FAILURE);
	}


	for (x = 0; x < GROPE_SIZE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + GROPE_AREA1;
			return 5;
		}
	}

	target = GROPE_AREA2;
	seekEntry(target);
	if (readEntry(buffer, GROPE_SIZE)) {
		printf("error reading GROPE_AREA2 from %p\n", (void *)target);
		exit(EXIT_FAILURE);
	}


	for (x = 0; x < GROPE_SIZE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + GROPE_AREA2;
			return 6;
		}
	}

	*paddr = (unsigned long)0;
	return 0;
}


int enumerate_cpus(void)
{
	unsigned long paddr;
	struct mpfps mpfps;

	silent = 1;

	/* open physical memory for access to MP structures */
	if ((pfd = open("/dev/mem", O_RDONLY)) < 0) {
		fprintf(stderr, "enumerate_cpus(): /dev/mem: %s\n", strerror(errno));
		return -1;
	}

	/* probe for MP structures */
	if (apic_probe(&paddr) <= 0)
		return 1;

	/* read in mpfps structure*/
	seekEntry(paddr);
	if (readEntry(&mpfps, sizeof(struct mpfps))) {
		printf("error reading mpfpsfrom %p\n", (void *)paddr);
		exit(EXIT_FAILURE);
	}


	/* check whether an MP config table exists */
	if (!mpfps.mpfb1)
		if (MPConfigTableHeader(mpfps.pap) == SMP_YES)
			return ncpu;

	return 1;
}

void display_mptable(void)
{
	unsigned long paddr;
	struct mpfps mpfps;

	silent = 0;

	/* open physical memory for access to MP structures */
	if ((pfd = open("/dev/mem", O_RDONLY)) < 0) {
		fprintf(stderr, "%s(): /dev/mem: %s\n", __func__, strerror(errno));
		return;
	}

	/* probe for MP structures */
	if (apic_probe(&paddr) <= 0)
		return;

	/* read in mpfps structure*/
	seekEntry(paddr);
	if (readEntry(&mpfps, sizeof(struct mpfps))) {
		printf("error reading mpfps from %p\n", (void *)paddr);
		exit(EXIT_FAILURE);
	}


	/* parse an MP config table if it exists */
	if (!mpfps.mpfb1)
		(void) MPConfigTableHeader(mpfps.pap);
}
