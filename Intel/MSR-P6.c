/*
 *  $Id: MSR-P6.c,v 1.1 2002/11/12 17:19:17 davej Exp $
 *  This file is part of x86info.
 *  (C) 2002 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel P6 specific MSR information
 *  See 24547203.pdf for more details.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"

void dump_p6_MSRs (struct cpudata *cpu)
{
	unsigned long long val=0;

	if (!user_is_root)
		return;

	printf ("P6 family MSRs:\n");

	if (read_msr (cpu->number, 0x2a, &val)==1) {
		printf ("Low power mode is ");
		if ((val & (1<<26))==0)
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
