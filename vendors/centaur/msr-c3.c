/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Centaur specific parts.
 */
#include <stdio.h>
#include <x86info.h>
#include "centaur.h"

void dump_C3_MSR (struct cpudata *cpu)
{
	printf("FCR: ");
	dumpmsr(cpu->number, 0x1107, 32, true);

	printf("Power management: ");
	if (cpu->model==6 || cpu->model==7) {
		printf("Longhaul\n");
		decode_c3_power(cpu, C3_LONGHAUL);
	}

	if (cpu->model==8 || cpu->model==9) {
		printf("Powersaver\n");
		decode_c3_power(cpu, C3_POWERSAVER);
	}
}
