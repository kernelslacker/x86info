/*
 *  $Id: identify.c,v 1.8 2001/12/11 00:58:06 davej Exp $
 *  This file is part of x86info. 
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Cyrix bits.
 */

#include <stdio.h>
#include "../x86info.h"
#include "Cyrix.h"

/* Decode TLB and cache info descriptors */
void decode_Cyrix_TLB (int x)
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
void Identify_Cyrix (unsigned int maxi, unsigned int maxei, struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	cpu->vendor = VENDOR_CYRIX;

	/* Do standard stuff */
	if (maxi >= 1) {
		cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;

		switch (cpu->family) {
			case 4:	if (cpu->model==5) {
						sprintf (cpu->name, "%s", "MediaGX");
						break;
					}
					sprintf (cpu->name, "%s", "Unknown CPU");
					break;
			case 5:	switch (cpu->model) {
						case 2:	sprintf (cpu->name, "%s", "6x86");
								break;
						case 4:	sprintf (cpu->name, "%s", "GXm");
								break;
						default:sprintf (cpu->name, "%s", "Unknown CPU");
								break;
					}
					break;
			case 6:	switch (cpu->model) {
						case 0:
							sprintf (cpu->name, "%s", "6x86/MX");
							break;
						case 2:
							sprintf (cpu->name, "%s", "MII");
							break;
						default:
							sprintf (cpu->name, "%s", "Unknown CPU");
							break;
					}
					break;
		}

		/* Check for presence of extended info */
		if (maxei < 0x80000000)
			return;

		if (maxei >= 0x80000001) {
			cpuid (cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
			cpu->stepping = eax & 0xf;
			cpu->model = (eax >> 4) & 0xf;
			cpu->family = (eax >> 8) & 0xf;

			switch (cpu->family) {
				case 4:	sprintf (cpu->name, "MediaGX");
						break;
				case 5:	sprintf (cpu->name, "6x86/GXm");
						break;
				case 6:	sprintf (cpu->name, "6x86/MX");
						break;
			}
		}

	}
}

void display_Cyrix_info(unsigned int maxi, unsigned int maxei, struct cpudata *cpu)
{
	unsigned int i, ntlb;
	unsigned long eax, ebx, ecx, edx;

	printf ("Family: %d Model: %d Stepping: %d [%s]\n",
		cpu->family, cpu->model, cpu->stepping, cpu->name);
	get_model_name (maxei, cpu);

	if (maxei >= 0x80000000 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (cpu->number, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx,
				edx);
		}
	}

	decode_feature_flags (cpu, edx, 0);

	printf ("TLB & L1 Cache info\n");
	if (maxi >= 2 && show_cacheinfo) {
		/* TLB and L1 Cache info */
		ntlb = 255;
		for (i = 0; i < ntlb; i++) {
			cpuid (cpu->number, 2, &eax, &ebx, &ecx, &edx);
			ntlb = eax & 0xff;
			decode_Cyrix_TLB (eax >> 8);
			decode_Cyrix_TLB (eax >> 16);
			decode_Cyrix_TLB (eax >> 24);

			/* ebx and ecx are reserved */
			if ((edx & 0x80000000) == 0) {
				decode_Cyrix_TLB (edx);
				decode_Cyrix_TLB (edx >> 8);
				decode_Cyrix_TLB (edx >> 16);
				decode_Cyrix_TLB (edx >> 24);
			}
		}
	}

	printf ("TLB & L1 Cache info from extended info\n");
	if (maxei >= 0x80000005 && show_cacheinfo) {
		/* TLB and L1 Cache info */
		ntlb = 255;
		for (i = 0; i < ntlb; i++) {
			cpuid (cpu->number, 0x80000005, &eax, &ebx, &ecx, &edx);
			ntlb = eax & 0xff;
			decode_Cyrix_TLB (ebx >> 8);
			decode_Cyrix_TLB (ebx >> 16);
			decode_Cyrix_TLB (ebx >> 24);

			/* eax and edx are reserved */
			if ((ecx & 0x80000000) == 0) {
				decode_Cyrix_TLB (ecx);
				decode_Cyrix_TLB (ecx >> 8);
				decode_Cyrix_TLB (ecx >> 16);
				decode_Cyrix_TLB (ecx >> 24);
			}
		}
	}
}

