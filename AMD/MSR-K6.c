/*
 *  $Id: MSR-K6.c,v 1.10 2002/11/25 15:34:30 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-K6 specific MSR information
 *  See 21329h1.pdf for more details.
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

	/* Dump EWBE register on K6-2 & K6-3 */
	if ((cpu->family==5) && (cpu->model>=8)) {
		if (read_msr (cpu->number, 0xC0000080, &val) == 1) {
			if (val & (1<<0))
				printf ("System call extension present.\n");
			if (val & (1<<1))
				printf ("Data prefetch enabled.\n");
			else
				printf ("Data prefetch disabled.\n");
			printf ("EWBE mode: ");
			switch ((val & (1<<2|1<<3|1<<4))>>2) {
				case 0:	printf ("strong ordering (slowest performance)\n");
					break;
				case 1:	printf ("speculative disable (close to best performance)\n");
					break;
				case 2:	printf ("invalid\n");
					break;
				case 3:	printf ("global disable (best performance)\n");
					break;
			}
		} else {
			printf ("Couldn't read EFER register.\n");
		}
	}

	printf ("\n");
}
