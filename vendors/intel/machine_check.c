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

#define MCG_CAP 0x0179
#define MCG_CTL_PBIT	8
#define MCG_EXT_PBIT	9
#define MCG_STATUS 0x17a
#define MCG_CTL 0x17b
#define MCG_EXT 0x180

#define MC_CTL 0x0400
#define MC_STATUS 0x0401
#define MC_ADDR 0x402

static int nr_msr_banks;

void decode_intel_machine_check(int cpunum, int family)
{
	unsigned long long val;
	int i;

	if (!user_is_root)
		return;

	if (read_msr(cpunum, MCG_CAP, &val) != 1)
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

	for (i=0; i < nr_msr_banks; i++) {
		printf("Bank: %d (0x%x)\n", i, (unsigned int)MC_CTL+i*4);
		printf("MC%dCTL:    ", i);
		dumpmsr_bin (cpunum, MC_CTL+i*4, 64);
		printf("MC%dSTATUS: ", i);
		dumpmsr_bin (cpunum, MC_STATUS+i*4, 64);
		printf("MC%dADDR:   ", i);
		dumpmsr_bin (cpunum, MC_ADDR+i*4, 64);
	}
	printf("\n");
}
