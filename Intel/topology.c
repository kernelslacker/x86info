/*
 *  (C) 2008 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel multicore/multithread determination.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "../x86info.h"
#include "Intel.h"

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */

static int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}


/* Determine the width of the bit field that can represent the value item. */
static unsigned int find_maskwidth(unsigned int item)
{
	unsigned int MaskWidth = 0;

	MaskWidth = fls(item)-1;
	return MaskWidth;
}

/* Extract the subset of bit field from the 8-bit value FullID.  It returns the 8-bit sub ID value */
static unsigned char GetSubID(unsigned char FullID, unsigned char MaxSubIDvalue, unsigned char ShiftCount)
{
	unsigned int MaskWidth, MaskBits, SubID;

	MaskWidth = find_maskwidth(MaxSubIDvalue);
	MaskBits = ((unsigned char) (0xff << ShiftCount)) ^ ((unsigned char) (0xff << (ShiftCount + MaskWidth))) ;
	SubID = (FullID & MaskBits) >> ShiftCount;
	return SubID;
}


void show_intel_topology(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
	unsigned int MaxLPPerCore;
	unsigned int smt_id, core_id, package_id;
	unsigned int shift;

	cpuid(cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);

	/* Find the max number of logical processors per physical package. */
	if (cpu->flags_edx & (1 << 28))
		cpu->nr_logical = (ebx >> 16) & 0xff;
	else
		cpu->nr_logical = 1;

	/* Find the max number of processor cores per physical processor package. */
	if (cpu->maxi >= 4) {
		cpuid4(cpu->number, 0, &eax, &ebx, &ecx, &edx);
		if (eax & 0x1f)
			cpu->nr_cores = ((eax >> 26) + 1);
	} else {
		cpu->nr_cores = 1;
	}

	//MaxLPPerCore = MaxLogicalProcPerPhysicalProc() / MaxCorePerPhysicalProc();
	MaxLPPerCore = cpu->nr_logical / cpu->nr_cores;
	printf("Number of cores per physical package=%d\n", cpu->nr_cores);		// 8
	printf("Number of logical processors per socket=%d\n", cpu->nr_logical);	// 16
	printf("Number of logical processors per core=%d\n", MaxLPPerCore);		// 2

	/* test for hyperthreading. */
	if (cpu->flags_edx & (1 << 28)) {
		// test that there's more logical processor IDs with the same physical ID
		// than the number of cores per physical processors.
	}

	smt_id = GetSubID(cpu->apicid, cpu->nr_logical, 0);
	shift = find_maskwidth(cpu->nr_logical);
	core_id = GetSubID(cpu->apicid, cpu->nr_cores, shift);
	shift += find_maskwidth(cpu->nr_cores);
	package_id = GetSubID(cpu->apicid, MaxLPPerCore, shift);

	printf("APIC ID: 0x%x\t", cpu->apicid);
	printf("Package: %u  Core: %u   SMT ID %u\n", package_id, core_id, smt_id);
}
