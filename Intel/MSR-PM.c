/*
 *  $Id: MSR-PM.c,v 1.1 2003/06/08 22:20:18 davej Exp $
 *  This file is part of x86info.
 *  (C) 2002 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel Pentium M specific MSR information
 *  See 24547203.pdf for more details.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"

void dump_centrino_MSRs (struct cpudata *cpu)
{
	unsigned long long val=0;
	int tcc = 0;

	if (!user_is_root)
		return;

	printf ("Pentium M MSRs:\n");

	if (read_msr (cpu->number, 0x198, &val)==1) {
		printf ("  Current performance mode is 0x%04x\n", (int)(val & 0xffff));
	}
	if (read_msr (cpu->number, 0x1a0, &val)==1) {
		printf ("  Enabled: ");
		if (val & (1<<3)) {
			printf("TCC ");
			tcc = 1;
		}
		if (val & (1<<7))
			printf("PerfMon ");
		if (val & (1<<10))
			printf("FERR# ");
		if (val & (1<<11))
			printf("noBTS ");
		if (val & (1<<12))
			printf("noPEBS ");
		if (val & (1<<16))
			printf("EnhancedSpeedStep ");

		printf("\n");
	}
	if (tcc && read_msr (cpu->number, 0x19d, &val)==1) { /* THERM2_CTL */
		printf("  Thermal monitor %d\n", (val & (1<<16)) ? 2 : 1);
	}
	if (read_msr (cpu->number, 0x19a, &val)==1) { /* THERM_CONTROL */
		if (val & (1<<4)) {
			printf("  Software-controlled clock: %f%% duty cycle\n",
			       ((val >> 1) & 7) / 8.);
		} else
			printf("  Software-controlled clock disabled (full speed)\n");
	}
	if (read_msr (cpu->number, 0x19c, &val)==1) { /* THERM_STATUS */
		printf("  Thermal status: ");
		if (val & (1<<0))
			printf("TooHot ");
		if (val & (1<<1))
			printf("WasTooHot ");
		printf("\n");
	}
	printf ("\n");
}
