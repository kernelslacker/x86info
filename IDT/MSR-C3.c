/*
 *  $Id: MSR-C3.c,v 1.4 2003/01/27 17:33:16 davej Exp $
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

void dump_C3_MSR (struct cpudata *cpu)
{
	if (!user_is_root)
		return;

	printf ("FCR: ");
	dumpmsr (cpu->number, 0x1107, 32);

	printf ("Power management: ");
	if (cpu->model==6 || (cpu->model==7 && cpu->stepping==0)) {
		printf ("Longhaul v1.0\n");
		decode_longhaul2(cpu);
	}

	if (cpu->model==7 && cpu->stepping>0) {
		printf ("Longhaul v2.0\n");
	}

	if (cpu->model==8 || cpu->model==9) {
		printf ("Powersaver v1.0\n");
		decode_powersaver(cpu);
	}
}
