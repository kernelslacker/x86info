/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Intel specific parts
 *
 * References:
 *	http://www.intel.com/Assets/PDF/appnote/241618.pdf
 */

#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include "intel.h"

#define MB(x)   ((x) * 1024)

struct _cache_table
{
	const unsigned char descriptor;
	const int size;
	const char *string;
};

static const struct _cache_table TRACE_cache_table[] =
{
	{ 0x70, 12, "Instruction trace cache: 12K uOps, 8-way associative." },
	{ 0x71, 16, "Instruction trace cache: 16K uOps, 8-way associative." },
	{ 0x72, 32, "Instruction trace cache: 32K uOps, 8-way associative." },
	{ 0x73, 64, "Instruction trace cache: 64K uOps, 8-way associative." },
	{ 0, 0, NULL }
};

static const struct _cache_table L1I_cache_table[] =
{
	{ 0x6,   8, "L1 Instruction cache: 8KB, 4-way associative. 32 byte line size." },
	{ 0x8,  16, "L1 Instruction cache: 16KB, 4-way associative. 32 byte line size." },
	{ 0x9,  32, "L1 Instruction cache: 32KB, 4-way associative. 64 byte line size." },
	{ 0x30, 32, "L1 Instruction cache: 32KB, 8-way associative. 64 byte line size." },
	{ 0, 0, NULL }
};

static const struct _cache_table L1D_cache_table[] =
{
	{ 0xa,   8, "L1 Data cache: 8KB, 2-way associative. 32 byte line size." },
	{ 0xc,  16, "L1 Data cache: 16KB, 4-way associative. 32 byte line size." },
	{ 0xd,  16, "L1 Data cache: 16KB, 4-way associative. 64 byte line size. ECC." },
	{ 0xe,  24, "L1 Data cache: 24KB, 6-way associative. 64 byte line size. ECC." },
	{ 0x2c, 32, "L1 Data cache: 32KB, 8-way associative. 64 byte line size." },
	{ 0x60, 16, "L1 Data cache: 16KB, sectored, 8-way associative. 64 byte line size." },
	{ 0x66 , 8, "L1 Data cache: 8KB, sectored, 4-way associative. 64 byte line size." },
	{ 0x67, 16, "L1 Data cache: 16KB, sectored, 4-way associative. 64 byte line size." },
	{ 0x68, 32, "L1 Data cache: 32KB, sectored, 4-way associative. 64 byte line size." },
	{ 0, 0, NULL }
};

static const struct _cache_table L2_cache_table[] =
{
	{ 0x21,  256, "L2 (MLC): 256KB, 8-way associative. 64 byte line size." },
	{ 0x39,  128, "L2 cache: 128KB, 4-way associative. Sectored. 64 byte line size." },
/*	{ 0x3a,  192, "L2 cache: 192KB, 6-way associative. Sectored. 64 byte line size." },
	{ 0x3b,  128, "L2 cache: 128KB, 2-way associative. Sectored. 64 byte line size." },
	{ 0x3c,  256, "L2 cache: 256KB, 4-way associative. Sectored. 64 byte line size." },
	{ 0x3d,  384, "L2 cache: 384KB, 6-way associative. Sectored. 64 byte line size." },
	{ 0x3e,  512, "L2 cache: 512KB, 4-way associative. Sectored. 64 byte line size." },
	{ 0x3f, 256, "L2 cache: 256KB, 2-way associative. Sectored. 64 byte line size." },	*/
/* 3a->3f are no longer listed. */
	{ 0x41,  128, "L2 cache: 128KB, 4-way associative. 32 byte line size." },
	{ 0x42,  256, "L2 cache: 256KB, 4-way associative. 32 byte line size." },
	{ 0x43,  512, "L2 cache: 512KB, 4-way associative. 32 byte line size." },
	{ 0x44, MB(1), "L2 cache: 1MB, 4-way associative. 32 byte line size." },
	{ 0x45, MB(2), "L2 cache: 2MB, 4-way associative. 32 byte line size." },
	{ 0x48, MB(3), "L2 cache: 3MB, 12-way associative. 64 byte line size. Unified on-die." },
	{ 0x4e, MB(6), "L2 cache: 6MB, 24-way set associative, 64-byte line size." },
	{ 0x79,  128, "L2 cache: 128KB, sectored, 8-way associative. 64 byte line size." },
	{ 0x7a,  256, "L2 cache: 256KB, sectored, 8-way associative. 64 byte line size." },
	{ 0x7b,  512, "L2 cache: 512KB, sectored, 8-way associative. 64 byte line size." },
	{ 0x7c, MB(1), "L2 cache: 1MB, sectored, 8-way associative. 64 byte line size." },
	{ 0x7d, MB(2), "L2 cache: 2MB, 8-way associative. 64 byte line size." },
	{ 0x7f,  512, "L2 cache: 512KB, 2-way associative. 64 byte line size." },
	{ 0x80,  512, "L2 cache: 512KB, 8-way associative. 64 byte line size." },
	{ 0x82,  256, "L2 cache: 256KB, 8-way associative. 32 byte line size." },
	{ 0x83,  512, "L2 cache: 512KB, 8-way associative. 32 byte line size." },
	{ 0x84, MB(1), "L2 cache: 1MB, 8-way associative. 32 byte line size." },
	{ 0x85, MB(2), "L2 cache: 2MB, 8-way associative. 32 byte line size." },
	{ 0x86,  512, "L2 cache: 512KB, 4-way associative. 64 byte line size." },
	{ 0x87, MB(1), "L2 cache: 1MB, 8-way associative. 64 byte line size." },
	{ 0, 0, NULL }
};

