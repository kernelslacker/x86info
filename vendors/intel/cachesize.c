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

enum desc_kind {
	K_TRACE,
	K_L1I,
	K_L1D,
	K_L2,
	K_L3,
	K_PREFETCH,
	K_ITLB,
	K_DTLB,
};

/* Descriptor 0x49 is an L3 cache on family 0xf model 6 and an L2 elsewhere. */
#define F0_M6_ONLY	1
#define NOT_F0_M6	2

struct cache_desc {
	unsigned char descriptor;
	unsigned char kind;
	unsigned char flags;
	int size;
	const char *string;
};

static const struct cache_desc cache_descs[] =
{
	{ 0x70, K_TRACE, 0, 12, "Instruction trace cache: 12K uOps, 8-way associative." },
	{ 0x71, K_TRACE, 0, 16, "Instruction trace cache: 16K uOps, 8-way associative." },
	{ 0x72, K_TRACE, 0, 32, "Instruction trace cache: 32K uOps, 8-way associative." },
	{ 0x73, K_TRACE, 0, 64, "Instruction trace cache: 64K uOps, 8-way associative." },
	{ 0x6, K_L1I, 0, 8, "L1 Instruction cache: 8KB, 4-way associative. 32 byte line size." },
	{ 0x8, K_L1I, 0, 16, "L1 Instruction cache: 16KB, 4-way associative. 32 byte line size." },
	{ 0x9, K_L1I, 0, 32, "L1 Instruction cache: 32KB, 4-way associative. 64 byte line size." },
	{ 0x30, K_L1I, 0, 32, "L1 Instruction cache: 32KB, 8-way associative. 64 byte line size." },
	{ 0xa, K_L1D, 0, 8, "L1 Data cache: 8KB, 2-way associative. 32 byte line size." },
	{ 0xc, K_L1D, 0, 16, "L1 Data cache: 16KB, 4-way associative. 32 byte line size." },
	{ 0xd, K_L1D, 0, 16, "L1 Data cache: 16KB, 4-way associative. 64 byte line size. ECC." },
	{ 0xe, K_L1D, 0, 24, "L1 Data cache: 24KB, 6-way associative. 64 byte line size. ECC." },
	{ 0x2c, K_L1D, 0, 32, "L1 Data cache: 32KB, 8-way associative. 64 byte line size." },
	{ 0x60, K_L1D, 0, 16, "L1 Data cache: 16KB, sectored, 8-way associative. 64 byte line size." },
	{ 0x66, K_L1D, 0, 8, "L1 Data cache: 8KB, sectored, 4-way associative. 64 byte line size." },
	{ 0x67, K_L1D, 0, 16, "L1 Data cache: 16KB, sectored, 4-way associative. 64 byte line size." },
	{ 0x68, K_L1D, 0, 32, "L1 Data cache: 32KB, sectored, 4-way associative. 64 byte line size." },
	{ 0x21, K_L2, 0, 256, "L2 (MLC): 256KB, 8-way associative. 64 byte line size." },
	{ 0x39, K_L2, 0, 128, "L2 cache: 128KB, 4-way associative. Sectored. 64 byte line size." },
	{ 0x3a, K_L2, 0, 192, "L2 cache: 192KB, 6-way associative. Sectored. 64 byte line size." },
	{ 0x3b, K_L2, 0, 128, "L2 cache: 128KB, 2-way associative. Sectored. 64 byte line size." },
	{ 0x3c, K_L2, 0, 256, "L2 cache: 256KB, 4-way associative. Sectored. 64 byte line size." },
	{ 0x3d, K_L2, 0, 384, "L2 cache: 384KB, 6-way associative. Sectored. 64 byte line size." },
	{ 0x3e, K_L2, 0, 512, "L2 cache: 512KB, 4-way associative. Sectored. 64 byte line size." },
	{ 0x3f, K_L2, 0, 256, "L2 cache: 256KB, 2-way associative. 64 byte line size." },
	{ 0x41, K_L2, 0, 128, "L2 cache: 128KB, 4-way associative. 32 byte line size." },
	{ 0x42, K_L2, 0, 256, "L2 cache: 256KB, 4-way associative. 32 byte line size." },
	{ 0x43, K_L2, 0, 512, "L2 cache: 512KB, 4-way associative. 32 byte line size." },
	{ 0x44, K_L2, 0, MB(1), "L2 cache: 1MB, 4-way associative. 32 byte line size." },
	{ 0x45, K_L2, 0, MB(2), "L2 cache: 2MB, 4-way associative. 32 byte line size." },
	{ 0x48, K_L2, 0, MB(3), "L2 cache: 3MB, 12-way associative. 64 byte line size. Unified on-die." },
	{ 0x4e, K_L2, 0, MB(6), "L2 cache: 6MB, 24-way set associative, 64-byte line size." },
	{ 0x79, K_L2, 0, 128, "L2 cache: 128KB, sectored, 8-way associative. 64 byte line size." },
	{ 0x7a, K_L2, 0, 256, "L2 cache: 256KB, sectored, 8-way associative. 64 byte line size." },
	{ 0x7b, K_L2, 0, 512, "L2 cache: 512KB, sectored, 8-way associative. 64 byte line size." },
	{ 0x7c, K_L2, 0, MB(1), "L2 cache: 1MB, sectored, 8-way associative. 64 byte line size." },
	{ 0x7d, K_L2, 0, MB(2), "L2 cache: 2MB, 8-way associative. 64 byte line size." },
	{ 0x7f, K_L2, 0, 512, "L2 cache: 512KB, 2-way associative. 64 byte line size." },
	{ 0x80, K_L2, 0, 512, "L2 cache: 512KB, 8-way associative. 64 byte line size." },
	{ 0x82, K_L2, 0, 256, "L2 cache: 256KB, 8-way associative. 32 byte line size." },
	{ 0x83, K_L2, 0, 512, "L2 cache: 512KB, 8-way associative. 32 byte line size." },
	{ 0x84, K_L2, 0, MB(1), "L2 cache: 1MB, 8-way associative. 32 byte line size." },
	{ 0x85, K_L2, 0, MB(2), "L2 cache: 2MB, 8-way associative. 32 byte line size." },
	{ 0x86, K_L2, 0, 512, "L2 cache: 512KB, 4-way associative. 64 byte line size." },
	{ 0x87, K_L2, 0, MB(1), "L2 cache: 1MB, 8-way associative. 64 byte line size." },
	{ 0x49, K_L2, NOT_F0_M6, MB(4), "L2 cache: 4MB, 16-way associative. 64 byte line size." },
	{ 0x78, K_L2, 0, MB(1), "L2 cache: 1MB, sectored, 8-way associative. 64 byte line size." },
	{ 0x22, K_L3, 0, 512, "L3 cache: 512KB, 4-way associative. 64 byte line size." },
	{ 0x23, K_L3, 0, MB(1), "L3 cache: 1MB, 8-way associative. 64 byte line size." },
	{ 0x25, K_L3, 0, MB(2), "L3 cache: 2MB, 8-way associative. 64 byte line size." },
	{ 0x29, K_L3, 0, MB(4), "L3 cache: 4MB, 8-way associative. 64 byte line size." },
	{ 0xd0, K_L3, 0, 512, "L3 cache: 512KB, 4-way associative. 64 byte line size." },
	{ 0xd1, K_L3, 0, MB(1), "L3 cache: 1MB, 4-way associative. 64 byte line size." },
	{ 0xd2, K_L3, 0, MB(2), "L3 cache: 2MB, 4-way associative. 64 byte line size." },
	{ 0xd6, K_L3, 0, MB(1), "L3 cache: 1MB, 8-way associative. 64 byte line size." },
	{ 0xd7, K_L3, 0, MB(2), "L3 cache: 2MB, 8-way associative. 64 byte line size." },
	{ 0xd8, K_L3, 0, MB(4), "L3 cache: 4MB, 8-way associative. 64 byte line size." },
	{ 0xdc, K_L3, 0, MB(2), "L3 cache: 2MB, 12-way associative. 64 byte line size." },
	{ 0xdd, K_L3, 0, MB(4), "L3 cache: 4MB, 12-way associative. 64 byte line size." },
	{ 0xe2, K_L3, 0, MB(2), "L3 cache: 2MB, 16-way associative. 64 byte line size." },
	{ 0xe3, K_L3, 0, MB(4), "L3 cache: 4MB, 16-way associative. 64 byte line size." },
	{ 0xe4, K_L3, 0, MB(8), "L3 cache: 8MB, 16-way associative. 64 byte line size." },
	{ 0xea, K_L3, 0, MB(12), "L3 cache: 12MB, 24-way associative. 64 byte line size." },
	{ 0xeb, K_L3, 0, MB(18), "L3 cache: 18MB, 24-way associative. 64 byte line size." },
	{ 0xec, K_L3, 0, MB(24), "L3 cache: 24MB, 24-way associative. 64 byte line size." },
	{ 0xde, K_L3, 0, MB(8), "L3 cache: 8MB, 12-way associative. 64 byte line size." },
	{ 0x46, K_L3, 0, MB(4), "L3 cache: 4MB, 4-way associative. 64 byte line size." },
	{ 0x47, K_L3, 0, MB(8), "L3 cache: 8MB, 8-way associative. 64 byte line size." },
	{ 0x49, K_L3, F0_M6_ONLY, MB(4), "L3 cache: 4MB, 16-way associative. 64 byte line size." },
	{ 0x4a, K_L3, 0, MB(6), "L3 cache: 6MB, 12-way associative. 64 byte line size." },
	{ 0x4b, K_L3, 0, MB(8), "L3 cache: 8MB, 16-way associative. 64 byte line size." },
	{ 0x4c, K_L3, 0, MB(12), "L3 cache: 12MB, 12-way associative. 64 byte line size." },
	{ 0x4d, K_L3, 0, MB(16), "L3 cache: 16MB, 16-way associative. 64 byte line size." },
	{ 0xf0, K_PREFETCH, 0, 64, "64 byte prefetching." },
	{ 0xf1, K_PREFETCH, 0, 64, "128 byte prefetching." },
	{ 0x1, K_ITLB, 0, 32, "Instruction TLB: 4KB pages, 4-way associative, 32 entries" },
	{ 0x2, K_ITLB, 0, 2, "Instruction TLB: 4MB pages, fully associative, 2 entries" },
	{ 0xb, K_ITLB, 0, 4, "Instruction TLB: 4MB pages, 4-way set associative, 4 entries" },
	{ 0x4f, K_ITLB, 0, 32, "Instruction TLB: 4KB pages, 32 entries" },
	{ 0x50, K_ITLB, 0, 64, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 64 entries." },
	{ 0x51, K_ITLB, 0, 128, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 128 entries." },
	{ 0x52, K_ITLB, 0, 256, "Instruction TLB: 4K, 2MB or 4MB pages, fully associative, 256 entries." },
	{ 0x55, K_ITLB, 0, 7, "Instruction TLB: 2MB or 4MB pages, fully associative, 7 entries" },
	{ 0x61, K_ITLB, 0, 48, "Instruction TLB: 4K pages, fully associative, 48 entries" },
	{ 0x76, K_ITLB, 0, 8, "Instruction TLB: 2M/4M pages, fully associative, 8 entries" },
	{ 0xb0, K_ITLB, 0, 128, "Instruction TLB: 4K pages, 4-way associative, 128 entries." },
	{ 0xb1, K_ITLB, 0, 4, "Instruction TLB: 4x 4MB page entries, or 8x 2MB pages entries, 4-way associative" },
	{ 0xb2, K_ITLB, 0, 64, "Instruction TLB: 4K pages, 4-way associative, 64 entries." },
	{ 0xb5, K_ITLB, 0, 64, "Instruction TLB: 4K pages, 8-way associative, 64 entries" },
	{ 0xb6, K_ITLB, 0, 128, "Instruction TLB: 4K pages, 8-way associative, 128 entries" },
	{ 0x3, K_DTLB, 0, 64, "Data TLB: 4KB pages, 4-way associative, 64 entries" },
	{ 0x4, K_DTLB, 0, 8, "Data TLB: 4MB pages, 4-way associative, 8 entries" },
	{ 0x5, K_DTLB, 0, 32, "Data TLB: 4MB pages, 4-way associative, 32 entries" },
	{ 0x56, K_DTLB, 0, 16, "Data TLB0: 4MB pages, 4-way associative, 16 entries" },
	{ 0x57, K_DTLB, 0, 16, "Data TLB0: 4KB pages, 4-way associative, 16 entries" },
	{ 0x59, K_DTLB, 0, 16, "Data TLB0: 4KB pages, fully associative, 16 entries" },
	{ 0x5a, K_DTLB, 0, 32, "Data TLB0: 2MB or 4MB pages, 4-way set associative, 32 entries" },
	{ 0x5b, K_DTLB, 0, 64, "Data TLB: 4KB or 4MB pages, fully associative, 64 entries." },
	{ 0x5c, K_DTLB, 0, 128, "Data TLB: 4KB or 4MB pages, fully associative, 128 entries." },
	{ 0x5d, K_DTLB, 0, 256, "Data TLB: 4KB or 4MB pages, fully associative, 256 entries." },
	{ 0x63, K_DTLB, 0, 32, "Data TLB: 2MB/4MB pages, 4-way set associative, 32 entries\n Data TLB: 1GB pages, 4-way set associative, 4 entries" },
	{ 0x64, K_DTLB, 0, 512, "Data TLB: 4KB pages, 4-way associative, 512 entries" },
	{ 0x6b, K_DTLB, 0, 256, "Data TLB: 4KB pages, 8-way associative, 256 entries" },
	{ 0x6c, K_DTLB, 0, 128, "Data TLB: 2MB/4MB pages, 8-way associative, 128 entries" },
	{ 0x6d, K_DTLB, 0, 16, "Data TLB: 1GB pages, fully associative, 16 entries" },
	{ 0xa0, K_DTLB, 0, 32, "Data TLB: 4KB pages, full associative, 32 entries" },
	{ 0xb3, K_DTLB, 0, 128, "Data TLB: 4KB pages, 4-way associative, 128 entries" },
	{ 0xb4, K_DTLB, 0, 256, "Data TLB1: 4KB pages, 4-way associative, 256 entries" },
	{ 0xba, K_DTLB, 0, 64, "Data TLB1: 4KB pages, 4-way associative, 64 entries" },
	{ 0xc0, K_DTLB, 0, 8, "Data TLB: 4KB or 4MB pages, 4-way associative, 8 entries" },
	{ 0xc1, K_DTLB, 0, 1024, "Shared L2 TLB: 4KB/2MB pages, 8-way associative, 1024 entries" },
	{ 0xc2, K_DTLB, 0, 16, "Data TLB: 2MB/4MB pages, 4-way associative, 16 entries" },
	{ 0xc3, K_DTLB, 0, 1536, "Shared L2 TLB: 4KB/2MB pages, 6-way associative, 1536 entries\n Shared L2 TLB: 1GB pages, 4-way associative, 16 entries" },
	{ 0xc4, K_DTLB, 0, 32, "Data TLB: 2MB/4MB pages, 4-way associative, 32 entries" },
	{ 0xca, K_DTLB, 0, 512, "Shared L2 TLB: 4KB pages, 4-way associative, 512 entries" },
};

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
	const char *unit;

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

		cpuid_count(cpu->number, 4, i++, &a, &b, &c, &d);
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

		unit = "KB";
		size = (associativity * partitions * line_size * sets) / 1024;
		if (size >= 1024 && size % 1024 == 0) {
			size /= 1024;
			unit = "MB";
		}

		if (output)
			printf(" L%u %s: %u%s, %u-way associative, %u byte line size\n",
			       level, type, size, unit, associativity, line_size);
	}
}

