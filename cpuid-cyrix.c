/*
 *  $Id: cpuid-cyrix.c,v 1.7 2001/03/11 04:12:54 davej Exp $
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
void docyrix (int cpunum, unsigned int maxi, struct cpudata *cpu)
{
	unsigned int i, ntlb;
	unsigned long maxei, eax, ebx, ecx, edx;
	int stepping, model, family, reserved;

	printf ("Cyrix-specific functions\n");
	cpuid (cpunum, 0x80000000, &maxei, NULL, NULL, NULL);
	if (maxei >= 0x80000000 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx,
				edx);
		}
	}

	/* Do standard stuff */
	if (maxi >= 1) {
		cpuid (cpunum, 1, &eax, &ebx, &ecx, &edx);
		stepping = eax & 0xf;
		model = (eax >> 4) & 0xf;
		family = (eax >> 8) & 0xf;
		reserved = eax >> 12;

		printf ("Family: %d Model: %d [", family, model);
		switch (family) {
		case 4:
			switch (model) {
			case 4:
				printf ("MediaGX");
				break;
			}
			break;
		case 5:
			switch (model) {
			case 2:
				printf ("6x86");
				break;
			case 4:
				printf ("GXm");
				break;
			}
			break;
		case 6:
			switch (model) {
			case 0:
				printf ("6x86/MX");
				break;
			}
			break;
		}
		printf ("]\n\n");
		decode_feature_flags (cpu, edx);

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

		/* Check for presence of extended info */
		if (maxei < 0x80000000)
			return;

		printf ("\nExtended feature flags:\n");
		if (maxei >= 0x80000001) {
			cpuid (cpunum, 0x80000001, &eax, &ebx, &ecx, &edx);
			stepping = eax & 0xf;
			model = (eax >> 4) & 0xf;
			family = (eax >> 8) & 0xf;
			reserved = eax >> 12;
			printf ("Family: %d Model: %d [", family, model);
			switch (family) {
			case 4:
				printf ("MediaGX");
				break;
			case 5:
				printf ("6x86/GXm");
				break;
			case 6:
				printf ("6x86/MX");
			}
			printf ("]\n");
		}
		printf ("\n");

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
			printf ("Processor name string: %s\n", namestring);
		}
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
}
