/*
 *  $Id: identify.c,v 1.22 2002/07/12 01:28:36 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include "x86info.h"

void identify (struct cpudata *cpu)
{
	unsigned long maxi, maxei, eax, ebx, ecx, edx;

	cpuid (cpu->number, 0, &maxi, NULL, NULL, NULL);
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */
	cpu->maxi = maxi;

	cpuid (cpu->number, 0, &eax, &ebx, &ecx, &edx);
	cpuid (cpu->number, 0x80000000, &maxei, NULL, NULL, NULL);
	cpu->maxei = maxei;
	
	switch (ebx) {
		case 0x756e6547:/* Intel */	Identify_Intel (cpu);
									break;
		case 0x68747541:/* AMD */	Identify_AMD (cpu);
									break;
		case 0x69727943:/* Cyrix */	Identify_Cyrix (cpu);
									break;
		case 0x746e6543:/* IDT */	Identify_IDT (cpu);
									break;
		case 0x646f6547:/* Natsemi Geode*/
									Identify_NatSemi (cpu);
									break;
		case 0x52697365:/* Rise This should be checked. Why 2 ? */
		case 0x65736952:/* Rise */	Identify_RiSE (cpu);
									break;
		default:					printf ("Unknown vendor\n");				return;
	}

	if (show_registers) {
		dumpregs (cpu->number, 0, maxi);
		if (cpu->maxei >=0x80000000)
			dumpregs (cpu->number, 0x80000000, maxei);
	}

	if (!silent) {

		switch (cpu->vendor) {
			case VENDOR_AMD:
				display_AMD_info (cpu);
				break;

			case VENDOR_CYRIX:
				display_Cyrix_info (cpu);
				break;

			case VENDOR_CENTAUR:
				display_IDT_info (cpu);
				break;

			case VENDOR_INTEL:
				display_Intel_info (cpu);
				break;

			case VENDOR_NATSEMI:
				display_NatSemi_info (cpu);
				break;

			case VENDOR_RISE:
				display_RiSE_info (cpu);
				break;

			default:
				break;
		}
	}
}
