/*
 *  $Id: MSR-P4.c,v 1.2 2003/04/11 00:17:20 davej Exp $
 *  This file is part of x86info.
 *  (C) 2002 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel P4 specific MSR information
 *  See 24547203.pdf for more details.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"

void dump_p4_MSRs (struct cpudata *cpu)
{
	unsigned long long val=0;

	if (!user_is_root)
		return;

	printf ("Pentium 4 specific MSRs:\n");

	if (read_msr (cpu->number, 0x17, &val)==1)
		printf ("IA32_PLATFORM_ID=%016llx\n", val);

	if (read_msr (cpu->number, 0x2a, &val)==1) {
		printf ("System bus in order queue depth=");
		if ((val & (1<<2)))
			printf ("1");
		else
			printf ("12");
		printf ("\n");
	}

	if (read_msr (cpu->number, 0x2c, &val)==1) {
		printf ("MSR_EBC_FREQUENCY_ID=%016llx\n", val);
	}

	if (read_msr (cpu->number, 0x8b, &val)==1) {
		printf ("IA32_BIOS_SIGN_ID=%016llx\n", val);
	}

	if (read_msr (cpu->number, 0x119, &val)==1) {
		printf ("Processor serial number is ");
		if ((val & (1<<21)))
			printf ("dis");
		else
			printf ("en");
		printf ("abled\n");
	}

	if (read_msr (cpu->number, 0x1a0, &val)==1) {
		printf ("Fast string enable is ");
		if (!(val & (1<<0)))
			printf ("un");
		printf ("set\n");

		printf ("x87 FPU Fopcode compatability mode is ");
		if (!(val & (1<<2)))
			printf ("un");
		printf ("set\n");

		printf ("Thermal monitor enable is ");
		if (!(val & (1<<3)))
			printf ("un");
		printf ("set\n");

		printf ("Split lock disable is ");
		if (!(val & (1<<4)))
			printf ("un");
		printf ("set\n");

		printf ("L3 cache disable is ");
		if (!(val & (1<<6)))
			printf ("un");
		printf ("set\n");

		printf ("Performance monitoring is ");
		if (!(val & (1<<7)))
			printf ("un");
		printf ("available\n");

		printf ("Suppress lock enable is ");
		if (!(val & (1<<8)))
			printf ("un");
		printf ("set\n");

		printf ("Prefetch queue disable is ");
		if (!(val & (1<<9)))
			printf ("un");
		printf ("set\n");

		printf ("FERR# Interrupt reporting enable is ");
		if (!(val & (1<<10)))
			printf ("un");
		printf ("set\n");

		printf ("Branch trace storage unavailable is ");
		if (!(val & (1<<11)))
			printf ("un");
		printf ("set\n");

		printf ("Precise Event Based Sampling Unavailable is ");
		if (!(val & (1<<12)))
			printf ("un");
		printf ("set\n");

	}
/*
	if (read_msr (cpu->number, 0x410, &val)==1) {
	}
*/

	printf ("\n");
}
