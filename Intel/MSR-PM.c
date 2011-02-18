/*
 *  (C) 2002 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel Pentium M specific MSR information
 *  See 24547203.pdf for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "Intel.h"

void dump_centrino_MSRs(struct cpudata *cpu)
{
	unsigned long long val = 0;
	int tcc = 0;

	if (!user_is_root)
		return;

	printf("Performance MSRs:\n");
	if (read_msr(cpu->number, MSR_IA32_PERF_STATUS, &val) == 1)
		printf("  MSR_IA32_PERF_STATUS: %llx\n", val);

	if (read_msr(cpu->number, MSR_IA32_MISC_ENABLE, &val) == 1) {
		printf("  MSR_IA32_MISC_ENABLE: %llx", val);
		printf(" [Enabled: ");
		if (val & (1<<3)) {
			printf("TCC ");
			tcc = 1;
		}
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


	printf("Thermal MSRs:\n");
	if (tcc && read_msr(cpu->number, MSR_PM_THERM2_CTL, &val) == 1) { /* THERM2_CTL */
		printf("  MSR_PM_THERM2_CTL: %llx, [Thermal monitor: %d]\n",
			val, (val & (1<<16)) ? 2 : 1);
	}
	if (read_msr(cpu->number, MSR_IA32_THERM_CONTROL, &val) == 1) {
		printf("  MSR_IA32_THERM_CONTROL: %llx", val);
		if (val & (1<<4)) {
			printf("[Software-controlled clock: %f%% duty cycle]\n",
			       ((val >> 1) & 7) / 8.);
		} else
			printf("[Software-controlled clock disabled (full speed)]\n");
	}
	if (read_msr (cpu->number, MSR_IA32_THERM_STATUS, &val) == 1) { /* THERM_STATUS */
		printf("  MSR_IA32_THERM_STATUS: %llx", val);
		if (val & (1<<0|1<<1)) {
			printf(" [");
			if (val & (1<<0))
				printf("TooHot ");
			if (val & (1<<1))
				printf("WasTooHot ");
			printf("]");
		}
		printf("\n");
	}
	printf("\n");
}
