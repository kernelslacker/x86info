/*
 *  $Id: MSR-K6.c,v 1.7 2001/12/10 20:17:34 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-K6 specific MSR information
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "AMD.h"

void dump_k6_MSR (struct cpudata *cpu)
{
	unsigned long long val=0;

	if (!user_is_root)
		return;

	printf("\t\t\t\t31       23       15       7 \n");
	dumpmsr(cpu->number, 0xC0000082, 32);

	/* Original K6 or K6-2 (old core). */
	if ((cpu->model < 8) || ((cpu->model==8) && (cpu->stepping <8))) {
		if (read_msr (cpu->number, 0xC0000082, &val) == 1) {
			printf ("Write allocate enable limit: %dMbytes\n", (int) ((val & 0x7e) >>1) * 4);
			printf ("Write allocate 15-16M bytes: %s\n", val & 1 ? "enabled" : "disabled");
		} else {
			printf ("Couldn't read WHCR register.\n");
		}
	}

	/* K6-2 core (Stepping 8-F), K6-III or later. */
	if ((cpu->model > 8) || ((cpu->model==8) && (cpu->stepping>=8))) {
		if (read_msr (cpu->number, 0xC0000082, &val) == 1) {
			if (!(val & (0x3ff << 22)))
				printf ("Write allocate disabled\n");
			else {
				printf ("Write allocate enable limit: %dMbytes\n", (int) ((val >> 22) & 0x3ff) * 4);
				printf ("Write allocate 15-16M bytes: %s\n", val & (1<<16) ? "enabled" : "disabled");
			}
		} else {
			printf ("Couldn't read WHCR register.\n");
		}
	}

	printf ("\n");
}
