/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  SiS specific parts.
 */
#include <stdio.h>
#include <x86info.h>

void identify_sis(struct cpudata *cpu)
{
	switch (tuple(cpu)) {
		case 0x505:	add_to_cpuname("SiS55x");
					break;
		default:	add_to_cpuname("Unknown CPU");
					break;
	}
}
