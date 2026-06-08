/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include <x86info.h>

static char *sis_nameptr;
#define add_to_cpuname(x) CPU_NAME_APPEND(sis_nameptr, cpu, x)

void identify_sis(struct cpudata *cpu)
{
	sis_nameptr = cpu->name;

	switch (tuple(cpu)) {
		case 0x505:	add_to_cpuname("SiS55x");
					break;
		default:	add_to_cpuname("Unknown CPU");
					break;
	}
}
