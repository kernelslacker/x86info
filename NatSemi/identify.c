/*
 *  $Id: identify.c,v 1.6 2002/11/11 20:02:55 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  NatSemi specific parts.
 */
#include <stdio.h>
#include "../x86info.h"

static char *NatSemi_nameptr;
#define add_to_cpuname(x)   NatSemi_nameptr += snprintf(NatSemi_nameptr, sizeof(x), "%s", x);

void Identify_NatSemi (struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	NatSemi_nameptr = cpu->name;
	cpu->vendor = VENDOR_NATSEMI;

	/* Do standard stuff */
	if (cpu->maxi < 1)
		return;

	cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;

	switch (tuple(cpu) & 0xff0) {
		case 0x540:	add_to_cpuname("Geode GX1");
					break;
		default:	add_to_cpuname("Unknown CPU");
					break;
	}
}


void display_NatSemi_info(struct cpudata *cpu)
{
	printf ("Family: %u Model: %u Stepping: %u\n",
		cpu->family, cpu->model, cpu->stepping);
	printf ("CPU Model : %s\n", cpu->name);
	get_model_name (cpu);

	decode_feature_flags (cpu);
}
