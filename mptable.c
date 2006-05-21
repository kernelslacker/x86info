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
 *
 *	  $Id: mptable.c,v 1.8 2005/04/25 20:54:12 davej Exp $
 */


#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "mptable.h"
#include "x86info.h"

typedef unsigned long vm_offset_t;

/* EBDA is @ 40:0e in real-mode terms */
#define EBDA_POINTER			0x040e		  /* location of EBDA pointer */

/* CMOS 'top of mem' is @ 40:13 in real-mode terms */
#define TOPOFMEM_POINTER		0x0413		  /* BIOS: base memory size */

#define DEFAULT_TOPOFMEM		0xa0000

#define BIOS_BASE			   0xf0000
#define BIOS_BASE2			  0xe0000
#define BIOS_SIZE			   0x10000
#define ONE_KBYTE			   1024

#define GROPE_AREA1			 0x80000
#define GROPE_AREA2			 0x90000
#define GROPE_SIZE			  0x10000

#define PROCENTRY_FLAG_EN	   0x01
#define PROCENTRY_FLAG_BP	   0x02
#define IOAPICENTRY_FLAG_EN	 0x01

#define MAXPNSTR				132

/* global data */
static int pfd;			/* physical /dev/mem fd */

static int	busses[16];
static int	apics[16];

static int	ncpu;
static int	nbus;
static int	napic;
static int	nintr;
static int verbose_mp;

typedef struct TABLE_ENTRY {
	u8	type;
	u8	length;
	char	name[32];
} tableEntry;

static tableEntry basetableEntryTypes[] =
{
	{ 0, 20, "Processor" },
	{ 1,  8, "Bus" },
	{ 2,  8, "I/O APIC" },
	{ 3,  8, "I/O INT" },
	{ 4,  8, "Local INT" }
};

/* MP Floating Pointer Structure */
typedef struct MPFPS {
	char	signature[4];
	u32		pap;
	u8	length;
	u8	spec_rev;
	u8	checksum;
	u8	mpfb1;
	u8	mpfb2;
	u8	mpfb3;
	u8	mpfb4;
	u8	mpfb5;
} mpfps_t;

/* MP Configuration Table Header */
typedef struct MPCTH {
	char	signature[4];
	u16	base_table_length;
	u8	spec_rev;
	u8	checksum;
	u8	oem_id[8];
	u8	product_id[12];
	u32 oem_table_pointer;
	u16	oem_table_size;
	u16	entry_count;
	u32	apic_address;
	u16	extended_table_length;
	u8	extended_table_checksum;
	u8	reserved;
} mpcth_t;

typedef struct PROCENTRY {
	u8	type;
	u8	apicID;
	u8	apicVersion;
	u8	cpuFlags;
	u32	cpuSignature;
	u32	featureFlags;
	u32	reserved1;
	u32	reserved2;
} ProcEntry;


static void seekEntry(vm_offset_t addr)
{
	if (lseek(pfd, (off_t)addr, SEEK_SET) < 0) {
		perror("/dev/mem seek");
		exit(EXIT_FAILURE);
	}
}

static void readEntry(void* entry, int size)
{
	if (read(pfd, entry, size) != size) {
		perror("readEntry");
		exit(EXIT_FAILURE);
	}
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
	ProcEntry entry;

	/* read it into local memory */
	readEntry(&entry, sizeof(entry));

	/* count it */
	++ncpu;

	if (verbose_mp) {
		printf("#\t%2d", (int) entry.apicID);
		printf("\t 0x%2x", (unsigned int) entry.apicVersion);

		printf("\t %s, %s",
				(entry.cpuFlags & PROCENTRY_FLAG_BP) ? "BSP" : "AP",
				(entry.cpuFlags & PROCENTRY_FLAG_EN) ? "usable" : "unusable");

		printf("\t %d\t %d\t %d",
				(int) (entry.cpuSignature >> 8) & 0x0f,
				(int) (entry.cpuSignature >> 4) & 0x0f,
				(int)  entry.cpuSignature & 0x0f);

		printf("\t 0x%04x\n", entry.featureFlags);
	}
}


