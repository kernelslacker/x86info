/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Generic routines for reading MSRs.
 */

#include <stdio.h>
#include <x86info.h>

/* Print the MSR in binary, preceded by its address and hex value if labelled. */
void dumpmsr(int cpu, unsigned int msr, int size, bool labelled)
{
	unsigned long long val = 0;

	if (read_msr(cpu, msr, &val) != 1) {
		printf("Couldn't read MSR 0x%x\n", msr);
		return;
	}
	if (size == 32) {
		if (labelled)
			printf("MSR: 0x%08x=0x%08lx : ", msr, (unsigned long) val);
		binary32(val);
	}
	if (size == 64) {
		if (labelled)
			printf("MSR: 0x%08x=0x%016llx : ", msr, val);
		binary64(val);
	}
}
