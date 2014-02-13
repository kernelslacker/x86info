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
	if (!user_is_root)
		return;

	printf("FCR: ");
	dumpmsr (cpu->number, 0x1107, 32);

	printf("Power management: ");
	if (cpu->model==6 || cpu->model==7) {
		printf("Longhaul\n");
		decode_longhaul(cpu);
	}

	if (cpu->model==8 || cpu->model==9) {
		printf("Powersaver\n");
		decode_powersaver(cpu);
	}
}
