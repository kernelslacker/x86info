/*
 *  $Id: get_model_name.c,v 1.1 2001/12/10 21:12:12 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Get CPU name string from cpuid.
 *
 */

#include <stdio.h>
#include "x86info.h"

void get_model_name (unsigned long maxei, struct cpudata *cpu)
{
	unsigned int i, j;
	unsigned long eax, ebx, ecx, edx;
	char namestring[49], *cp;

	if (maxei < 0x80000004)
		return;

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
	printf ("Processor name string: %s\n\n", namestring);
}
