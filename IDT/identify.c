/*
 *  $Id: identify.c,v 1.27 2002/11/11 20:02:55 davej Exp $
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

void Identify_IDT (struct cpudata *cpu)
{
	char *nameptr;
	unsigned long eax, ebx, ecx, edx;

	nameptr = cpu->name;

	cpu->vendor = VENDOR_CENTAUR;

	/* Do standard stuff */
	if (cpu->maxi < 1)
		return;

	cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;

	switch (tuple(cpu) & 0xff0) {
		case 0x540:	sprintf (cpu->name, "%s", "Winchip C6");
					break;
		case 0x580 ... 0x586:
			sprintf (cpu->name, "%s", "Winchip 2");
			break;
		case 0x587 ... 0x589:
			sprintf (cpu->name, "%s", "Winchip 2A");
			break;
		case 0x58A ... 0x58F:
			sprintf (cpu->name, "%s", "Winchip 2B");
			break;
		case 0x590:
			sprintf (cpu->name, "%s", "Winchip 3");
			break;
		
		/* Family 6 is when VIA bought out Cyrix & IDT
		 * This is the CyrixIII family. */
		case 0x660:	sprintf (cpu->name, "%s", "VIA Cyrix III");
					break;
		case 0x670:	nameptr += sprintf (cpu->name, "%s", "VIA C3");
					if (cpu->stepping>7)
						sprintf(nameptr, "%s", " \"Ezra\"");
					break;
		case 0x680:	nameptr += sprintf (cpu->name, "%s", "VIA Ezra-T");
					break;
		default:	printf ("Unknown CPU");
					break;
	}
}


void decode_IDT_cacheinfo(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	if (cpu->maxei >= 0x80000005) {
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
	if (cpu->maxei >= 0x80000006) {
		cpuid (cpu->number, 0x80000006, &eax, &ebx, &ecx, &edx);
		if ((cpu->family==6) && (cpu->model==7 || cpu->model==8)) {
			printf ("L2 cache size errata detected. Using workaround\n");
			printf ("L2 (on CPU) cache:\n\tSize: %ldKb\t%ld-way associative.\n\tlines per tag=%ld\tline size=%ld bytes.\n",
				ecx >> 24, (ecx >> 16) & 0x0f, (ecx >> 8) & 0x0f, ecx & 0xff);
		} else
			printf ("L2 (on CPU) cache:\n\tSize: %ldKb\t%ld-way associative.\n\tlines per tag=%ld\tline size=%ld bytes.\n",
				ecx >> 16, (ecx >> 12) & 0x0f, (ecx >> 8) & 0x0f, ecx & 0xff);
	}
}


void display_IDT_info(struct cpudata *cpu)
{
	printf ("Family: %d Model: %d Stepping: %d\n",
		cpu->family, cpu->model, cpu->stepping);
	printf ("CPU Model : %s\n", cpu->name);
	get_model_name (cpu);


	/* Check for presence of extended info */
	decode_feature_flags (cpu);

	if (cpu->maxei == 0)
		return;

	if (show_cacheinfo)
		decode_IDT_cacheinfo(cpu);

	if (cpu->family == 6 && show_registers)
		dump_C3_MSR(cpu);
}
