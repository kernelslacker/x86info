/*
 *  $Id: MSR-Athlon.c,v 1.3 2001/12/10 20:17:34 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-Athlon specific MSR information
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "AMD.h"

void dump_athlon_MSR(struct cpudata *cpu)
{
	if (!user_is_root)
		return;

	printf("\t\t\t\t31       23       15       7 \n");
	dumpmsr(cpu->number, 0x2A, 32);
	dumpmsr(cpu->number, 0xC0000080, 32);
	dumpmsr(cpu->number, 0xC0010010, 32);
	dumpmsr(cpu->number, 0xC0010015, 32);
	dumpmsr(cpu->number, 0xC001001B, 32);
	printf ("\n");
}