static const struct _cache_table L2L3_cache_table[] =
{
	{ 0x46,  MB(4), "L2 cache: 4MB, 4-way associative. 64 byte line size." },
	{ 0x47,  MB(8), "L2 cache: 8MB, 8-way associative. 64 byte line size." },
	{ 0x48,  MB(3), "L2 cache: 3MB, 12-way associative. 64 byte line size." },
	{ 0x49,  MB(4), "L2 cache: 4MB, 16-way associative. 64 byte line size." },
	{ 0x4a,  MB(6), "L2 cache: 6MB, 12-way associative. 64 byte line size." },
	{ 0x4b,  MB(8), "L2 cache: 8MB, 16-way associative. 64 byte line size." },
	{ 0x4c, MB(12), "L2 cache: 12MB, 12-way associative. 64 byte line size." },
	{ 0x4d, MB(16), "L2 cache: 16MB, 16-way associative. 64 byte line size." },
	{ 0x78,  MB(1), "L2 cache: 1MB, sectored, 8-way associative. 64 byte line size." },
	{ 0, 0, NULL }
};
static const struct _cache_table L3L2_cache_table[] =
{
	{ 0x46,  MB(4), "L3 cache: 4MB, 4-way associative. 64 byte line size." },
	{ 0x47,  MB(8), "L3 cache: 8MB, 8-way associative. 64 byte line size." },
	{ 0x49,  MB(4), "L3 cache: 4MB, 16-way associative. 64 byte line size." },
	{ 0x4a,  MB(6), "L3 cache: 6MB, 12-way associative. 64 byte line size." },
	{ 0x4b,  MB(8), "L3 cache: 8MB, 16-way associative. 64 byte line size." },
	{ 0x4c, MB(12), "L3 cache: 12MB, 12-way associative. 64 byte line size." },
	{ 0x4d, MB(16), "L3 cache: 16MB, 16-way associative. 64 byte line size." },
	{ 0x78,  MB(1), "L3 cache: 1MB, sectored, 8-way associative. 64 byte line size." },
	{ 0, 0, NULL }
};

