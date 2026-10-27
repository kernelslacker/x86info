/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD TLB and cache information (CPUID 0x80000005/6/19/21/1D).
 */

#include <stdio.h>
#include <x86info.h>
#include <amd.h>

static void do_assoc(unsigned long assoc)
{
	switch (assoc & 0xff) {
	case 0x0:
		break;
	case 0x1:
		printf("Direct mapped. ");
		break;
	case 0xff:
		printf("Fully associative. ");
		break;
	default:
		printf("%lu-way associative. ", assoc);
	}
}

/* L2/L3 associativity field to number of ways; 0 for the special values. */
static const unsigned char l2assoc_ways[16] = {
	[0x2] = 2, [0x4] = 4, [0x6] = 8, [0x8] = 16, [0xa] = 32,
	[0xb] = 48, [0xc] = 64, [0xd] = 96, [0xe] = 128,
};

static void do_l2assoc(unsigned long assoc)
{
	switch (assoc) {
	case 0x0:
		printf("Disabled. ");
		return;
	case 0x1:
		printf("Direct mapped. ");
		return;
	case 0xf:
		printf("Fully associative. ");
		return;
	case 0x9:
		printf("See CPUID 0x8000001D. ");
		return;
	}

	if (assoc < ARRAY_SIZE(l2assoc_ways) && l2assoc_ways[assoc])
		printf("%u-way associative. ", l2assoc_ways[assoc]);
}

static void print_tlb(const char *label, void (*assoc_fn)(unsigned long),
		      unsigned long assoc, unsigned int entries)
{
	printf("%-28s", label);
	assoc_fn(assoc);
	printf("%u entries.\n", entries);
}

static void print_cache(const char *label, unsigned int size_kb,
			void (*assoc_fn)(unsigned long), unsigned long assoc,
			unsigned int lines_per_tag, unsigned int line_size)
{
	printf("%s:\n\tSize: %uKb\t", label, size_kb);
	assoc_fn(assoc);
	printf("\n\tlines per tag=%u\tline size=%u bytes.\n",
	       lines_per_tag, line_size);
}

void decode_amd_cacheinfo(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
	unsigned int tlb1g_eax = 0, tlb1g_ebx = 0;
	unsigned int shift = 0;
	int have_1g = 0;

	if ((cpu->eflags_edx & 1<<26) && cpu->maxei >= 0x80000019) {
		cpuid(cpu->number, 0x80000019, &tlb1g_eax, &tlb1g_ebx, &ecx, &edx);
		have_1g = 1;

		print_tlb("L1 Data TLB (1G):", do_l2assoc, tlb1g_eax >> 28,
			  (tlb1g_eax >> 16) & 0xfff);
		print_tlb("L1 Instruction TLB (1G):", do_l2assoc,
			  (tlb1g_eax >> 12) & 0xf, tlb1g_eax & 0xfff);
	}

	if (cpu->maxei >= 0x80000005) {
		/* TLB and cache info */
		cpuid(cpu->number, 0x80000005, &eax, &ebx, &ecx, &edx);

		print_tlb("L1 Data TLB (2M/4M):", do_assoc, eax >> 24,
			  (eax >> 16) & 0xff);
		print_tlb("L1 Instruction TLB (2M/4M):", do_assoc,
			  (eax >> 8) & 0xff, eax & 0xff);
		print_tlb("L1 Data TLB (4K):", do_assoc, ebx >> 24,
			  (ebx >> 16) & 0xff);
		print_tlb("L1 Instruction TLB (4K):", do_assoc,
			  (ebx >> 8) & 0xff, ebx & 0xff);

		print_cache("L1 Data cache", ecx >> 24, do_assoc,
			    (ecx >> 16) & 0xff, (ecx >> 8) & 0xff, ecx & 0xff);
		print_cache("L1 Instruction cache", edx >> 24, do_assoc,
			    (edx >> 16) & 0xff, (edx >> 8) & 0xff, edx & 0xff);
	}

	if (have_1g) {
		print_tlb("L2 Data TLB (1G):", do_l2assoc, tlb1g_ebx >> 28,
			  (tlb1g_ebx >> 16) & 0xfff);
		print_tlb("L2 Instruction TLB (1G):", do_l2assoc,
			  (tlb1g_ebx >> 12) & 0xf, tlb1g_ebx & 0xfff);
	}

	if (cpu->maxei >= 0x80000006) {
		/* K6-III (and later) on-chip L2 cache size */
		cpuid(cpu->number, 0x80000006, &eax, &ebx, &ecx, &edx);

		/* L2TlbSizeX32: L2 TLB sizes are multiples of 32 */
		if (cpu->maxei >= 0x80000021) {
			unsigned int ext2_eax, d1, d2, d3;

			cpuid(cpu->number, 0x80000021, &ext2_eax, &d1, &d2, &d3);
			if (ext2_eax & (1 << 14))
				shift = 5;
		}

		print_tlb("L2 Data TLB (2M/4M):", do_l2assoc, eax >> 28,
			  ((eax >> 16) & 0xfff) << shift);
		print_tlb("L2 Instruction TLB (2M/4M):", do_l2assoc,
			  (eax >> 12) & 0xf, (eax & 0xfff) << shift);
		print_tlb("L2 Data TLB (4K):", do_l2assoc, ebx >> 28,
			  ((ebx >> 16) & 0xfff) << shift);
		print_tlb("L2 Instruction TLB (4K):", do_l2assoc,
			  (ebx >> 12) & 0xf, (ebx & 0xfff) << shift);

		print_cache("L2 cache", ecx >> 16, do_l2assoc,
			    (ecx >> 12) & 0x0f, (ecx >> 8) & 0x0f, ecx & 0xff);
		if (family(cpu) >= 0x10) {
			if (!(edx >> 18))
				printf("L3 (shared) cache:\n\tnone/disabled\n");
			else
				print_cache("L3 (shared) cache", (edx >> 18) * 512,
					    do_l2assoc, (edx >> 12) & 0x0f,
					    (edx >> 8) & 0x0f, edx & 0xff);
		}
	}

	if (family(cpu) >= 0x17 && cpu->maxei >= 0x8000001d) {
		static const char * const ctype[] = {
			"", "data", "instruction", "unified"
		};
		unsigned int sub;

		printf("Caches (CPUID 0x8000001D):\n");
		for (sub = 0; sub < 8; sub++) {
			unsigned int type, ways, parts, line;
			unsigned long long sets;

			cpuid_count(cpu->number, 0x8000001d, sub, &eax, &ebx, &ecx, &edx);
			type = eax & 0x1f;
			if (type == 0 || type >= ARRAY_SIZE(ctype))
				break;
			ways = (ebx >> 22) + 1;
			parts = ((ebx >> 12) & 0x3ff) + 1;
			line = (ebx & 0xfff) + 1;
			sets = (unsigned long long)ecx + 1;
			printf("\tL%u %s: %lluKb, %u-way, line size=%u bytes, shared by %u threads\n",
			       (eax >> 5) & 7, ctype[type],
			       sets * ways * parts * line / 1024, ways, line,
			       ((eax >> 14) & 0xfff) + 1);
		}
	}

	printf("\n");
}
