/*
 *  $Id: identify.c,v 1.19 2001/12/11 01:03:20 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  IDT/Centaur specific parts.
 */
#include <stdio.h>
#include "../x86info.h"
#include "IDT.h"

void Identify_IDT (unsigned int maxi, struct cpudata *cpu)
{
	char *nameptr;
	unsigned long eax, ebx, ecx, edx;

	nameptr = cpu->name;

	cpu->vendor = VENDOR_CENTAUR;

	/* Do standard stuff */
	if (maxi >= 1) {
		cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;

		switch (cpu->family) {
		case 5:
			switch (cpu->model) {
			case 4:	sprintf (cpu->name, "%s", "Winchip C6");
					break;
			case 8:
				switch (cpu->stepping) {
				default:
					sprintf (cpu->name, "%s", "Winchip 2");
					break;
				case 7:
				case 8:
				case 9:
					sprintf (cpu->name, "%s", "Winchip 2A");
					break;
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
					sprintf (cpu->name, "%s", "Winchip 2B");
					break;
				}
				break;
			case 9:
				sprintf (cpu->name, "%s", "Winchip 3");
				break;
			default:
				sprintf (cpu->name, "%s", "unknown CPU");
				break;
			}
			break;
		
		/* Family 6 is when VIA bought out Cyrix & IDT
		 * This is the CyrixIII family. */
		case 6:
			switch (cpu->model) {
				case 6:	sprintf (cpu->name, "%s", "VIA Cyrix III");
						break;
				case 7:	nameptr += sprintf (cpu->name, "%s", "VIA C3");
						if (cpu->stepping>7)
							sprintf(nameptr, "%s", " \"Ezra\"");
						break;
				default:
						printf ("Unknown CPU");
						break;
			}
			break;

		default:
			sprintf (cpu->name, "%s", "Unknown CPU");
			break;
		}
	}
}


void decode_IDT_cacheinfo(unsigned int maxei, struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	if (maxei >= 0x80000005) {
		/* TLB and cache info */
		cpuid (cpu->number, 0x80000005, &eax, &ebx, &ecx, &edx);
		printf ("Instruction TLB: %ld-way associative. %ld entries.\n", (ebx >> 8) & 0xff, ebx & 0xff);
		printf ("Data TLB: %ld-way associative. %ld entries.\n", ebx >> 24, (ebx >> 16) & 0xff);
		printf ("L1 Data cache:\n\tSize: %ldKb\t%ld-way associative.\n\tlines per tag=%ld\tline size=%ld bytes.\n",
			ecx >> 24, (ecx >> 16) & 0xff, (ecx >> 8) & 0xff, ecx & 0xff);
		printf ("L1 Instruction cache:\n\tSize: %ldKb\t%ld-way associative.\n\tlines per tag=%ld\tline size=%ld bytes.\n",
			edx >> 24, (edx >> 16) & 0xff, (edx >> 8) & 0xff, edx & 0xff);
	}

	/* check on-chip L2 cache size */
	if (maxei >= 0x80000006) {
		cpuid (cpu->number, 0x80000006, &eax, &ebx, &ecx, &edx);
		if ((cpu->family==6) && (cpu->model==7 || cpu->model==8))
			printf ("L2 (on CPU) cache:\n\tSize: %ldKb\t%ld-way associative.\n\tlines per tag=%ld\tline size=%ld bytes.\n",
				ecx >> 24, (ecx >> 16) & 0x0f, (ecx >> 8) & 0x0f, ecx & 0xff);
		else
			printf ("L2 (on CPU) cache:\n\tSize: %ldKb\t%ld-way associative.\n\tlines per tag=%ld\tline size=%ld bytes.\n",
				ecx >> 16, (ecx >> 12) & 0x0f, (ecx >> 8) & 0x0f, ecx & 0xff);
	}
}


void display_IDT_info(unsigned int maxei, struct cpudata *cpu)
{
	unsigned int i;
	unsigned long eax, ebx, ecx, edx, tmp=0;

	if (maxei != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (cpu->number, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx,
				edx);
		}
		printf ("\n");
	}

	printf ("Family: %d Model: %d Stepping: %d [%s]\n",
		cpu->family, cpu->model, cpu->stepping, cpu->name);
	get_model_name (maxei, cpu);


	/* Check for presence of extended info */
	if (maxei == 0)
		return;

	cpuid (cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	if (maxei >= 0x80000001)
		cpuid (cpu->number, 0x80000001, &eax, &ebx, &ecx, &tmp);
	decode_feature_flags (cpu, edx, tmp);

	if (show_cacheinfo)
		decode_IDT_cacheinfo(maxei, cpu);

	if (cpu->family == 6 && show_registers)
		dump_C3_MSR(cpu);
}
