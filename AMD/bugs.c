/*
 *  $Id: bugs.c,v 1.5 2004/06/02 22:29:29 davej Exp $
 *  This file is part of x86info.
 *  (C) 2002 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific errata information
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "AMD.h"

static void show_k7_bugs(struct cpudata *cpu)
{
	unsigned long long val;

	/* As per 27212 0.2 - Some newer athlons are more robust with
	   CLK_CTL reprogrammed to 0x20000000 instead of 0x60000000 */
	if (tuple(cpu) > 0x681) {
		if (read_msr (cpu->number, MSR_CLKCTL, &val) == 1) {
			if ((val & 0xfff00000) != 0x20000000) {
				printf ("CLK_CTL is programmed to %08llx, instead of %08llx\n",
						val, ((val&~0xfff00000)|0x20000000));
			}
		}
	}
}

void show_amd_bugs(struct cpudata *cpu)
{
	switch (cpu->family) {
	/* Athlons. */
	case 6:
		show_k7_bugs(cpu);
		break;

	default:
		break;
	}
}

