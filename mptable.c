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
 *	  $Id: mptable.c,v 1.5 2001/12/11 17:03:45 davej Exp $
 */

#define MP_SIG				  0x5f504d5f	  /* _MP_ */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "mptable.h"

#ifdef linux
typedef unsigned int vm_offset_t;
#else
#include <machine/types.h>
#endif

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

typedef struct TABLE_ENTRY {
	u_char	type;
	u_char	length;
	char	name[32];
} tableEntry;

tableEntry basetableEntryTypes[] =
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
	void*	pap;
	u_char	length;
	u_char	spec_rev;
	u_char	checksum;
	u_char	mpfb1;
	u_char	mpfb2;
	u_char	mpfb3;
	u_char	mpfb4;
	u_char	mpfb5;
} mpfps_t;

/* MP Configuration Table Header */
typedef struct MPCTH {
	char	signature[4];
	u_short	base_table_length;
	u_char	spec_rev;
	u_char	checksum;
	u_char	oem_id[8];
	u_char	product_id[12];
	void*	oem_table_pointer;
	u_short	oem_table_size;
	u_short	entry_count;
	void*	apic_address;
	u_short	extended_table_length;
	u_char	extended_table_checksum;
	u_char	reserved;
} mpcth_t;

typedef struct PROCENTRY {
	u_char	type;
	u_char	apicID;
	u_char	apicVersion;
	u_char	cpuFlags;
	u_long	cpuSignature;
	u_long	featureFlags;
	u_long	reserved1;
	u_long	reserved2;
} ProcEntry;

static void apic_probe(vm_offset_t* paddr, int* where);

static int MPConfigTableHeader(void* pap);

static int readType(void);
static void seekEntry(vm_offset_t addr);
static void readEntry(void* entry, int size);

static void processorEntry(void);

/* global data */
int	pfd;			/* physical /dev/mem fd */

int	busses[16];
int	apics[16];

int	ncpu;
int	nbus;
int	napic;
int	nintr;
int	verbose;

int issmp(int *numcpu, int verb)
{
	vm_offset_t paddr;
	mpfps_t mpfps;
	int where;
	int defaultConfig;

	verbose=verb;
	/* open physical memory for access to MP structures */
	if ((pfd = open("/dev/mem", O_RDONLY)) < 0) {
		fprintf(stderr, "issmp(): /dev/mem: %s\n", strerror(errno));
		return -1;
	}

	/* probe for MP structures */
	apic_probe(&paddr, &where);
	if (where <= 0) {
		if(numcpu) *numcpu=1;
		return SMP_NO;
	}

	/* read in mpfps structure*/
	seekEntry(paddr);
	readEntry(&mpfps, sizeof(mpfps_t));

	/* check whether an MP config table exists */
	if (! (defaultConfig = mpfps.mpfb1))
		MPConfigTableHeader(mpfps.pap);

	if(numcpu) *numcpu=ncpu;
	return SMP_YES;
}

/*
 * set PHYSICAL address of MP floating pointer structure
 */
