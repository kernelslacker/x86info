/*
 *  $Id: MSR-P4.c,v 1.1 2002/11/12 17:15:31 davej Exp $
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
		printf ("IA32_PLATFORM_ID=%llx\n", val);

	if (read_msr (cpu->number, 0x2a, &val)==1) {
		printf ("System bus in order queue depth=");
		if ((val & (1<<2))==1)
			printf ("1");
		else
			printf ("12");
		printf ("\n");
	}

	if (read_msr (cpu->number, 0x2c, &val)==1) {
		printf ("MSR_EBC_FREQUENCY_ID=%llx\n", val);
	}

	if (read_msr (cpu->number, 0x8b, &val)==1) {
		printf ("IA32_BIOS_SIGN_ID=%llx\n", val);
	}

	if (read_msr (cpu->number, 0x119, &val)==1) {
		printf ("Processor serial number is ");
		if ((val & (1<<21))==1)
			printf ("dis");
		else
			printf ("en");
		printf ("abled\n");
	}

	if (read_msr (cpu->number, 0x1a0, &val)==1) {
		printf ("Fast strings are ");
		if ((val & (1<<0))==0)
			printf ("dis");
		else
			printf ("en");
		printf ("abled\n");

		printf ("x87 FPU Fopcode compatability mode is ");
		if ((val & (1<<2))==0)
			printf ("dis");
		else
			printf ("en");
		printf ("abled\n");

		printf ("Thermal monitor is ");
		if ((val & (1<<3))==0)
			printf ("dis");
		else
			printf ("en");
		printf ("abled\n");

		printf ("Split lock is ");
		if ((val & (1<<4))==1)
			printf ("dis");
		else
			printf ("en");
		printf ("abled\n");

	}
/*
	if (read_msr (cpu->number, 0x410, &val)==1) {
	}
*/

	printf ("\n");
}
