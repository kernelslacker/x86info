/*
 *  $Id: identify.c,v 1.1 2003/04/11 00:10:42 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include "../x86info.h"

void Identify_SiS (struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;
	cpu->vendor = VENDOR_SIS;

	/* Do standard stuff */
	if (cpu->maxi < 1)
		return;

	cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;

	switch (tuple(cpu)) {
		case 0x505:	sprintf (cpu->name, "%s", "SiS55x");
					break;
		default:	sprintf (cpu->name, "%s", "Unknown CPU");
					break;
	}
}


void display_SiS_info(struct cpudata *cpu)
{
	printf ("Family: %d Model: %d Stepping: %d\n",
		cpu->family, cpu->model, cpu->stepping);
	printf ("CPU Model : %s\n", cpu->name);
	get_model_name (cpu);

	decode_feature_flags (cpu);
}
