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

static int	ncpu;
static int	silent;

#define mp_error(...)	do { if (!silent) printf(__VA_ARGS__); } while (0)
#define mp_perror(s)	do { if (!silent) perror(s); } while (0)

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

/* These are read straight from /dev/mem: the layout must have no padding. */
_Static_assert(sizeof(struct mpfps) == 16, "mpfps layout");
_Static_assert(sizeof(struct mpcth) == 44, "mpcth layout");
_Static_assert(sizeof(struct ProcEntry) == 20, "ProcEntry layout");


static int seekEntry(unsigned long addr)
{
	if (lseek(pfd, (off_t)addr, SEEK_SET) < 0) {
		mp_perror("/dev/mem seek");
		return -1;
	}
	return 0;
}

static int readEntry(void* entry, int size)
{
	if (read(pfd, entry, size) != size) {
		mp_perror("readEntry");
		return -1;
	}
	return 0;
}

/* Seek to a physical address and read size bytes; 0 on success. */
static int readAt(unsigned long addr, void *buf, int size)
{
	if (seekEntry(addr))
		return -1;
	return readEntry(buf, size);
}

/* Returns the type of the next entry without consuming it, or -1. */
static int readType(void)
{
	unsigned char type;

	if (read(pfd, &type, sizeof(unsigned char)) != sizeof(unsigned char)) {
		mp_perror("type read");
		return -1;
	}

	if (lseek(pfd, -1, SEEK_CUR) < 0) {
		mp_perror("type seek");
		return -1;
	}

	return (int)type;
}

