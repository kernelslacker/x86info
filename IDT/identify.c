/*
 *  $Id: identify.c,v 1.9 2001/09/14 15:11:39 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  IDT/Centaur specific parts.
 */
#include <stdio.h>
#include "../x86info.h"

extern int show_cacheinfo;
extern int show_registers;

void Identify_IDT (unsigned int maxi, unsigned int maxei, struct cpudata *cpu)
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
			case 4:	sprintf (cpu->name, "%s", "Winchip C6");	break;
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
				case 6:	sprintf (cpu->name, "%s", "VIA Cyrix III");	break;
				case 7:	nameptr += sprintf (cpu->name, "%s", "VIA C3");
					if (cpu->stepping>7)
						sprintf(nameptr, "%s", " \"Ezra\"");
					break;
				default:
					printf ("Unknown CPU");
			}
			break;

		default:
			sprintf (cpu->name, "%s", "Unknown CPU");
			break;
		}
	}

	if (maxei >= 0x80000002) {
		/* Processor identification string */
		char namestring[49], *cp;
		unsigned int j;
		unsigned int i;
		cp = namestring;
		for (j = 0x80000002; j <= 0x80000004; j++) {
			cpuid (cpu->number, j, &eax, &ebx, &ecx, &edx);

			for (i = 0; i < 4; i++)
				*cp++ = eax >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = ebx >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = ecx >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = edx >> (8 * i);
		}
		*cp++ = '\n';
		sprintf (cpu->name, "%s", namestring);
	}
}


void decode_IDT_cacheinfo(unsigned int maxei, struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	printf ("\n");
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
	unsigned long eax, ebx, ecx, edx;
	unsigned long long val=0;
	int longhaul=0;

	if (maxei != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (cpu->number, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx,
				edx);
		}
	}

	/* Check for presence of extended info */
	if (maxei == 0)
		return;

	if (maxei >= 0x80000001) {
		cpuid (cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
		decode_feature_flags (cpu, edx);
	}

	if (show_cacheinfo)
		decode_IDT_cacheinfo(maxei, cpu);

	/* Decode longhaul info. */
	if (cpu->family != 6)	/* only interested in VIA Cyrix */
		return;

	printf ("Longhaul v");
	if (cpu->model==6 || (cpu->model==7 && cpu->stepping==0))
		printf ("1.0");
	if (cpu->model==7 && cpu->stepping>0) {
		printf ("2.0");
		longhaul=2;
	}
	if (cpu->model==8)
		printf ("3.0");
	printf (" present\n");

	if (longhaul==2 && rdmsr(cpu->number, 0x110A, &val) == 1) {
		dumpmsr (cpu->number, 0x110A, 64);
		if (val & 1)
			printf ("\tSoftVID support\n");
		if (val & 2)
			printf ("\tSoftBSEL support\n");
		if (val==0)
			printf ("\tSoftware clock multiplier only: No Softvid\n");

		printf ("\tRevision key: %d\n", (val & (1<<7|1<<6|1<<5|1<<4)) >> 4);
		if (val & (1<<8))
			printf ("\tEnableSoftBusRatio=Enabled\n");
		if (val & (1<<9))
			printf ("\tEnableSoftVid=Enabled\n");
		if (val & (1<<10))
			printf ("\tEnableSoftBSEL=Enabled\n");

		printf ("\tSoftBusRatio=%d\n", val & 1<<14);
		if (val & 0xf)
			printf ("\tVRM Rev=%s\n",
				((val & 1<<15)) ? "Mobile VRM" : "VRM 8.5");

		val = val>>32;
		binary32 (val);
		printf ("\tMaxMHzBR: %s%s%s%s\n",
			(((val & (1<<3)) >>3) ? "1" : "0"),
			(((val & (1<<2)) >>2) ? "1" : "0"),
			(((val & (1<<1)) >>1) ? "1" : "0"),
			(((val & (1<<0)) >>0) ? "1" : "0"));
		printf ("\tMaximumVID: %s%s%s%s%s\n",
			(((val & (1<<8)) >>8) ? "1" : "0"),
			(((val & (1<<7)) >>7) ? "1" : "0"),
			(((val & (1<<6)) >>6) ? "1" : "0"),
			(((val & (1<<5)) >>5) ? "1" : "0"),
			(((val & (1<<4)) >>4) ? "1" : "0"));
		printf ("\tMaxMHzFSB: %s%s\n",
			(((val & (1<<10)) >>10) ? "1" : "0"),
			(((val & (1<<9))  >>9) ? "1" : "0"));
		printf ("\tMinMHzBR: %s%s%s%s\n",
			(((val & (1<<19)) >>19) ? "1" : "0"),
			(((val & (1<<18)) >>18) ? "1" : "0"),
			(((val & (1<<17)) >>17) ? "1" : "0"),
			(((val & (1<<16)) >>16) ? "1" : "0"));
		printf ("\tMinimumVID: %s%s%s%s\n",
			(((val & (1<<23)) >>23) ? "1" : "0"),
			(((val & (1<<22)) >>22) ? "1" : "0"),
			(((val & (1<<21)) >>21) ? "1" : "0"),
			(((val & (1<<20)) >>20) ? "1" : "0"));
		printf ("\tMinMHzFSB: %s%s\n",
			(((val & (1<<26)) >>26) ? "1" : "0"),
			(((val & (1<<25)) >>25) ? "1" : "0"));
	}
}
