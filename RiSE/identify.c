/*
 *  $Id: identify.c,v 1.11 2002/11/11 20:02:56 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include "../x86info.h"

static char *rise_nameptr;
#define add_to_cpuname(x)   rise_nameptr += snprintf(rise_nameptr, sizeof(x), "%s", x);

void Identify_RiSE (struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	rise_nameptr = cpu->name;
	cpu->vendor = VENDOR_RISE;

	/* Do standard stuff */
	if (cpu->maxi < 1)
		return;

	cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;

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


void display_RiSE_info(struct cpudata *cpu)
{
	printf ("Family: %u Model: %u Stepping: %u\n",
		cpu->family, cpu->model, cpu->stepping);
	printf ("CPU Model : %s\n", cpu->name);
	get_model_name (cpu);

	decode_feature_flags (cpu);
}
