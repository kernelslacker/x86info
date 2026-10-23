/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <x86info.h>
#include "intel.h"

void dump_performance_msrs(struct cpudata *cpu)
{
	unsigned long long val = 0;

	printf("Performance msrs:\n");
	if (read_msr(cpu->number, MSR_IA32_PERF_STATUS, &val) == 1)
		printf("  MSR_IA32_PERF_STATUS: 0x%llx\n", val);

	if (read_msr(cpu->number, MSR_IA32_MISC_ENABLE, &val) == 1) {
		printf("  MSR_IA32_MISC_ENABLE: 0x%llx", val);
		decode_misc_enable_brief(val);
	}
	printf("\n");
}
