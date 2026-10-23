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

/* State of one scan of the MP table. */
struct mp_scan {
	int fd;			/* physical /dev/mem fd */
	int ncpu;
	int silent;
};

#define mp_error(mp, ...)	do { if (!(mp)->silent) printf(__VA_ARGS__); } while (0)
#define mp_perror(mp, s)	do { if (!(mp)->silent) perror(s); } while (0)

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


static int seekEntry(struct mp_scan *mp, unsigned long addr)
{
	if (lseek(mp->fd, (off_t)addr, SEEK_SET) < 0) {
		mp_perror(mp, "/dev/mem seek");
		return -1;
	}
	return 0;
}

static int readEntry(struct mp_scan *mp, void* entry, int size)
{
	if (read(mp->fd, entry, size) != size) {
		mp_perror(mp, "readEntry");
		return -1;
	}
	return 0;
}

/* Seek to a physical address and read size bytes; 0 on success. */
static int readAt(struct mp_scan *mp, unsigned long addr, void *buf, int size)
{
	if (seekEntry(mp, addr))
		return -1;
	return readEntry(mp, buf, size);
}

static void processorEntry(struct mp_scan *mp, const unsigned char *raw)
{
	struct ProcEntry entry;

	memcpy(&entry, raw, sizeof(entry));

	/* count it */
	++mp->ncpu;

	if (!mp->silent) {
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

/* Size of each MP configuration table entry type, or 0 if unknown. */
static unsigned int entry_size(unsigned int type)
{
	switch (type) {
	case 0:			/* processor */
		return sizeof(struct ProcEntry);
	case 1:			/* bus */
	case 2:			/* I/O APIC */
	case 3:			/* I/O interrupt assignment */
	case 4:			/* local interrupt assignment */
		return 8;
	default:
		return 0;
	}
}

static unsigned int checksum(const unsigned char *p, size_t len)
{
	unsigned int sum = 0;

	while (len--)
		sum += *p++;
	return sum & 0xff;
}

static int MPConfigTableHeader(struct mp_scan *mp, u32 pap)
{
	unsigned long paddr;
	unsigned char *table;
	struct mpcth cth;
	unsigned int off;
	u16 size;
	int c;

	if (pap == 0) {
		mp_error(mp, "MP Configuration Table Header MISSING!\n");
		return SMP_NO;
	}

	paddr = (unsigned long) pap;

	/* read in cth structure */
	if (readAt(mp, paddr, &cth, sizeof(cth))) {
		mp_error(mp, "error reading MP Config table header structure\n");
		return SMP_NO;
	}

	if (memcmp(cth.signature, "PCMP", 4) ||
	    cth.base_table_length < sizeof(cth)) {
		mp_error(mp, "invalid MP Configuration Table Header\n");
		return SMP_NO;
	}

	size = cth.base_table_length;
	table = malloc(size);
	if (!table)
		return SMP_NO;
	if (readAt(mp, paddr, table, size)) {
		mp_error(mp, "error reading MP Config table\n");
		free(table);
		return SMP_NO;
	}
	if (checksum(table, size)) {
		mp_error(mp, "MP Configuration Table checksum is bad\n");
		free(table);
		return SMP_NO;
	}

	mp->ncpu = 0;

	/* process all the CPUs */
	if (!mp->silent)
		printf("MP Table:\n#\tAPIC ID\tVersion\tState\t\tFamily\tModel\tStep\tFlags\n");
	off = sizeof(cth);
	for (c = cth.entry_count; c; c--) {
		unsigned int len = off < size ? entry_size(table[off]) : 0;

		if (!len || size - off < len) {
			mp_error(mp, "MP Configuration Table entry is invalid\n");
			free(table);
			return SMP_NO;
		}
		if (table[off] == 0)
			processorEntry(mp, table + off);
		off += len;
	}
	free(table);
	if (!mp->silent)
		printf("\n");

	return SMP_YES;
}



/*
 * Scan a region for the MP floating pointer signature.
 * Returns 1 and sets *paddr if found, 0 if not, -1 if it can't be read.
 */
#define MP_ALIGN	16	/* the floating pointer is 16-byte aligned */
static int scan_for_mp(struct mp_scan *mp, const char *what,
		       unsigned long addr, int size, unsigned long *paddr)
{
	const char MP_SIG[] = "_MP_";
	static unsigned char buffer[BIOS_SIZE];
	int x;

	if (readAt(mp, addr, buffer, size)) {
		mp_error(mp, "error reading %s from %p\n", what, (void *)addr);
		return -1;
	}

	for (x = 0; x + (int)sizeof(struct mpfps) <= size; x += MP_ALIGN) {
		/* length is in 16-byte units and must be 1 */
		if (!strncmp((char *)&buffer[x], MP_SIG, 4) &&
		    buffer[x + 8] == 1 &&
		    !checksum(&buffer[x], sizeof(struct mpfps))) {
			*paddr = x + addr;
			return 1;
		}
	}
	return 0;
}

/*
 * set PHYSICAL address of MP floating pointer structure
 */
static int apic_probe(struct mp_scan *mp, unsigned long* paddr)
{
	u16 segment;
	unsigned long target;
	int ret;

	/* search Extended Bios Data Area, if present */
	if (readAt(mp, (unsigned long)EBDA_POINTER, &segment, 2)) {
		mp_error(mp, "error reading EBDA pointer\n");
		return 0;
	}

	if (segment != 0) {
		if (opts.debug)
			printf("\nEBDA points to: %x\n", segment);

		target = (unsigned long)segment << 4;
		if (opts.debug)
			printf("EBDA segment ptr: %lx\n", target);
		ret = scan_for_mp(mp, "1K at the EBDA", target, ONE_KBYTE, paddr);
		if (ret)
			return ret < 0 ? 0 : 1;
	}

	/* read CMOS for real top of mem */
	if (readAt(mp, (unsigned long)TOPOFMEM_POINTER, &segment, 2)) {
		mp_error(mp, "error reading CMOS for real top of mem (%p)\n", (void *) TOPOFMEM_POINTER);
		return 0;
	}

	target = 0;
	if (segment != 0) {
		--segment;			/* less ONE_KBYTE */
		target = segment * 1024;
		ret = scan_for_mp(mp, "1KB at the top of base memory", target, ONE_KBYTE, paddr);
		if (ret)
			return ret < 0 ? 0 : 2;
	}

	/* we don't necessarily believe CMOS, check base of the last 1K of 640K */
	if (target != (DEFAULT_TOPOFMEM - 1024)) {
		ret = scan_for_mp(mp, "DEFAULT_TOPOFMEM - 1024",
				  DEFAULT_TOPOFMEM - 1024, ONE_KBYTE, paddr);
		if (ret)
			return ret < 0 ? 0 : 3;
	}

	/* search the BIOS */
	ret = scan_for_mp(mp, "BIOS_BASE",BIOS_BASE, BIOS_SIZE, paddr);
	if (ret)
		return ret < 0 ? 0 : 4;

	/* search the extended BIOS */
	ret = scan_for_mp(mp, "BIOS_BASE2",BIOS_BASE2, BIOS_SIZE, paddr);
	if (ret)
		return ret < 0 ? 0 : 4;

	/* search additional memory */
	ret = scan_for_mp(mp, "GROPE_AREA1",GROPE_AREA1, GROPE_SIZE, paddr);
	if (ret)
		return ret < 0 ? 0 : 5;

	ret = scan_for_mp(mp, "GROPE_AREA2",GROPE_AREA2, GROPE_SIZE, paddr);
	if (ret)
		return ret < 0 ? 0 : 6;

	*paddr = (unsigned long)0;
	return 0;
}

/*
 * Find and parse the MP table in the physical memory image open on fd.
 * Returns 1 if an MP configuration table was parsed, 0 otherwise.
 * *ncpu is the number of processor entries found.
 */
int mptable_scan_fd(int fd, int quiet, int *ncpu)
{
	struct mp_scan mp = { .fd = fd, .silent = quiet };
	unsigned long paddr;
	struct mpfps mpfps;
	int ret = 0;

	*ncpu = 0;

	/* probe for MP structures */
	if (apic_probe(&mp, &paddr) <= 0)
		return 0;

	/* read in mpfps structure*/
	if (readAt(&mp, paddr, &mpfps, sizeof(struct mpfps))) {
		mp_error(&mp, "error reading mpfps from %p\n", (void *)paddr);
		return 0;
	}

	/* check whether an MP config table exists */
	if (!mpfps.mpfb1)
		ret = MPConfigTableHeader(&mp, mpfps.pap) == SMP_YES;

	*ncpu = ret ? mp.ncpu : 0;
	return ret;
}

/*
 * Returns -1 if /dev/mem can't be opened, else as mptable_scan_fd().
 */
static int read_mptable(int quiet, int *ncpu)
{
	int fd, ret;

	/* open physical memory for access to MP structures */
	if ((fd = open("/dev/mem", O_RDONLY)) < 0) {
		if (!quiet)
			fprintf(stderr, "mptable: /dev/mem: %s\n", strerror(errno));
		return -1;
	}

	ret = mptable_scan_fd(fd, quiet, ncpu);
	close(fd);
	return ret;
}

/* Result of the scan done by display_mptable(), if any. */
static struct {
	int valid;
	int ret;
	int ncpu;
} displayed;

int enumerate_cpus(void)
{
	int ret, ncpu = 0;

	if (displayed.valid) {
		ret = displayed.ret;
		ncpu = displayed.ncpu;
	} else {
		ret = read_mptable(1, &ncpu);
	}

	if (ret < 0)
		return -1;
	return ret ? ncpu : 1;
}

void display_mptable(void)
{
	displayed.ret = read_mptable(0, &displayed.ncpu);
	displayed.valid = 1;
}