static const struct _cache_table L3_cache_table[] =
{
	{ 0x22,  512, "L3 cache: 512KB, 4-way associative. 64 byte line size." },
	{ 0x23, MB(1), "L3 cache: 1MB, 8-way associative. 64 byte line size." },
	{ 0x25, MB(2), "L3 cache: 2MB, 8-way associative. 64 byte line size." },
	{ 0x29, MB(4), "L3 cache: 4MB, 8-way associative. 64 byte line size." },
	{ 0xd0,   512, "L3 cache: 512KB, 4-way associative. 64 byte line size." },
	{ 0xd1,  MB(1), "L3 cache: 1MB, 4-way associative. 64 byte line size." },
	{ 0xd2,  MB(2), "L3 cache: 2MB, 4-way associative. 64 byte line size." },
	{ 0xd6,  MB(1), "L3 cache: 1MB, 8-way associative. 64 byte line size." },
	{ 0xd7,  MB(2), "L3 cache: 2MB, 8-way associative. 64 byte line size." },
	{ 0xd8,  MB(4), "L3 cache: 4MB, 8-way associative. 64 byte line size." },
	{ 0xdc,  MB(2), "L3 cache: 2MB, 12-way associative. 64 byte line size." },
	{ 0xdd,  MB(4), "L3 cache: 4MB, 12-way associative. 64 byte line size." },
	{ 0xe2,  MB(2), "L3 cache: 2MB, 16-way associative. 64 byte line size." },
	{ 0xe3,  MB(4), "L3 cache: 4MB, 16-way associative. 64 byte line size." },
	{ 0xe4,  MB(8), "L3 cache: 8MB, 16-way associative. 64 byte line size." },
	{ 0xea, MB(12), "L3 cache: 12MB, 24-way associative. 64 byte line size." },
	{ 0xeb, MB(18), "L3 cache: 18MB, 24-way associative. 64 byte line size." },
	{ 0xec, MB(24), "L3 cache: 24MB, 24-way associative. 64 byte line size." },
	{ 0xde,  MB(8), "L3 cache: 8MB, 12-way associative. 64 byte line size." },
	{ 0, 0, NULL }
};

