/*
 *  $Id: dumpregs.c,v 1.3 2001/12/10 23:51:58 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include "x86info.h"

void dumpregs (int cpunum)
{
	unsigned int i;
	unsigned long maxi, eax, ebx, ecx, edx;

	/* Dump all the CPUID results in raw hex */
	cpuid (cpunum, 0, &maxi, NULL, NULL, NULL);
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */
	for (i = 0; i <= maxi; i++) {
		cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
		printf ("eax in: 0x%08x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
	}
}

void showvendor (int cpunum)
{
	unsigned int i;
	unsigned long maxi, eax, ebx, ecx, edx;

	cpuid (cpunum, 0, &maxi, NULL, NULL, NULL);
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */

	/* Vendor ID and max CPUID level supported */
	cpuid (cpunum, 0, &eax, &ebx, &ecx, &edx);
	printf ("Vendor ID: \"");
	for (i = 0; i < 4; i++)
		putchar (ebx >> (8 * i));
	for (i = 0; i < 4; i++)
		putchar (edx >> (8 * i));
	for (i = 0; i < 4; i++)
		putchar (ecx >> (8 * i));
	printf ("\"; Max CPUID level %ld\n\n", maxi);
}

