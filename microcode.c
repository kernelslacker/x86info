/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Microcode revision: from sysfs if available, else from MSR 0x8b.
 */

#include <stdio.h>
#include <x86info.h>
#include <intel.h>

static int sysfs_microcode_version(struct cpudata *cpu, unsigned long long *ver)
{
	char path[80];
	FILE *f;
	int ret;

	snprintf(path, sizeof(path),
		 "/sys/devices/system/cpu/cpu%u/microcode/version", cpu->number);
	f = fopen(path, "r");
	if (!f)
		return 0;
	ret = fscanf(f, "%llx", ver) == 1;
	fclose(f);
	return ret;
}

void decode_microcode(struct cpudata *cpu)
{
	unsigned long long ver = 0;

	if (!sysfs_microcode_version(cpu, &ver)) {
		/* The MSR fallback is AMD only */
		if (cpu->vendor == VENDOR_INTEL || !user_is_root ||
		    read_msr(cpu->number, MSR_IA32_UCODE_REV, &ver) != 1)
			return;
	}

	if (ver > 0)
		printf("Microcode version: 0x%llx\n", ver);
	printf("\n");
}
