/*
 *  $Id: identify.c,v 1.8 2002/07/12 00:56:19 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include "../x86info.h"

void Identify_RiSE (struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;
	cpu->vendor = VENDOR_RISE;

	/* Do standard stuff */
	if (cpu->maxi < 1)
		return;

	cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;

	switch (tuple(cpu) & 0xff0) {
		case 0x500:	sprintf (cpu->name, "%s", "iDragon (0.25um)");
					break;
		case 0x520:	sprintf (cpu->name, "%s", "iDragon (0.18um)");
					break;
		case 0x580:	sprintf (cpu->name, "%s", "iDragon II (0.25um)");
					break;
		case 0x590:	sprintf (cpu->name, "%s", "iDragon II (0.18um)");
					break;
		default:	sprintf (cpu->name, "%s", "Unknown CPU");
					break;
	}
}


void display_RiSE_info(struct cpudata *cpu)
{
	unsigned int i;
	unsigned long eax, ebx, ecx, edx, tmp=0;

	printf ("Family: %d Model: %d Stepping: %d [%s]\n",
		cpu->family, cpu->model, cpu->stepping, cpu->name);
	get_model_name (cpu);

	if (cpu->maxi != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x00000000; i <= cpu->maxi; i++) {
			cpuid (cpu->number, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
		}
		printf ("\n");
	}

	if (cpu->maxei != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= cpu->maxei; i++) {
			cpuid (cpu->number, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
		}
		printf ("\n");
	}

	cpuid (cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	if (cpu->maxei >= 0x80000001)
		cpuid (cpu->number, 0x80000001, &eax, &ebx, &ecx, &tmp);
	decode_feature_flags (cpu, edx, tmp);
}
