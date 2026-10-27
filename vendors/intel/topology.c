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

int intel_num_cpu_cores(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	if (cpu->cpuid_level < 4)
		return 1;

	/* intel.has a non-standard dependency on %ecx for this CPUID level. */
	cpuid_count(cpu->number, 4, 0, &eax, &ebx, &ecx, &edx);
	if (eax & 0x1f)
		return ((eax >> 26) & 0x3f) + 1;
	else
		return 1;
}

void get_intel_topology(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
	unsigned int logical, index_msb, core_bits, initial_apicid;

	cpuid(cpu->number, 1, &eax, &ebx, &ecx, &edx);
	initial_apicid = (ebx >> 24) & 0xFF;

	logical = 1;
	if (cpu->flags_edx & X86_FEATURE_HT)
		logical = (ebx & 0xff0000) >> 16;

	if (logical < cpu->x86_max_cores)
		logical = cpu->x86_max_cores;

	index_msb = get_count_order(logical);
	cpu->phys_proc_id = initial_apicid >> index_msb;

	cpu->num_siblings = logical / cpu->x86_max_cores;

	if (opts.debug && cpu->num_siblings == 1)
		printf("Hyper-Threading is disabled\n");

	index_msb = get_count_order(cpu->num_siblings);

	core_bits = get_count_order(cpu->x86_max_cores);

	cpu->cpu_core_id = (initial_apicid >> index_msb) &
		((1 << core_bits) - 1);

	if (opts.debug) {
		if ((cpu->x86_max_cores * cpu->num_siblings) > 1) {
			printf("%s:\n", __func__);
			printf("\tSiblings: %u\n", cpu->num_siblings);
			printf("\tPhysical Processor ID: %u\n", cpu->phys_proc_id);
			printf("\tProcessor Core ID: %u\n", cpu->cpu_core_id);
		}
	}
}