static int MPConfigTableHeader(u32 pap)
{
	vm_offset_t paddr;
	mpcth_t	 cth;
	int x;
	int totalSize, t;
	int count, c;

	if (pap == 0) {
		printf("MP Configuration Table Header MISSING!\n");
		return SMP_NO;
	}

	/* convert physical address to virtual address */
	paddr = (vm_offset_t)pap;

	/* read in cth structure */
	seekEntry(paddr);
	readEntry(&cth, sizeof(cth));

	totalSize = cth.base_table_length - sizeof(struct MPCTH);
	count = cth.entry_count;

	/* initialize tables */
	for (x = 0; x < 16; ++x)
		busses[ x ] = apics[ x ] = 0xff;

	ncpu = 0;
	nbus = 0;
	napic = 0;
	nintr = 0;

	/* process all the CPUs */
	if (verbose_mp)
		printf("MP Table:\n#\tAPIC ID\tVersion\tState\t\tFamily\tModel\tStep\tFlags\n");
	for (t = totalSize, c = count; c; c--) {
		if (readType() == 0)
			processorEntry();
		totalSize -= basetableEntryTypes[ 0 ].length;
	}
	if (verbose_mp)
		printf ("\n");

	return SMP_YES;
}



/*
 * set PHYSICAL address of MP floating pointer structure
 */
#define NEXT(X)		((X) += 4)
static int apic_probe(vm_offset_t* paddr)
{
	unsigned int x;
	u16 segment;
	vm_offset_t target;
	unsigned int buffer[BIOS_SIZE];
	const char MP_SIG[]="_MP_";

	/* search Extended Bios Data Area, if present */
	seekEntry((vm_offset_t)EBDA_POINTER);
	readEntry(&segment, 2);
	if (segment) {				/* search EBDA */
		target = (vm_offset_t)segment << 4;
		seekEntry(target);
		readEntry(buffer, ONE_KBYTE);

		for (x = 0; x < ONE_KBYTE / 4; NEXT(x)) {
			if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
				*paddr = (x*4) + target;
				return 1;
			}
		}
	}

	/* read CMOS for real top of mem */
	seekEntry((vm_offset_t)TOPOFMEM_POINTER);
	readEntry(&segment, 2);
	--segment;						  /* less ONE_KBYTE */
	target = segment * 1024;
	seekEntry(target);
	readEntry(buffer, ONE_KBYTE);

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
		readEntry(buffer, ONE_KBYTE);

		for (x = 0; x < ONE_KBYTE/4; NEXT(x)) {
			if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
				*paddr = (x*4) + target;
				return 3;
			}
		}
	}

	/* search the BIOS */
	seekEntry(BIOS_BASE);
	readEntry(buffer, BIOS_SIZE);

	for (x = 0; x < BIOS_SIZE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + BIOS_BASE;
			return 4;
		}
	}

	/* search the extended BIOS */
	seekEntry(BIOS_BASE2);
	readEntry(buffer, BIOS_SIZE);

	for (x = 0; x < BIOS_SIZE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + BIOS_BASE2;
			return 4;
		}
	}

	/* search additional memory */
	target = GROPE_AREA1;
	seekEntry(target);
	readEntry(buffer, GROPE_SIZE);

	for (x = 0; x < GROPE_SIZE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + GROPE_AREA1;
			return 5;
		}
	}

	target = GROPE_AREA2;
	seekEntry(target);
	readEntry(buffer, GROPE_SIZE);

	for (x = 0; x < GROPE_SIZE/4; NEXT(x)) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = (x*4) + GROPE_AREA2;
			return 6;
		}
	}

	*paddr = (vm_offset_t)0;
	return 0;
}


int enumerate_cpus(void)
{
	vm_offset_t paddr;
	mpfps_t mpfps;

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
	readEntry(&mpfps, sizeof(mpfps_t));

	/* check whether an MP config table exists */
	if (!mpfps.mpfb1)
		if (MPConfigTableHeader(mpfps.pap) == SMP_YES)
			return ncpu;

	return 1;
}

int issmp(int verb)
{
	vm_offset_t paddr;
	mpfps_t mpfps;

	verbose_mp=verb;
	/* open physical memory for access to MP structures */
	if ((pfd = open("/dev/mem", O_RDONLY)) < 0) {
		fprintf(stderr, "issmp(): /dev/mem: %s\n", strerror(errno));
		return -1;
	}

	/* probe for MP structures */
	if (apic_probe(&paddr) <= 0)
		return SMP_NO;

	/* read in mpfps structure*/
	seekEntry(paddr);
	readEntry(&mpfps, sizeof(mpfps_t));

	/* check whether an MP config table exists */
	if (!mpfps.mpfb1)
		return MPConfigTableHeader(mpfps.pap);

	return SMP_NO;
}

#ifdef STANDALONE
int main()
{
	int	numcpu, smp;
	numcpu = enumerate_cpus();
	smp=issmp(1);
	printf("SMP: %d\nCPU: %d\n", smp, numcpu);
	return 0;
}
#endif
