/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <x86info.h>
#include "intel.h"

void dump_thermal_msrs(struct cpudata *cpu)
{
	unsigned long long val = 0;

	if (!user_is_root)
		return;

	if (read_msr(cpu->number, MSR_IA32_MISC_ENABLE, &val) != 1)
		return;

	// tcc enabled ?
	if (!(val & (1<<3)))
		return;

	printf("Thermal msrs:\n");
	if (read_msr(cpu->number, MSR_PM_THERM2_CTL, &val) == 1) { /* THERM2_CTL */
		printf("  MSR_PM_THERM2_CTL: 0x%llx [Thermal monitor: %d]\n",
			val, (val & (1<<16)) ? 2 : 1);
	}
	if (read_msr(cpu->number, MSR_IA32_THERM_CONTROL, &val) == 1) {
		printf("  MSR_IA32_THERM_CONTROL: 0x%llx ", val);
		if (val & (1<<4)) {
			printf("[Software-controlled clock: %f%% duty cycle]\n",
			       ((val >> 1) & 7) / 8.);
		} else
			printf("[Software-controlled clock disabled (full speed)]\n");
	}
	if (read_msr (cpu->number, MSR_IA32_THERM_STATUS, &val) == 1) { /* THERM_STATUS */
		printf("  MSR_IA32_THERM_STATUS: 0x%llx", val);
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
