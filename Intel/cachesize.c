/*
 *  $Id: cachesize.c,v 1.14 2005/08/07 18:42:37 davej Exp $
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
	int size;
	char *string;
};

static struct _cache_table TRACE_cache_table[] =
{
	{ 0x70, 12, "Instruction trace cache:\n\tSize: 12K uOps\t8-way associative." },
	{ 0x71, 16, "Instruction trace cache:\n\tSize: 16K uOps\t8-way associative." },
	{ 0x72, 32, "Instruction trace cache:\n\tSize: 32K uOps\t8-way associative." },
	{ 0x73, 64, "Instruction trace cache:\n\tSize: 64K uOps\t8-way associative." },
	{ 0, 0, 0 }
};

static struct _cache_table L1I_cache_table[] =
{
	{ 0x6,  8, "L1 Instruction cache:\n\tSize: 8KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x8,  16, "L1 Instruction cache:\n\tSize: 16KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x30, 32,	"L1 Instruction cache:\n\tSize 32KB\t8-way associative.\n\tline size=64 bytes." },
	{ 0, 0, 0 }
};

static struct _cache_table L1D_cache_table[] =
{
	{ 0xa,  8, "L1 Data cache:\n\tSize: 8KB\t2-way associative.\n\tline size=32 bytes." },
	{ 0xc,  16, "L1 Data cache:\n\tSize: 16KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x2c, 32, "L1 Data cache:\n\tSize: 32KB\t8-way associative.\n\tline size=64 bytes." },
	{ 0x60, 16, "L1 Data cache:\n\tSize: 16KB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x66, 8, "L1 Data cache:\n\tSize: 8KB\tSectored, 4-way associative.\n\tline size=64 bytes." },
	{ 0x67, 16, "L1 Data cache:\n\tSize: 16KB\tSectored, 4-way associative.\n\tline size=64 bytes." },
	{ 0x68, 32, "L1 Data cache:\n\tSize: 32KB\tSectored, 4-way associative.\n\tline size=64 bytes." },
	{ 0, 0, 0 }
};

static struct _cache_table L2_cache_table[] =
{
	{ 0x39, 128, "L2 unified cache:\n\tSize: 128KB\t4-way associative.\n\tline size=64 bytes." },
	{ 0x3a, 192, "L2 unified cache:\n\tSize: 192KB\t6-way associative.\n\tline size=64 bytes." },
	{ 0x3b, 128, "L2 unified cache:\n\tSize: 128KB\t2-way associative.\n\tline size=64 bytes." },
	{ 0x3c, 256, "L2 unified cache:\n\tSize: 256KB\t4-way associative.\n\tline size=64 bytes." },
	{ 0x3d, 384, "L2 unified cache:\n\tSize: 384KB\t6-way associative.\n\tline size=64 bytes." },
	{ 0x3e, 512, "L2 unified cache:\n\tSize: 512KB\t4-way associative.\n\tline size=64 bytes." },
	{ 0x41, 128, "L2 unified cache:\n\tSize: 128KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x42, 256, "L2 unified cache:\n\tSize: 256KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x43, 512, "L2 unified cache:\n\tSize: 512KB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x44, 1024, "L2 unified cache:\n\tSize: 1MB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x45, 2048, "L2 unified cache:\n\tSize: 2MB\t4-way associative.\n\tline size=32 bytes." },
	{ 0x46, 4096, "L3 unified cache:\n\tSize: 4MB\t4-way associative.\n\tline size=64 bytes." },
	{ 0x47, 8192, "L3 unified cache:\n\tSize: 8MB\t8-way associative.\n\tline size=64 bytes." },
	{ 0x49, 4096, "L3 unified cache:\n\tSize: 4MB\t16-way associative.\n\tline size=64 bytes." },
	{ 0x4a, 6144, "L3 unified cache:\n\tSize: 6MB\t12-way associative.\n\tline size=64 bytes." },
	{ 0x4b, 8192, "L3 unified cache:\n\tSize: 8MB\t16-way associative.\n\tline size=64 bytes." },
	{ 0x4c, 12288, "L3 unified cache:\n\tSize: 12MB\t12-way associative.\n\tline size=64 bytes." },
	{ 0x4d, 16384, "L3 unified cache:\n\tSize: 16MB\t16-way associative.\n\tline size=64 bytes." },
	{ 0x78, 1024, "L3 unified cache:\n\tSize: 1MB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x79, 128, "L2 unified cache:\n\tSize: 128KB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x7a, 256, "L2 unified cache:\n\tSize: 256KB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x7b, 512, "L2 unified cache:\n\tSize: 512KB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x7c, 1024, "L2 unified cache:\n\tSize: 1MB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x7d, 2048, "L2 unified cache:\n\tSize: 2MB\tSectored, 8-way associative.\n\tline size=64 bytes." },
	{ 0x7f, 512, "L2 unified cache:\n\tSize: 512KB\t2-way associative.\n\tline size=64 bytes." },
	{ 0x82, 256, "L2 unified cache:\n\tSize: 256KB\t8-way associative.\n\tline size=32 bytes." },
	{ 0x83, 512, "L2 unified cache:\n\tSize: 512KB\t8-way associative.\n\tline size=32 bytes." },
	{ 0x84, 1024, "L2 unified cache:\n\tSize: 1MB\t8-way associative.\n\tline size=32 bytes." },
	{ 0x85, 2048, "L2 unified cache:\n\tSize: 2MB\t8-way associative.\n\tline size=32 bytes." },
	{ 0x86, 512, "L2 unified cache:\n\tSize: 512KB\t4-way associative\n\tline size=64 bytes." },
	{ 0x87, 1024, "L2 unified cache:\n\tSize: 1MB\t8-way associative.\n\tline size=64 bytes." },
	{ 0, 0, 0 }
};

static struct _cache_table L3_cache_table[] =
{
	{ 0x22, 512, "L3 unified cache:\n\tSize: 512KB\t4-way associative.\n\tline size=64 bytes." },
	{ 0x23, 1024, "L3 unified cache:\n\tSize: 1MB\t8-way associative.\n\tline size=64 bytes." },
	{ 0x25, 2048, "L3 unified cache:\n\tSize: 2MB\t8-way associative.\n\tline size=64 bytes." },
	{ 0x29, 4096, "L3 unified cache:\n\tSize: 4MB\t8-way associative.\n\tline size=64 bytes." },
	{ 0, 0, 0 }
};

static struct _cache_table ITLB_cache_table[] =
{
	{ 0x1,  32, "Instruction TLB: 4KB pages, 4-way associative, 32 entries" },
	{ 0x2,  2, "Instruction TLB: 4MB pages, fully associative, 2 entries" },
	{ 0x50, 64, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 64 entries." },
	{ 0x51, 128, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 128 entries." },
	{ 0x52, 256, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 256 entries." },
	{ 0xb0, 128, "Instruction TLB: 4K pages, 4-way associative, 128 entries." },
	{ 0, 0, 0 }
};

static struct _cache_table DTLB_cache_table[] =
{
	{ 0x3,  64, "Data TLB: 4KB pages, 4-way associative, 64 entries" },
	{ 0x4,  8, "Data TLB: 4MB pages, 4-way associative, 8 entries" },
	{ 0x5b, 64, "Data TLB: 4KB or 4MB pages, fully associative, 64 entries." },
	{ 0x5c, 128, "Data TLB: 4KB or 4MB pages, fully associative, 128 entries." },
	{ 0x5d, 256, "Data TLB: 4KB or 4MB pages, fully associative, 256 entries." },
	{ 0xb3, 128, "Data TLB: 4K pages, 4-way associative, 128 entries." },
	{ 0, 0, 0 }
};

static char found_unknown=0;
static char unknown_array[256];

/* Decode Intel TLB and cache info descriptors */
//TODO : Errata workaround. http://www.sandpile.org/post/msgs/20002736.htm
static void decode_Intel_cache (int des, struct cpudata *cpu, int output,
			struct _cache_table *table)
{
	int k=0;

