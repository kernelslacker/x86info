/*
 *  $Id: cachesize.c,v 1.6 2003/01/18 16:59:42 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Intel specific parts
 *
 * References:
 *	http://developer.intel.com/
 *	http://microcodes.sourceforge.net/CPUID.htm
 *
 */

#include <stdio.h>
#include <string.h>
#include "../x86info.h"
#include "Intel.h"

struct _cache_table
{
	unsigned char descriptor;
	char type;
	int size;
	char *string;
};

#define LVL_1_INST 1
#define LVL_1_DATA 2
#define LVL_2      3
#define LVL_3      4
#define LVL_TRACE  5
#define INST_TLB   6
#define DATA_TLB   7

static struct _cache_table cache_table[] =
{
	{ 0x1,  INST_TLB,   32, "Instruction TLB: 4KB pages, 4-way associative, 32 entries" },
	{ 0x2,  INST_TLB,    2, "Instruction TLB: 4MB pages, fully associative, 2 entries" },
	{ 0x3,  DATA_TLB,   64, "Data TLB: 4KB pages, 4-way associative, 64 entries" },
	{ 0x4,  DATA_TLB,    8, "Data TLB: 4MB pages, 4-way associative, 8 entries" },
	{ 0x6,  LVL_1_INST,  8, "L1 Instruction cache:\n\tSize: 8KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x8,  LVL_1_INST, 16, "L1 Instruction cache:\n\tSize: 16KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0xa,  LVL_1_DATA,  8, "L1 Data cache:\n\tSize: 8KB\t2-way associative.\n\tline size=32 bytes." },
	{ 0xc,  LVL_1_DATA, 16, "L1 Data cache:\n\tSize: 16KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x22, LVL_3,     512, "L3 unified cache:\n\tSize: 512KB\t4-way associative.\n\tline size=64 bytes." },
	{ 0x23, LVL_3,    1024, "L3 unified cache:\n\tSize: 1MB\t8-way associative.\n\tline size=64 bytes." },
	{ 0x25, LVL_3,    2048, "L3 unified cache:\n\tSize: 2MB\t8-way associative.\n\tline size=64 bytes." },
	{ 0x29, LVL_3,    4096, "L3 unified cache:\n\tSize: 4MB\t8-way associative.\n\tline size=64 bytes." },
	{ 0x39, LVL_2,     128, "L2 unified cache:\n\tSize: 128KB\t4-way associative.\n\tline size=64 bytes." },
	{ 0x3c, LVL_2,     256, "L2 unified cache:\n\tSize: 256KB\t4-way associative.\n\tline size=64 bytes." },
	{ 0x41, LVL_2,     128, "L2 unified cache:\n\tSize: 128KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x42, LVL_2,     256, "L2 unified cache:\n\tSize: 256KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x43, LVL_2,     512, "L2 unified cache:\n\tSize: 512KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x44, LVL_2,    1024, "L2 unified cache:\n\tSize: 1MB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x45, LVL_2,    2048, "L2 unified cache:\n\tSize: 2MB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x50, INST_TLB,   64, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 64 entries." },
	{ 0x51, INST_TLB,  128, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 128 entries." },
	{ 0x52, INST_TLB,  256, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 256 entries." },
	{ 0x5b, DATA_TLB,   64, "Data TLB: 4KB or 4MB pages, fully associative, 64 entries." },
	{ 0x5c, DATA_TLB,  128, "Data TLB: 4KB or 4MB pages, fully associative, 128 entries." },
	{ 0x5d, DATA_TLB,  256, "Data TLB: 4KB or 4MB pages, fully associative, 256 entries." },
	{ 0x66, LVL_1_DATA,  8, "L1 Data cache:\n\tSize: 8KB\tSectored, 4-way associative.\n\tline size=64 bytes." },
	{ 0x67, LVL_1_DATA, 16, "L1 Data cache:\n\tSize: 16KB\tSectored, 4-way associative.\n\tline size=64 bytes." },
	{ 0x68, LVL_1_DATA, 32, "L1 Data cache:\n\tSize: 32KB\tSectored, 4-way associative.\n\tline size=64 bytes." },
	{ 0x70, LVL_TRACE,  12, "Instruction trace cache:\n\tSize: 12K uOps\t8-way associative." },
	{ 0x71, LVL_TRACE,  16, "Instruction trace cache:\n\tSize: 16K uOps\t8-way associative." },
	{ 0x72, LVL_TRACE,  32, "Instruction trace cache:\n\tSize: 32K uOps\t8-way associative." },
	{ 0x79, LVL_2,     128, "L2 unified cache:\n\tSize: 128KB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x7a, LVL_2,     256, "L2 unified cache:\n\tSize: 256KB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x7b, LVL_2,     512, "L2 unified cache:\n\tSize: 512KB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x7c, LVL_2,    1024, "L2 unified cache:\n\tSize: 1MB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x82, LVL_2,     256, "L2 unified cache:\n\tSize: 256KB\t8-way associative.\n\tline size=32 bytes." },
	{ 0x83, LVL_2,     512, "L2 unified cache:\n\tSize: 512KB\t8-way associative.\n\tline size=32 bytes." },
	{ 0x84, LVL_2,    1024, "L2 unified cache:\n\tSize: 1MB\t8-way associative.\n\tline size=32 bytes." },
	{ 0x85, LVL_2,    2048, "L2 unified cache:\n\tSize: 2MB\t8-way associative.\n\tline size=32 bytes." },
	{ 0, 0, 0, 0 }
};

/* Decode Intel TLB and cache info descriptors */
//TODO : Errata workaround. http://www.sandpile.org/post/msgs/20002736.htm
static void decode_Intel_cache (int des, struct cpudata *cpu)
{
	int k=0;

	//TODO: Add description to link-list in cpu->

	while (cache_table[k].descriptor != 0) {
//		printf ("descriptor: %x\n", des);
		if (cache_table[k].descriptor == des) {
			switch (cache_table[k].type) {
				case LVL_1_INST:
					cpu->cachesize_L1_I += cache_table[k].size;
					break;
				case LVL_1_DATA:
					cpu->cachesize_L1_D += cache_table[k].size;
					break;
				case LVL_2:
					cpu->cachesize_L2 += cache_table[k].size;
					break;
				case LVL_3:
					cpu->cachesize_L3 += cache_table[k].size;
					break;
				case LVL_TRACE:
					cpu->cachesize_trace += cache_table[k].size;
					break;
			}
		}
		k++;
	}
}


void decode_Intel_caches (struct cpudata *cpu)
{
	int i, j, n;
	long regs[4];
	unsigned char *dp = (unsigned char *)regs;

	if (cpu->maxi < 2)
		return;

	/* Decode TLB and cache info */
	cpuid (cpu->number, 2, &regs[0], &regs[1], &regs[2], &regs[3]);

	/* Number of times to iterate */
	n = regs[0] & 0xff;

	for (i=0 ; i<n ; i++) {
		cpuid(cpu->number, 2, &regs[0], &regs[1], &regs[2], &regs[3]);

		/* If bit 31 is set, this is an unknown format */
		for (j=0; j<3; j++)
			if (regs[j] < 0)
				regs[j] = 0;

		/* Byte 0 is level count, not a descriptor */
		for (j=1; j<16; j++)
			if (dp[j]!=0)
				decode_Intel_cache (dp[j], cpu);
	}
}

void show_Intel_caches(struct cpudata *cpu)
{
	if (!show_cacheinfo)
		return;

	if (cpu->cachesize_L1_I)
		printf ("L1I:%d\n", cpu->cachesize_L1_I);
	if (cpu->cachesize_L1_D)
		printf ("L1D:%d\n",cpu->cachesize_L1_D);
	if (cpu->cachesize_L2)
		printf ("L2:%d\n", cpu->cachesize_L2);
	if (cpu->cachesize_L3)
		printf ("L3:%d\n", cpu->cachesize_L3);
	if (cpu->cachesize_trace)
		printf ("Trace cache: %d\n", cpu->cachesize_trace);
	printf ("\n");
}

