/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Microcode revision: from sysfs if available, else from MSR 0x8b.
 */

#include <stdio.h>
#include <x86info.h>
#include <intel.h>

void decode_microcode(struct cpudata *cpu)
{
	unsigned long long ver = 0;

	if (!host_microcode_version(cpu->number, &ver)) {
		/* The MSR fallback is AMD only */
		if (cpu->vendor == VENDOR_INTEL || !is_root() ||
		    read_msr(cpu->number, MSR_IA32_UCODE_REV, &ver) != 1)
			return;
	}

	if (ver > 0)
		printf("Microcode version: 0x%llx\n", ver);
	printf("\n");
}