static const char * const tlbnames[32] = {
	NULL,
	"Data",
	"Instruction",
	"Unified",
	"Load Only",
	"Store Only",
};

static const char * const pagenames[] = {
	"None",
	"4KB",
	"2MB",
	"4KB/2MB",
	"4MB",
	"4MB/4KB",
	"4MB/2MB",
	"4MB/2MB/4KB",
	"1GB",
	"1GB/4KB",
	"1GB/2MB",
	"1GB/4KB/2MB",
	"1GB/4MB",
	"1GB/4MB/4KB",
	"1GB/4MB/2MB",
	"1GB/4MB/2MB/4KB",
};

static void decode_general_tlb(struct cpudata *cpu, int output)
{
	unsigned int i = 0;
	unsigned int max = 0xffffffff;
	unsigned int a, b, c, d;

	if (cpu->cpuid_level < 0x18) {
		if (output)
			printf("Leaf 0x18 not supported\n");
		return;
	}

	while (i <= max) {
		unsigned long long idx = i;
		unsigned int page_size, ways, type, level, entries;
		const char *typename;
		char assoc[32];

		idx <<= 32;
		idx |= 0x18;
		cpuid(cpu->number, idx, &a, &b, &c, &d);

		if (i++ == 0) {
			max = a;
			if (max > 64)
				max = 64;
		}
		page_size = b & 0xf;
		ways = b >> 16;
		type = d & 0x1f;
		level = (d >> 5) & 7;
		entries = c * ways;
		if ((d >> 8) & 1)
			strcpy(assoc, "fully");
		else
			snprintf(assoc, sizeof(assoc), "%u-way", ways);
		if (type == 0)
			continue;
		typename = tlbnames[type];
		if (!typename)
			typename = "Unknown";
		if (output)
			printf(" L%u %s TLB: %s pages, %s associative, "
				"%u entries\n", level, typename,
				pagenames[page_size], assoc, entries);
	}
}

