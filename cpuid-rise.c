/*
 *  $Id: cpuid-rise.c,v 1.1 2001/02/28 00:48:00 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include "x86info.h"

extern int show_cacheinfo;
extern int show_flags;
extern int show_registers;

void doRise (int cpunum, unsigned int maxi, struct cpudata *cpu)
{
	unsigned int i;
	unsigned long maxei, eax, ebx, ecx, edx;

	cpu->vendor = VENDOR_RISE;

	cpuid (cpunum, 0x80000000, &maxei, NULL, NULL, NULL);

	if (maxi != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x00000000; i <= maxi; i++) {
			cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
		}
		printf ("\n");
	}

	if (maxei != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
		}
		printf ("\n");
	}

	/* Do standard stuff */
	if (maxi >= 1) {
		cpuid (cpunum, 1, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;

		printf ("Family: %d Model: %d [iDragon ", cpu->family, cpu->model);
		switch (cpu->family) {
		case 5:
			switch (cpu->model) {
			case 0:
				printf ("(0.25um)");
				break;
			case 1:
				printf ("(0.18um)");
				break;
			case 8:
				printf ("II (0.25um)");
				break;
			case 9:
				printf ("II (0.18um)");
				break;
			default:
				printf ("<unknown type>");
				break;
			}
			break;

		default:
			printf ("<unknown type>");
			break;
		}
		printf ("]\n");
	}

	/* Check for presence of extended info */
	if (maxei == 0)
		return;

	if (maxei >= 0x80000001) {
		cpuid (cpunum, 0x80000001, &eax, &ebx, &ecx, &edx);
		decode_feature_flags (cpu, edx);
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
		printf ("Processor name string: %s\n", namestring);
	}
}
