/*
 *  $Id: MSR-C3.c,v 1.5 2003/11/04 01:36:43 davej Exp $
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
	if (cpu->model==6 || cpu->model==7) {
		printf ("Longhaul\n");
		decode_longhaul(cpu);
	}

	if (cpu->model==8 || cpu->model==9) {
		printf ("Powersaver\n");
		decode_powersaver(cpu);
	}
}