	//TODO: Add description to link-list in cpu->
	//TODO: print unknown descriptors.

	while (table[k].descriptor != 0) {
		if (table[k].descriptor == des) {

			if (table == TRACE_cache_table)
				cpu->cachesize_trace += table[k].size;

			if (table == L1I_cache_table)
				cpu->cachesize_L1_I += table[k].size;

			if (table == L1D_cache_table)
				cpu->cachesize_L1_D += table[k].size;

			if (table == L2_cache_table)
				cpu->cachesize_L2 += table[k].size;

			if (table == L3_cache_table)
				cpu->cachesize_L3 += table[k].size;

			if (output)
				printf ("%s\n", table[k].string);
		}
		k++;
	}
	if (table[k].descriptor == 0) {
		unknown_array[des]=1;
		found_unknown=1;
	}
}

static void decode_cache(struct cpudata *cpu, struct _cache_table *table, int output)
{
	int i, j, n;
	long regs[4];
	unsigned char *dp = (unsigned char *)regs;

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
				decode_Intel_cache (dp[j], cpu, output, table);
	}
}


void decode_Intel_caches (struct cpudata *cpu, int output)
{
	int i;

	if (cpu->maxi < 2)
		return;

	memset (&unknown_array, 0, sizeof(unknown_array));

	decode_cache (cpu, TRACE_cache_table, output);
	decode_cache (cpu, L1I_cache_table, output);
	decode_cache (cpu, L1D_cache_table, output);
	decode_cache (cpu, L2_cache_table, output);
	decode_cache (cpu, L3_cache_table, output);
	decode_cache (cpu, ITLB_cache_table, output);

	if (found_unknown == 0)
		return;

	printf ("Found unknown cache descriptors: ");

	for (i=0; i<256; i++) {
		if (unknown_array[i]==1)
			printf ("%d ", i);
	}
	printf ("\n");
	decode_cache (cpu, DTLB_cache_table, output);
}

/*
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
*/

