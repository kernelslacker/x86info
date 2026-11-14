/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel P4 specific MSR information
 *  See 24547203.pdf for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <x86info.h>
#include "intel.h"

void dump_p4_msrs (struct cpudata *cpu)
{
	unsigned long long val = 0;

	printf("Pentium 4 specific msrs:\n");

	if (read_msr (cpu->number, MSR_IA32_PLATFORM_ID, &val))
		printf("IA32_PLATFORM_ID=%016llx\n", val);

	if (read_msr (cpu->number, 0x2a, &val)) {
		printf("System bus in order queue depth=");
		if ((val & (1<<2)))
			printf("1");
		else
			printf("12");
		printf("\n");
	}

	if (read_msr (cpu->number, 0x2c, &val)) {
		printf("MSR_EBC_FREQUENCY_ID=%016llx\n", val);
	}

	if (read_msr (cpu->number, MSR_IA32_UCODE_REV, &val)) {
		printf("IA32_BIOS_SIGN_ID=%016llx\n", val);
	}

	if (read_msr (cpu->number, 0x119, &val)) {
		printf("Processor serial number is ");
		if ((val & (1<<21)))
			printf("dis");
		else
			printf("en");
		printf("abled\n");
	}

	if (read_msr (cpu->number, MSR_IA32_MISC_ENABLE, &val)) {
		decode_misc_enable(val);
	}

	printf("\n");
}
