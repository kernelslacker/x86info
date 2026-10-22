/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include <x86info.h>

void identify_rise(struct cpudata *cpu)
{
	switch (tuple(cpu) & 0xff0) {
	case 0x500:	add_to_cpuname("iDragon (0.25um)");
			break;
	case 0x520:	add_to_cpuname("iDragon (0.18um)");
			break;
	case 0x580:	add_to_cpuname("iDragon II (0.25um)");
			break;
	case 0x590:	add_to_cpuname("iDragon II (0.18um)");
			break;
	default:	add_to_cpuname("Unknown CPU");
			break;
	}
}
