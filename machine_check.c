/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Machine check bank dump shared by the vendors.
 */

#include <stdio.h>
#include <x86info.h>

#define MC_CTL 0x0400
#define MC_STATUS 0x0401
#define MC_ADDR 0x402
#define MC_MISC 0x403

void dump_mc_banks(int cpunum, unsigned int banks)
{
	unsigned int i;

	for (i = 0; i < banks; i++) {
		printf("Bank: %u (0x%x)\n", i, (unsigned int)MC_CTL+i*4);
		printf("MC%uCTL:    ", i);
		dumpmsr(cpunum, MC_CTL+i*4, 64, false);
		printf("MC%uSTATUS: ", i);
		dumpmsr(cpunum, MC_STATUS+i*4, 64, false);
		printf("MC%uADDR:   ", i);
		dumpmsr(cpunum, MC_ADDR+i*4, 64, false);
		printf("MC%uMISC:   ", i);
		dumpmsr(cpunum, MC_MISC+i*4, 64, false);
		printf("\n");
	}
}