/* Descriptor bytes that carry no table entry. */
#define DESC_NO_L3	0x40
#define DESC_USE_LEAF4	0xff
#define DESC_USE_LEAF18	0xfe

#define MAX_LEAF2_ITER	8

static int desc_applies(const struct cache_desc *d, const struct cpudata *cpu)
{
	int f0_m6 = cpu->family == 0xf && cpu->model == 6;

	if ((d->flags & F0_M6_ONLY) && !f0_m6)
		return 0;
	if ((d->flags & NOT_F0_M6) && f0_m6)
		return 0;
	return 1;
}

static const struct cache_desc *lookup_desc(unsigned char des,
					    const struct cpudata *cpu)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cache_descs); i++)
		if (cache_descs[i].descriptor == des &&
		    desc_applies(&cache_descs[i], cpu))
			return &cache_descs[i];
	return NULL;
}

/* Read the CPUID 2 descriptor bytes; returns how many were stored. */
static unsigned int read_descriptors(struct cpudata *cpu,
				     unsigned char *bytes)
{
	unsigned int i, j, n, count = 0;
	unsigned int regs[4];

	cpuid(cpu->number, 2, &regs[0], &regs[1], &regs[2], &regs[3]);

	/* Number of times to iterate */
	n = regs[0] & 0xff;
	if (n > MAX_LEAF2_ITER)
		n = MAX_LEAF2_ITER;

	for (i = 0; i < n; i++) {
		cpuid(cpu->number, 2, &regs[0], &regs[1], &regs[2], &regs[3]);

		/* If bit 31 is set, this is an unknown format */
		for (j = 0; j < 4; j++)
			if (regs[j] & 0x80000000)
				regs[j] = 0;

		/* Byte 0 is level count, not a descriptor */
		for (j = 1; j < 16; j++) {
			unsigned char val = regs[j / 4] >> (unsigned int)(8 * (j % 4));

			if (val)
				bytes[count++] = val;
		}
	}
	return count;
}