#define NEXT(X)		((X) += 4)
static void
apic_probe(vm_offset_t* paddr, int* where)
{
	/*
	 * c rewrite of apic_probe() by Jack F. Vogel
	 */

unsigned int x;
	u_short segment;
	vm_offset_t target;
	u_int buffer[ BIOS_SIZE / sizeof(int) ];

	/* search Extended Bios Data Area, if present */
	seekEntry((vm_offset_t)EBDA_POINTER);
	readEntry(&segment, 2);
	if (segment) {				/* search EBDA */
		target = (vm_offset_t)segment << 4;
		seekEntry(target);
		readEntry(buffer, ONE_KBYTE);

		for (x = 0; x < ONE_KBYTE / sizeof (unsigned int); NEXT(x)) {
			if (buffer[ x ] == MP_SIG) {
				*where = 1;
				*paddr = (x * sizeof(unsigned int)) + target;
				return;
			}
		}
	}

	/* read CMOS for real top of mem */
	seekEntry((vm_offset_t)TOPOFMEM_POINTER);
	readEntry(&segment, 2);
	--segment;										  /* less ONE_KBYTE */
	target = segment * 1024;
	seekEntry(target);
	readEntry(buffer, ONE_KBYTE);

	for (x = 0; x < ONE_KBYTE / sizeof (unsigned int); NEXT(x)) {
		if (buffer[ x ] == MP_SIG) {
			*where = 2;
			*paddr = (x * sizeof(unsigned int)) + target;
			return;
		}
	}

	/* we don't necessarily believe CMOS, check base of the last 1K of 640K */
	if (target != (DEFAULT_TOPOFMEM - 1024)) {
		target = (DEFAULT_TOPOFMEM - 1024);
		seekEntry(target);
		readEntry(buffer, ONE_KBYTE);

		for (x = 0; x < ONE_KBYTE / sizeof (unsigned int); NEXT(x)) {
			if (buffer[ x ] == MP_SIG) {
				*where = 3;
				*paddr = (x * sizeof(unsigned int)) + target;
				return;
			}
		}
	}

	/* search the BIOS */
	seekEntry(BIOS_BASE);
	readEntry(buffer, BIOS_SIZE);

	for (x = 0; x < BIOS_SIZE / sizeof(unsigned int); NEXT(x)) {
		if (buffer[ x ] == MP_SIG) {
			*where = 4;
			*paddr = (x * sizeof(unsigned int)) + BIOS_BASE;
			return;
		}
	}

	/* search the extended BIOS */
	seekEntry(BIOS_BASE2);
	readEntry(buffer, BIOS_SIZE);

	for (x = 0; x < BIOS_SIZE / sizeof(unsigned int); NEXT(x)) {
		if (buffer[ x ] == MP_SIG) {
			*where = 5;
			*paddr = (x * sizeof(unsigned int)) + BIOS_BASE2;
			return;
		}
	}

	/* search additional memory */
	target = GROPE_AREA1;
	seekEntry(target);
	readEntry(buffer, GROPE_SIZE);

	for (x = 0; x < GROPE_SIZE / sizeof(unsigned int); NEXT(x)) {
		if (buffer[ x ] == MP_SIG) {
			*where = 6;
			*paddr = (x * sizeof(unsigned int)) + GROPE_AREA1;
			return;
		}
	}

	target = GROPE_AREA2;
	seekEntry(target);
	readEntry(buffer, GROPE_SIZE);

	for (x = 0; x < GROPE_SIZE / sizeof(unsigned int); NEXT(x)) {
		if (buffer[ x ] == MP_SIG) {
			*where = 7;
			*paddr = (x * sizeof(unsigned int)) + GROPE_AREA2;
			return;
		}
	}

	*where = 0;
	*paddr = (vm_offset_t)0;
}

static int MPConfigTableHeader(void* pap)
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

	/* initialze tables */
	for (x = 0; x < 16; ++x)
		busses[ x ] = apics[ x ] = 0xff;

	ncpu = 0;
	nbus = 0;
	napic = 0;
	nintr = 0;

	/* process all the CPUs */
	if(verbose) printf("# Processors:\tAPIC ID\tVersion\tState\t\tFamily\tModel\tStep\tFlags\n");
	for (t = totalSize, c = count; c; c--) {
		if (readType() == 0)
			processorEntry();
		totalSize -= basetableEntryTypes[ 0 ].length;
	}
	
	return SMP_YES;
}

static int readType(void)
{
	u_char type;

	if (read(pfd, &type, sizeof(u_char)) != sizeof(u_char)) {
		perror("type read");
		exit(1);
	}

	if (lseek(pfd, -1, SEEK_CUR) < 0) {
		perror("type seek");
		exit(1);
	}

	return (int)type;
}

static void seekEntry(vm_offset_t addr)
{
	if (lseek(pfd, (off_t)addr, SEEK_SET) < 0) {
		perror("/dev/mem seek");
		exit(1);
	}
}

static void readEntry(void* entry, int size)
{
	if (read(pfd, entry, size) != size) {
		perror("readEntry");
		exit(1);
	}
}

static void processorEntry(void)
{
	ProcEntry entry;

	/* read it into local memory */
	readEntry(&entry, sizeof(entry));

	/* count it */
	++ncpu;

	if(verbose)
	{
		printf("#\t\t%2d", entry.apicID);
		printf("\t 0x%2x", entry.apicVersion);
		
		printf("\t %s, %s",
				(entry.cpuFlags & PROCENTRY_FLAG_BP) ? "BSP" : "AP",
				(entry.cpuFlags & PROCENTRY_FLAG_EN) ? "usable" : "unusable");
		
		printf("\t %ld\t %ld\t %ld",
				(entry.cpuSignature >> 8) & 0x0f,
				(entry.cpuSignature >> 4) & 0x0f,
				entry.cpuSignature & 0x0f);
		
		printf("\t 0x%04lx\n", entry.featureFlags);
	}
}

#ifdef STANDALONE
int main()
{
	int	numcpu, smp;

	smp=issmp(&numcpu, 1);
	printf("SMP: %d\nCPU: %d\n", smp, numcpu);
	return 0;
}
#endif
