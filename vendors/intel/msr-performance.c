/*
 *  (C) 2011 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <x86info.h>
#include "intel.h"

void dump_performance_MSRs(struct cpudata *cpu)
{
	unsigned long long val = 0;

	if (!user_is_root)
		return;

	printf("Performance MSRs:\n");
	if (read_msr(cpu->number, MSR_IA32_PERF_STATUS, &val) == 1)
		printf("  MSR_IA32_PERF_STATUS: 0x%llx\n", val);

	if (read_msr(cpu->number, MSR_IA32_MISC_ENABLE, &val) == 1) {
		printf("  MSR_IA32_MISC_ENABLE: 0x%llx", val);
		printf(" [Enabled: ");
		if (val & (1<<3))
			printf("TCC ");
		if (val & (1<<7))
			printf("PerfMon ");
		if (val & (1<<10))
			printf("FERR# ");
		if (val & (1<<11))
			printf("noBTS ");
		if (val & (1<<12))
			printf("noPEBS ");
		if (val & (1<<16))
			printf("EnhancedSpeedStep ");

		printf("]\n");
	}
	printf("\n");
}