/* Print (and total up) the descriptors of one kind, in CPUID order. */
static void decode_kind(struct cpudata *cpu, const unsigned char *bytes,
			unsigned int count, enum desc_kind kind, int output)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		const struct cache_desc *d = lookup_desc(bytes[i], cpu);

		if (!d || d->kind != kind)
			continue;

		switch (kind) {
		case K_L1I:
			cpu->cachesize_L1_I += d->size;
			break;
		case K_L1D:
			cpu->cachesize_L1_D += d->size;
			break;
		case K_L2:
			cpu->cachesize_L2 += d->size;
			break;
		case K_L3:
			cpu->cachesize_L3 += d->size;
			break;
		case K_TRACE:
		case K_PREFETCH:
		case K_ITLB:
		case K_DTLB:
			break;
		}

		if (output)
			printf(" %s\n", d->string);
	}
}

static int has_descriptor(const unsigned char *bytes, unsigned int count,
			  unsigned char des)
{
	unsigned int i;

	for (i = 0; i < count; i++)
		if (bytes[i] == des)
			return 1;
	return 0;
}

static void print_unknown(const struct cpudata *cpu, const unsigned char *bytes,
			  unsigned int count)
{
	unsigned char seen[256] = { 0 };
	int any = 0;
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned char des = bytes[i];

		if (des == DESC_NO_L3 || des == DESC_USE_LEAF4 ||
		    des == DESC_USE_LEAF18 || seen[des] || lookup_desc(des, cpu))
			continue;
		seen[des] = 1;
		printf(any ? " %02x" : "Found unknown cache descriptors: %02x", des);
		any = 1;
	}
	if (any)
		printf("\n");
}

