/*
 *  $Id: identify.c,v 1.10 2001/08/10 10:35:36 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include "x86info.h"

extern int silent;

void identify (int cpunum)
{
	struct cpudata cpu;
	unsigned long maxi, maxei, eax, ebx, ecx, edx;

	/* Dump all the CPUID results in raw hex */
	cpuid (cpunum, 0, &maxi, NULL, NULL, NULL);
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */

	cpuid (cpunum, 0, &eax, &ebx, &ecx, &edx);
	cpuid (cpunum, 0x80000000, &maxei, NULL, NULL, NULL);
	
	switch (ebx) {
		case 0x756e6547:/* Intel */	Identify_Intel (cpunum, maxi, &cpu);		break;
		case 0x68747541:/* AMD */	Identify_AMD (cpunum, maxi, &cpu);			break;
		case 0x69727943:/* Cyrix */	Identify_Cyrix (cpunum, maxi, maxei, &cpu);	break;
		case 0x746e6543:/* IDT */	Identify_IDT (cpunum, maxi, maxei, &cpu);					break;
		case 0x52697365:/* Rise This should be checked. Why 2 ? */
		case 0x65736952:/* Rise */	doRise (cpunum, maxi, &cpu);				break;
		default:					printf ("Unknown vendor\n");				return;
	}

	if (!silent) {

		switch (cpu.vendor) {
			case VENDOR_AMD:
				printf ("Family: %d Model: %d Stepping: %d [%s]\n",
					cpu.family, cpu.model, cpu.stepping, cpu.name);
				display_AMD_info (cpunum, maxei, &cpu);
				break;

			case VENDOR_CYRIX:
				printf ("Family: %d Model: %d Stepping: %d [%s]\n",
					cpu.family, cpu.model, cpu.stepping, cpu.name);
				display_Cyrix_info (cpunum, maxi, maxei, &cpu);
				break;

			case VENDOR_INTEL:
				printf ("Family: %d Model: %d Stepping: %d Type: %d [%s]\n",
					cpu.family, cpu.model, cpu.stepping, cpu.type, cpu.name);
				display_Intel_info (cpunum, maxi, &cpu);
				break;

			default:
				break;
		}
	}
}
