/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Dump machine check registers.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <x86info.h>
#include "intel.h"
#include <machine_check.h>

void decode_intel_machine_check(unsigned int cpunum, int family)
{
	unsigned long long val;
	int nr_msr_banks;

	if (!read_msr(cpunum, MCG_CAP, &val))
		return;

	nr_msr_banks = val & 0xff;

	printf("Machine check MSRs:\n");
	printf("Number of reporting banks : %d\n", nr_msr_banks);

	if (family == 0xf) {
		if ((val & (1<<MCG_EXT_PBIT))) {
			int extcount = 0;

			extcount = (val >> 16) & 0xff;
			printf("Number of extended MC registers : %d\n\n", extcount);
		}
		else
			printf("Erk, MCG_EXT not present! :%016llx:\n", val);
	}

	dump_mc_banks(cpunum, nr_msr_banks);
}
