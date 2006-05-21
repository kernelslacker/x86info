/*
 *  $Id: MSR-PM.c,v 1.4 2003/06/12 05:56:10 davej Exp $
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
#include "Intel.h"

void dump_centrino_MSRs (struct cpudata *cpu)
{
	unsigned long long val=0;
	int tcc = 0;

	if (!user_is_root)
		return;

	printf ("Pentium M MSRs:\n");

	if (read_msr (cpu->number, MSR_IA32_PERF_STATUS, &val)==1) {
		/*
		   Voltage and frequency values derived from 1300MHz
		   Pentium M in an IBM ThinkPad X31.  Constants for
		   voltage function derived from voltage points documented
		   in Pentium M datasheet (Intel document 25261201.pdf,
		   table 5), and ACPI power management tables.

		   This register is specifically documented as
		   implementation-defined, so it may not apply to
		   other Enhanced SpeedStep enabled CPUs, or even other
		   Pentium Ms.
		 */
		unsigned uv = (unsigned)val & 0xffff;
		int volt = (uv & 0xff) * 16 + 700;
		int mhz = 100 * (uv & 0xff00) >> 8UL;
		printf ("  Current performance mode is 0x%04x: %dMHz, %d.%dV\n",
			uv, mhz, volt/1000, volt%1000);
	}
	if (read_msr (cpu->number, MSR_IA32_MISC_ENABLE, &val)==1) {
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
	if (tcc && read_msr (cpu->number, MSR_PM_THERM2_CTL, &val)==1) { /* THERM2_CTL */
		printf("  Thermal monitor %d\n", (val & (1<<16)) ? 2 : 1);
	}
	if (read_msr (cpu->number, MSR_IA32_THERM_CONTROL, &val)==1) {
		if (val & (1<<4)) {
			printf("  Software-controlled clock: %f%% duty cycle\n",
			       ((val >> 1) & 7) / 8.);
		} else
			printf("  Software-controlled clock disabled (full speed)\n");
	}
	if (read_msr (cpu->number, MSR_IA32_THERM_STATUS, &val)==1) { /* THERM_STATUS */
		printf("  Thermal status: ");
		if (val & (1<<0))
			printf("TooHot ");
		if (val & (1<<1))
			printf("WasTooHot ");
		printf("\n");
	}
	printf ("\n");
}