static const struct _cache_table ITLB_cache_table[] =
{
	{ 0x1,   32, "Instruction TLB: 4KB pages, 4-way associative, 32 entries" },
	{ 0x2,    2, "Instruction TLB: 4MB pages, fully associative, 2 entries" },
	{ 0xb,    4, "Instruction TLB: 4MB pages, 4-way set associative, 4 entries" },
	{ 0x4f,  32, "Instruction TLB: 4KB pages, 32 entries" },
	{ 0x50,  64, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 64 entries." },
	{ 0x51, 128, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 128 entries." },
	{ 0x52, 256, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 256 entries." },
	{ 0x55,	  7, "Instruction TLB: 2MB or 4MB pages, fully associative, 7 entries" },
	{ 0x61,	 48, "Instruction TLB: 4K pages, fully associative, 48 entries" },
	{ 0x76,   8, "Instruction TLB: 2M/4M pages, fully associative, 8 entries" },
	{ 0xb0, 128, "Instruction TLB: 4K pages, 4-way associative, 128 entries." },
	{ 0xb1,   4, "Instruction TLB: 4x 4MB page entries, or 8x 2MB pages entries, 4-way associative" },
	{ 0xb2,  64, "Instruction TLB: 4K pages, 4-way associative, 64 entries." },
	{ 0xb5,  64, "Instruction TLB: 4K pages, 8-way associative, 64 entries" },
	{ 0xb6, 128, "Instruction TLB: 4K pages, 8-way associative, 128 entries" },
	{ 0, 0, NULL }
};

static const struct _cache_table DTLB_cache_table[] =
{
	{ 0x3,   64, "Data TLB: 4KB pages, 4-way associative, 64 entries" },
	{ 0x4,    8, "Data TLB: 4MB pages, 4-way associative, 8 entries" },
	{ 0x5,   32, "Data TLB: 4MB pages, 4-way associative, 32 entries" },
	{ 0x56,	 16, "Data TLB0: 4MB pages, 4-way associative, 16 entries" },
	{ 0x57,	 16, "Data TLB0: 4KB pages, 4-way associative, 16 entries" },
	{ 0x59,  16, "Data TLB0: 4KB pages, fully associative, 16 entries" },
	{ 0x5a,  32, "Data TLB0: 2MB or 4MB pages, 4-way set associative, 32 entries" },
	{ 0x5b,  64, "Data TLB: 4KB or 4MB pages, fully associative, 64 entries." },
	{ 0x5c, 128, "Data TLB: 4KB or 4MB pages, fully associative, 128 entries." },
	{ 0x5d, 256, "Data TLB: 4KB or 4MB pages, fully associative, 256 entries." },
	{ 0x63,   4, "Data TLB: 1GB pages, 4-way set associative, 4 entries" },
	{ 0x64, 512, "Data TLB: 4KB pages, 4-way associative, 512 entries" },
	{ 0xa0,  32, "Data TLB: 4KB pages, full associative, 32 entries" },
	{ 0xb3, 128, "Data TLB: 4KB pages, 4-way associative, 128 entries" },
	{ 0xb4, 256, "Data TLB1: 4KB pages, 4-way associative, 256 entries" },
	{ 0xba,  64, "Data TLB1: 4KB pages, 4-way associative, 64 entries" },
	{ 0xc0,   8, "Data TLB: 4KB or 4MB pages, 4-way associative, 8 entries" },
	{ 0xc1,1024, "Shared L2 TLB: 4KB/2MB pages, 8-way associative, 1024 entries" },
	{ 0xc2,  16, "Data TLB: 2MB/4MB pages, 4-way associative, 16 entries" },
	{ 0xc3,1024, "Shared L2 TLB: 4KB/2MB pages, 6-way associative, 1536 entries" },
	{ 0xc4,  32, "Data TLB: 2MB/4MB pages, 4-way associative, 32 entries" },
	{ 0xca, 512, "Shared L2 TLB: 4KB pages, 4-way associative, 512 entries" },
	{ 0, 0, NULL }
};

static const struct _cache_table prefetch_table[] =
{
	{0xf0, 64, "64 byte prefetching."},
	{0xf1, 64, "128 byte prefetching."},
	{ 0, 0, NULL }
};

static unsigned char found_unknown=0;
static unsigned char found_general=0;
static unsigned char unknown_array[256];

/* Decode Intel TLB and cache info descriptors */
//TODO : Errata workaround. http://www.sandpile.org/post/msgs/20002736.htm
static void decode_intel_cache(int des, struct cpudata *cpu, int output,
			const struct _cache_table *table)
{
	int k = 0;
	int found = 0;

	/* "No 2nd-level cache or, if processor contains a valid 2nd-level
	   cache, no 3rd-level cache". Skip this pointless entry.*/
	if (des == 0x40)
		return;

	if (des == 0xff) {
		found_general = 1;
		return;
	}

	//TODO: Add description to link-list in cpu->

	while ((table[k].descriptor != 0) && (found == 0)) {
		if (table[k].descriptor == des) {

			if (table == TRACE_cache_table)
				cpu->cachesize_trace += table[k].size;

			if (table == L1I_cache_table)
				cpu->cachesize_L1_I += table[k].size;

			if (table == L1D_cache_table)
				cpu->cachesize_L1_D += table[k].size;

			if (table == L2_cache_table)
				cpu->cachesize_L2 += table[k].size;
			if (table == L2L3_cache_table)
				cpu->cachesize_L2 += table[k].size;

			if (table == L3_cache_table)
				cpu->cachesize_L3 += table[k].size;
			if (table == L3L2_cache_table)
				cpu->cachesize_L3 += table[k].size;

			if (output)
				printf(" %s\n", table[k].string);
			found = 1;
		}
		k++;
	}

	if ((found == 0) && (unknown_array[des] == 0)) {
		unknown_array[des] = 1;
		found_unknown++;
	}
}

static void decode_cache(struct cpudata *cpu, const struct _cache_table *table, int output)
{
	unsigned int i, j, n;
	unsigned int regs[4];

	/* Decode TLB and cache info */
	cpuid(cpu->number, 2, &regs[0], &regs[1], &regs[2], &regs[3]);

	/* Number of times to iterate */
	n = regs[0] & 0xff;

	for (i=0; i<n; i++) {
		cpuid(cpu->number, 2, &regs[0], &regs[1], &regs[2], &regs[3]);

		/* If bit 31 is set, this is an unknown format */
		for (j=0; j<3; j++)
			if (regs[j] & 0x80000000)
				regs[j] = 0;

		/* Byte 0 is level count, not a descriptor */
		for (j=1; j<16; j++) {
			unsigned char val = regs[j / 4] >> (unsigned int)(8 * (j % 4));
			if (val)
				decode_intel_cache(val, cpu, output, table);
		}
	}
}

static void clean_unknowns(const struct _cache_table *table)
{
	int j=0;

	while (table[j].descriptor != 0) {
		int des;

		des = table[j++].descriptor;
		if (unknown_array[des] == 1) {
			unknown_array[des] = 0;
			found_unknown--;
		}
	}
}

static const char *cache_types[32] =
{
	NULL,
	"Data Cache",
	"Instruction Cache",
	"Unified Cache",
};

static void decode_general_cache(struct cpudata *cpu, int output)
{
	unsigned int i;
	unsigned int a, b, c, d;

	if (cpu->cpuid_level < 4)
		return;

	i = 0;
	for (;;) {
		const char *type;
		unsigned int level;
		unsigned int associativity;
		unsigned int partitions;
		unsigned int line_size;
		unsigned int sets;
		unsigned int size;

		cpuid4(cpu->number, i++, &a, &b, &c, &d);
		if ((a & 0x1f) == 0)
			break;

		type = cache_types[a & 0x1f];
		if (!type)
			type = "Unknown Cache Type";
		level = (a >> 5) & 0x7;
		associativity = ((b >> 22) & 0x3ff) + 1;
		partitions = ((b >> 12) & 0x3ff) + 1;
		line_size = ((b >> 0) & 0xfff) + 1;
		sets = c + 1;

		size = (associativity * partitions * line_size * sets) / 1024;

		if (output)
			printf(" L%u %s: %uKB, %u-way associative, %u byte line size\n",
			       level, type, size, associativity, line_size);
	}
}

void decode_intel_caches(struct cpudata *cpu, int output)
{
	unsigned int i = 0;
	unsigned char oldknown;

	if (cpu->cpuid_level < 2)
		return;

	memset(&unknown_array, 0, sizeof(unknown_array));

	if (output)
		printf("Cache info\n");

	decode_cache(cpu, TRACE_cache_table, output);
	decode_cache(cpu, L1I_cache_table, output);
	decode_cache(cpu, L1D_cache_table, output);
	oldknown = found_unknown;
	decode_cache(cpu, L2_cache_table, output);
	if (found_unknown > 0) {
		if (oldknown == found_unknown)
			 decode_cache(cpu, L2L3_cache_table, output);
		else
			 decode_cache(cpu, L3L2_cache_table, output);
	}

	decode_cache(cpu, L3_cache_table, output);
	/* The cache descriptor 0xff means the CPU doesn't provide any
	 * cache information in cpuid.2. It is instead provided in
	 * cpuid.4.
	 */
	if (found_general) {
		decode_general_cache (cpu, output);
		found_general = 0;
	};
	if (output)
		printf("TLB info\n");
	decode_cache(cpu, ITLB_cache_table, output);
	decode_cache(cpu, DTLB_cache_table, output);
	decode_cache(cpu, prefetch_table, output);

	if (found_unknown == 0)
		return;

	/* Remove any known entries */
	clean_unknowns(TRACE_cache_table);
	clean_unknowns(L1I_cache_table);
	clean_unknowns(L1D_cache_table);
	clean_unknowns(L2_cache_table);
	clean_unknowns(L3_cache_table);
	clean_unknowns(L2L3_cache_table);
	clean_unknowns(ITLB_cache_table);
	clean_unknowns(DTLB_cache_table);
	clean_unknowns(prefetch_table);

	if (found_unknown == 0)
		return;

	if (output)
		printf("Found unknown cache descriptors: ");

	for (i=0; i<256; i++) {
		if (unknown_array[i] == 1)
			if (output)
				printf("%02x ", i);
	}
	if (output)
		printf("\n");
	found_unknown = 0;
}

