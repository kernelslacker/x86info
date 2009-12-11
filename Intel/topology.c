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

static int get_count_order(unsigned int count)
{
	int order;

	order = fls(count) - 1;
	if (count & (count - 1))
		order++;
	return order;
}

static int intel_num_cpu_cores(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	if (cpu->cpuid_level < 4)
		return 1;

	/* Intel has a non-standard dependency on %ecx for this CPUID level. */
	cpuid_count(cpu, 4, 0, &eax, &ebx, &ecx, &edx);
	if (eax & 0x1f)
		return (eax >> 26) + 1;
	else
		return 1;
}

static int phys_pkg_id(int cpuid_apic, int index_msb)
{
	return cpuid_apic >> index_msb;
}

#define X86_FEATURE_HT
#define X86_FEATURE_CMP_LEGACY
#define X86_FEATURE_XTOPOLOGY

void show_intel_topology(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
	unsigned int index_msb, core_bits;
	unsigned int smp_num_siblings;
/*
	if (!cpu_has(cpu, X86_FEATURE_HT))
		return;

	if (cpu_has(cpu, X86_FEATURE_CMP_LEGACY))
		goto out;

	if (cpu_has(cpu, X86_FEATURE_XTOPOLOGY))
		return;
*/


	cpuid(cpu->number, 1, &eax, &ebx, &ecx, &edx);
	smp_num_siblings = (ebx & 0xff0000) >> 16;

	if (smp_num_siblings == 1) {
		printf("Hyper-Threading is disabled\n");
		goto out;
	}

	if (smp_num_siblings <= 1)
		goto out;

	index_msb = get_count_order(smp_num_siblings);
	cpu->initial_apicid = (cpuid_ebx(cpu, 1) >> 24) & 0xFF;
	cpu->phys_proc_id = phys_pkg_id(cpu->initial_apicid, index_msb);

	cpu->x86_max_cores = intel_num_cpu_cores(cpu);
	smp_num_siblings = smp_num_siblings / cpu->x86_max_cores;

	index_msb = get_count_order(smp_num_siblings);

	core_bits = get_count_order(cpu->x86_max_cores);

	cpu->cpu_core_id = phys_pkg_id(cpu->apicid, index_msb) &
		((1 << core_bits) - 1);

out:
	if ((cpu->x86_max_cores * smp_num_siblings) > 1) {
		printf("Physical Processor ID: %d\n", cpu->phys_proc_id);
		printf("Processor Core ID: %d\n", cpu->cpu_core_id);
	}
}