/* Sizes summed from the CPUID 2 descriptors; levels the CPU lacks are left out. */
static void print_cache_totals(const struct cpudata *cpu)
{
	const struct {
		const char *label;
		unsigned int kb;
	} levels[] = {
		{ "L1 instruction", cpu->cachesize_L1_I },
		{ "L1 data", cpu->cachesize_L1_D },
		{ "L2", cpu->cachesize_L2 },
		{ "L3", cpu->cachesize_L3 },
	};
	const char *sep = "Total cache:";
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(levels); i++) {
		if (!levels[i].kb)
			continue;
		printf("%s %s %uKB", sep, levels[i].label, levels[i].kb);
		sep = ",";
	}
	if (sep[0] == ',')
		printf("\n");
}

void decode_intel_caches(struct cpudata *cpu, int output)
{
	unsigned char bytes[16 * MAX_LEAF2_ITER];
	unsigned int count;
	int use_leaf4, use_leaf18;

	if (cpu->cpuid_level < 2)
		return;

	cpu->cachesize_L1_I = 0;
	cpu->cachesize_L1_D = 0;
	cpu->cachesize_L2 = 0;
	cpu->cachesize_L3 = 0;

	count = read_descriptors(cpu, bytes);
	use_leaf4 = has_descriptor(bytes, count, DESC_USE_LEAF4);
	use_leaf18 = has_descriptor(bytes, count, DESC_USE_LEAF18);

	if (output)
		printf("Cache info\n");

	decode_kind(cpu, bytes, count, K_TRACE, output);
	decode_kind(cpu, bytes, count, K_L1I, output);
	decode_kind(cpu, bytes, count, K_L1D, output);
	decode_kind(cpu, bytes, count, K_L2, output);
	decode_kind(cpu, bytes, count, K_L3, output);
	if (output && !use_leaf4)
		print_cache_totals(cpu);
	/* The cache descriptor 0xff means the CPU doesn't provide any
	 * cache information in cpuid.2. It is instead provided in
	 * cpuid.4.
	 */
	if (use_leaf4)
		decode_general_cache(cpu, output);
	decode_kind(cpu, bytes, count, K_PREFETCH, output);
	if (output)
		printf("TLB info\n");
	if (use_leaf18)
		decode_general_tlb(cpu, output);
	decode_kind(cpu, bytes, count, K_ITLB, output);
	decode_kind(cpu, bytes, count, K_DTLB, output);

	if (output)
		print_unknown(cpu, bytes, count);
}
