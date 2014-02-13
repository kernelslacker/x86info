/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Generic routines for reading MSRs.
 */

#include <stdio.h>
#include <x86info.h>

void dumpmsr(int cpu, unsigned int msr, int size)
{
	unsigned long long val=0;

	if (read_msr(cpu, msr, &val) == 1) {
		if (size == 32){
			printf("MSR: 0x%08x=0x%08lx : ", msr, (unsigned long) val);
			binary32(val);
		}
		if (size == 64) {
			printf("MSR: 0x%08x=0x%016llx : ", msr, val);
			binary64(val);
		}
		return;
	}
	printf("Couldn't read MSR 0x%x\n", msr);
}

void dumpmsr_bin(int cpu, unsigned int msr, int size)
{
	unsigned long long val=0;

	if (read_msr(cpu, msr, &val) == 1) {
		if (size == 32)
			binary32(val);
		if (size == 64)
			binary64(val);
		return;
	}
	printf("Couldn't read MSR 0x%x\n", msr);
}