static int processorEntry(void)
{
	struct ProcEntry entry;

	/* read it into local memory */
	if (readEntry(&entry, sizeof(entry)) < 0) {
		mp_error("Error reading processor entry\n");
		return -1;
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
	return 0;
}


static int MPConfigTableHeader(u32 pap)
{
	unsigned long paddr;
	struct mpcth cth;
	int count, c;

	if (pap == 0) {
		mp_error("MP Configuration Table Header MISSING!\n");
		return SMP_NO;
	}

	/* convert physical address to virtual address */
	paddr = (unsigned long) pap;

	/* read in cth structure */
	if (readAt(paddr, &cth, sizeof(cth))) {
		mp_error("error reading MP Config table header structure\n");
		return SMP_NO;
	}

	count = cth.entry_count;

	ncpu = 0;

	/* process all the CPUs */
	if (!silent)
		printf("MP Table:\n#\tAPIC ID\tVersion\tState\t\tFamily\tModel\tStep\tFlags\n");
	for (c = count; c; c--) {
		int type = readType();
		if (type < 0)
			return SMP_NO;
		if (type == 0) {
			if (processorEntry())
				return SMP_NO;
		} else {
			/* Non-CPU entries (bus, I/O APIC, interrupt) are 8 bytes each;
			 * skip them so the file position advances past every entry. */
			if (lseek(pfd, 8, SEEK_CUR) < 0) {
				mp_perror("entry seek");
				return SMP_NO;
			}
		}
	}
	if (!silent)
		printf("\n");

	return SMP_YES;
}



/*
 * Scan a region for the MP floating pointer signature.
 * Returns 1 and sets *paddr if found, 0 if not, -1 if it can't be read.
 */
#define MP_ALIGN	16	/* the floating pointer is 16-byte aligned */
static int scan_for_mp(const char *what, unsigned long addr, int size,
		       unsigned long *paddr)
{
	const char MP_SIG[] = "_MP_";
	unsigned char buffer[BIOS_SIZE];
	int x;

	if (readAt(addr, buffer, size)) {
		mp_error("error reading %s from %p\n", what, (void *)addr);
		return -1;
	}

	for (x = 0; x < size; x += MP_ALIGN) {
		if (!strncmp((char *)&buffer[x], MP_SIG, 4)) {
			*paddr = x + addr;
			return 1;
		}
	}
	return 0;
}

/*
 * set PHYSICAL address of MP floating pointer structure
 */
static int apic_probe(unsigned long* paddr)
{
	u16 segment;
	unsigned long target;
	int ret;

	/* search Extended Bios Data Area, if present */
	if (readAt((unsigned long)EBDA_POINTER, &segment, 2)) {
		mp_error("error reading EBDA pointer\n");
		return 0;
	}

	if (segment != 0) {
		if (opts.debug)
			printf("\nEBDA points to: %x\n", segment);

		target = (unsigned long)segment << 4;
		if (opts.debug)
			printf("EBDA segment ptr: %lx\n", target);
		ret = scan_for_mp("1K at the EBDA", target, ONE_KBYTE, paddr);
		if (ret)
			return ret < 0 ? 0 : 1;
	}

	/* read CMOS for real top of mem */
	if (readAt((unsigned long)TOPOFMEM_POINTER, &segment, 2)) {
		mp_error("error reading CMOS for real top of mem (%p)\n", (void *) TOPOFMEM_POINTER);
		return 0;
	}

	target = 0;
	if (segment != 0) {
		--segment;			/* less ONE_KBYTE */
		target = segment * 1024;
		ret = scan_for_mp("1KB at the top of base memory", target, ONE_KBYTE, paddr);
		if (ret)
			return ret < 0 ? 0 : 2;
	}

	/* we don't necessarily believe CMOS, check base of the last 1K of 640K */
	if (target != (DEFAULT_TOPOFMEM - 1024)) {
		ret = scan_for_mp("DEFAULT_TOPOFMEM - 1024", DEFAULT_TOPOFMEM - 1024,
				  ONE_KBYTE, paddr);
		if (ret)
			return ret < 0 ? 0 : 3;
	}

	/* search the BIOS */
	ret = scan_for_mp("BIOS_BASE", BIOS_BASE, BIOS_SIZE, paddr);
	if (ret)
		return ret < 0 ? 0 : 4;

	/* search the extended BIOS */
	ret = scan_for_mp("BIOS_BASE2", BIOS_BASE2, BIOS_SIZE, paddr);
	if (ret)
		return ret < 0 ? 0 : 4;

	/* search additional memory */
	ret = scan_for_mp("GROPE_AREA1", GROPE_AREA1, GROPE_SIZE, paddr);
	if (ret)
		return ret < 0 ? 0 : 5;

	ret = scan_for_mp("GROPE_AREA2", GROPE_AREA2, GROPE_SIZE, paddr);
	if (ret)
		return ret < 0 ? 0 : 6;

	*paddr = (unsigned long)0;
	return 0;
}

/*
 * Find and parse the MP table.
 * Returns -1 if /dev/mem can't be opened, 1 if an MP configuration table
 * was parsed, 0 otherwise.
 */
static int read_mptable(int quiet)
{
	unsigned long paddr;
	struct mpfps mpfps;
	int ret = 0;

	silent = quiet;

	/* open physical memory for access to MP structures */
	if ((pfd = open("/dev/mem", O_RDONLY)) < 0) {
		if (!quiet)
			fprintf(stderr, "mptable: /dev/mem: %s\n", strerror(errno));
		return -1;
	}

	/* probe for MP structures */
	if (apic_probe(&paddr) <= 0) {
		close(pfd);
		return 0;
	}

	/* read in mpfps structure*/
	if (readAt(paddr, &mpfps, sizeof(struct mpfps))) {
		mp_error("error reading mpfps from %p\n", (void *)paddr);
		close(pfd);
		return 0;
	}

	/* check whether an MP config table exists */
	if (!mpfps.mpfb1)
		ret = MPConfigTableHeader(mpfps.pap) == SMP_YES;

	close(pfd);
	return ret;
}

int enumerate_cpus(void)
{
	int ret = read_mptable(1);

	if (ret < 0)
		return -1;
	return ret ? ncpu : 1;
}

void display_mptable(void)
{
	read_mptable(0);
}
