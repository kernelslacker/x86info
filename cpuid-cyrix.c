/*
 *  $Id: cpuid-cyrix.c,v 1.8 2001/08/10 10:25:25 davej Exp $
 *  This file is part of x86info. 
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Cyrix bits.
 */

#include <stdio.h>
#include "x86info.h"

extern int show_cacheinfo;
extern int show_registers;

/* Decode TLB and cache info descriptors */
void decode_cyrix_tlb (int x)
{
	switch (x & 0xff) {
	case 0:
		break;
	case 0x70:
		printf ("TLB: 32 entries 4-way associative 4KB pages\n");
		break;
	case 0x80:
		printf ("L1 Cache: 16KB 4-way associative 16 bytes/line\n");
		break;
	}
}

/* Cyrix-specific information */
void Identify_Cyrix (int cpunum, unsigned int maxi, unsigned int maxei, struct cpudata *cpu)
{
	int i;
	unsigned long eax, ebx, ecx, edx;

	cpu->vendor = VENDOR_CYRIX;

	/* Do standard stuff */
	if (maxi >= 1) {
		cpuid (cpunum, 1, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;

		switch (cpu->family) {
			case 4:	if (cpu->model==5) sprintf (cpu->name, "%s", "MediaGX");	break;
					sprintf (cpu->name, "%s", "Unknown CPU");
					break;
			case 5:	switch (cpu->model) {
						case 2:	sprintf (cpu->name, "%s", "6x86");	break;
						case 4:	sprintf (cpu->name, "%s", "GXm");		break;
						default:sprintf (cpu->name, "%s", "Unknown CPU"); break;
					}
					break;
			case 6:	if (cpu->model==0) sprintf (cpu->name, "%s", "6x86/MX");	break;
					sprintf (cpu->name, "%s", "Unknown CPU");
					break;
		}

		/* Check for presence of extended info */
		if (maxei < 0x80000000)
			return;

		if (maxei >= 0x80000001) {
			cpuid (cpunum, 0x80000001, &eax, &ebx, &ecx, &edx);
			cpu->stepping = eax & 0xf;
			cpu->model = (eax >> 4) & 0xf;
			cpu->family = (eax >> 8) & 0xf;

			switch (cpu->family) {
				case 4:	sprintf (cpu->name, "MediaGX");		break;
				case 5:	sprintf (cpu->name, "6x86/GXm");	break;
				case 6:	sprintf (cpu->name, "6x86/MX");		break;
			}
		}

		if (maxei >= 0x80000002) {
			/* Processor identification string */
			char namestring[49], *cp;
			unsigned int j;
			cp = namestring;
			for (j = 0x80000002; j <= 0x80000004; j++) {
				cpuid (cpunum, j, &eax, &ebx, &ecx, &edx);

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
}

void display_Cyrix_info(int cpunum, unsigned int maxi, unsigned int maxei, struct cpudata *cpu)
{
	unsigned int i, ntlb;
	unsigned long eax, ebx, ecx, edx;

	printf ("Cyrix-specific functions\n");
	if (maxei >= 0x80000000 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx,
				edx);
		}
	}

	decode_feature_flags (cpu, edx);

	printf ("TLB & L1 Cache info\n");
	if (maxi >= 2 && show_cacheinfo) {
		/* TLB and L1 Cache info */
		ntlb = 255;
		for (i = 0; i < ntlb; i++) {
			cpuid (cpunum, 2, &eax, &ebx, &ecx, &edx);
			ntlb = eax & 0xff;
			decode_cyrix_tlb (eax >> 8);
			decode_cyrix_tlb (eax >> 16);
			decode_cyrix_tlb (eax >> 24);

			/* ebx and ecx are reserved */
			if ((edx & 0x80000000) == 0) {
				decode_cyrix_tlb (edx);
				decode_cyrix_tlb (edx >> 8);
				decode_cyrix_tlb (edx >> 16);
				decode_cyrix_tlb (edx >> 24);
			}
		}
	}

	printf ("TLB & L1 Cache info from extended info\n");
	if (maxei >= 0x80000005 && show_cacheinfo) {
		/* TLB and L1 Cache info */
		ntlb = 255;
		for (i = 0; i < ntlb; i++) {
			cpuid (cpunum, 0x80000005, &eax, &ebx, &ecx, &edx);
			ntlb = eax & 0xff;
			decode_cyrix_tlb (ebx >> 8);
			decode_cyrix_tlb (ebx >> 16);
			decode_cyrix_tlb (ebx >> 24);

			/* eax and edx are reserved */
			if ((ecx & 0x80000000) == 0) {
				decode_cyrix_tlb (ecx);
				decode_cyrix_tlb (ecx >> 8);
				decode_cyrix_tlb (ecx >> 16);
				decode_cyrix_tlb (ecx >> 24);
			}
		}
	}
}

