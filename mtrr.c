/*
 *  $Id: mtrr.c,v 1.1 2002/05/31 12:14:51 davej Exp $
 *  This file is part of x86info
 *  (C) 2002 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  MTRR register dumping.
 *  TODO : decode the registers too.
 */

#include <stdio.h>
#include "x86info.h"

void dump_mtrr (int cpu, int msr)
{
	unsigned long long val=0;

	if (read_msr(cpu, msr, &val) == 1) {
		printf ("  MSR: 0x%x=0x%016llx\n", msr, val);
	}
}

void dump_mtrrs (struct cpudata *cpu)
{
	int i;

	printf ("MTRR registers:\n");

	for (i=0x200; i<0x211; i++)
		dump_mtrr(cpu->number, i);
	dump_mtrr (cpu->number, 0x2ff);

	printf ("\n\n");
}
