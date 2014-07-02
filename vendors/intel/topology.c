/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel multicore/multithread determination.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <x86info.h>
#include "intel.h"

/**
 * tp_fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */

static int tp_fls(int x)
{
	return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}

static int get_count_order(unsigned int count)
{
	int order;

	order = tp_fls(count) - 1;
	if (count & (count - 1))
		order++;
	return order;
}

static int intel_num_cpu_cores(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	if (cpu->cpuid_level < 4)
		return 1;

	/* intel.has a non-standard dependency on %ecx for this CPUID level. */
	cpuid_count(cpu->number, 4, 0, &eax, &ebx, &ecx, &edx);
	if (eax & 0x1f)
		return (eax >> 26) + 1;
	else
		return 1;
}

static int phys_pkg_id(int cpuid_apic, int index_msb)
{
	return cpuid_apic >> index_msb;
}

void get_intel_topology(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
	unsigned int index_msb, core_bits;

	if (!(cpu->flags_edx & X86_FEATURE_HT))
		return;
/*
	if (cpu_has(cpu, X86_FEATURE_CMP_LEGACY))
		goto out;

	if (cpu_has(cpu, X86_FEATURE_XTOPOLOGY))
		return;
*/


	cpuid(cpu->number, 1, &eax, &ebx, &ecx, &edx);
	cpu->num_siblings = (ebx & 0xff0000) >> 16;

	if (cpu->num_siblings == 1) {
		if (debug)
			printf("Hyper-Threading is disabled\n");
		goto out;
	}

	if (cpu->num_siblings <= 1)
		goto out;

	index_msb = get_count_order(cpu->num_siblings);
	cpu->initial_apicid = (cpuid_ebx(cpu->number, 1) >> 24) & 0xFF;
	cpu->phys_proc_id = phys_pkg_id(cpu->initial_apicid, index_msb);

	cpu->x86_max_cores = intel_num_cpu_cores(cpu);
	cpu->num_siblings = cpu->num_siblings / cpu->x86_max_cores;

	index_msb = get_count_order(cpu->num_siblings);

	core_bits = get_count_order(cpu->x86_max_cores);

	cpu->cpu_core_id = phys_pkg_id(cpu->apicid, index_msb) &
		((1 << core_bits) - 1);

	if (debug == 1) {
		if ((cpu->x86_max_cores * cpu->num_siblings) > 1) {
			printf("%s:\n", __func__);
			printf("\tSiblings: %d\n", cpu->num_siblings);
			printf("\tPhysical Processor ID: %d\n", cpu->phys_proc_id);
			printf("\tProcessor Core ID: %d\n", cpu->cpu_core_id);
		}
	}
out:
	return;
}
