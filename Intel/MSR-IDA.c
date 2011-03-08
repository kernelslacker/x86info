/*
 *  (C) 2011 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "Intel.h"

void dump_IDA_MSRs(struct cpudata *cpu)
{
	unsigned long long val = 0;
	unsigned int eax, ebx, ecx, edx;

	if (!user_is_root)
		return;

	if (cpu->cpuid_level < 6)
		return;

	cpuid_count(cpu->number, 6, 0, &eax, &ebx, &ecx, &edx);
	printf("Dynamic Acceleration MSRs:\n");
	printf("  Opportunistic performance operation ");
	if ((eax & (1 << 1)) == 1)
		printf("enabled by BIOS\n");
	else
		printf("disabled by BIOS (or not supported)\n");

	if (read_msr(cpu->number, MSR_IA32_MISC_ENABLE, &val) != 1)
		return;

	if ((val & (1L << 38)) == 1) {
		printf("  IA32_MISC_ENABLES[38] is 1 (disabled opportunistic performance operation)\n");
		return;
	}

	printf("\n");
}
