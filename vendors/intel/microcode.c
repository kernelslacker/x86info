/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel P6 microcode information.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <x86info.h>
#include "intel.h"

void decode_microcode(struct cpudata *cpu)
{
	unsigned long long val = 0;

	if (!user_is_root)
		return;

	if (cpu->family < 6)
		return;

	if (read_msr (cpu->number, MSR_IA32_UCODE_REV, &val) == 1) {
		unsigned int ver;

		ver = val >> 32;
		if (ver > 0)
			printf("Microcode version: 0x%x\n", ver);
		printf("\n");
	}
}
