/*
 *  $Id: identify.c,v 1.4 2001/04/27 22:11:29 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include "x86info.h"

void identify (int cpunum)
{
	struct cpudata cpu;
	unsigned long maxi, eax, ebx, ecx, edx;

	/* Dump all the CPUID results in raw hex */
	cpuid (cpunum, 0, &maxi, NULL, NULL, NULL);
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */

	cpuid (cpunum, 0, &eax, &ebx, &ecx, &edx);

	switch (ebx) {
	case 0x756e6547:	/* Intel */
		dointel (cpunum, maxi, &cpu);
		break;

	case 0x68747541:	/* AMD */
		doamd (cpunum, maxi, &cpu);
		break;

	case 0x69727943:	/* Cyrix */
		docyrix (cpunum, maxi, &cpu);
		break;

	case 0x746e6543:	/* IDT */
		doIDT (cpunum, maxi, &cpu);
		break;

	case 0x52697365:	/* This should be checked. Why 2 ? */
	case 0x65736952:	/* Rise */
		doRise (cpunum, maxi, &cpu);
		break;

	default:
		printf ("Unknown vendor\n");
		break;
	}
}
