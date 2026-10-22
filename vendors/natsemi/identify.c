/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  NatSemi specific parts.
 */
#include <stdio.h>
#include <x86info.h>

void identify_natsemi(struct cpudata *cpu)
{
	switch (tuple(cpu) & 0xff0) {
		case 0x540:	add_to_cpuname("Geode GX1");
					break;
		default:	add_to_cpuname("Unknown CPU");
					break;
	}
}
