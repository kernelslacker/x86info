/*
 * Feature flag decoding.
 * Modified from a routine in Linux kernel 2.4.0test3 by Jauder Ho <jauderho@carumba.com>
 */

#include <stdio.h>
#include "x86info.h"

extern struct cpudata *cpu;

void decode_feature_flags (struct cpudata *cpu, int flags)
{
	/* 
	 * Flags should be entered into the array ONLY if there is no overlap.
	 * Else a number should be used and then overridden in the case 
	 * statement below. --Jauder <jauderho@carumba.com>
	 *
	 * NOTE: bits 10, 19-22, 26-31 are reserved.
	 *
	 * Data courtesy of http://www.sandpile.org/arch/cpuid.htm
	 * Thanks to the Greasel!
	 */
	static char *cap_flags[] = {
		"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
		"cx8", "apic", "10", "sep", "mtrr", "pge", "mca", "cmov",
		"16", "pse36", "psn", "19", "20", "21", "22", "mmx",
		"24", "xmm", "26", "27", "28", "29", "30", "31"
	};
	int i;

	/* Modify the capabilities according to chip type */
	switch (cpu->vendor) {

	case VENDOR_CYRIX:
		cap_flags[24] = "cxmmx";
		break;

	case VENDOR_AMD:
		if (cpu->family == 5 && cpu->model == 6)
			cap_flags[10] = "sep";
		if (cpu->family < 6)
			cap_flags[16] = "fcmov";
		cap_flags[16] = "pat";
		cap_flags[22] = "mmxext";
		cap_flags[24] = "fxsr";
		cap_flags[30] = "3dnowext";
		cap_flags[31] = "3dnow";
		break;

	case VENDOR_INTEL:
		cap_flags[16] = "pat";
		cap_flags[18] = "pn";
		cap_flags[24] = "fxsr";
		cap_flags[25] = "xmm";
		break;

	case VENDOR_CENTAUR:
		if (cpu->model >= 8)	/* Only Winchip2 and above */
			cap_flags[31] = "3dnow";
		break;

	default:
		/* Unknown CPU manufacturer or no special handling needed */
		break;
	}

	for (i = 0; i < 32; i++)
		if (flags & (1 << i))
			printf (" %s", cap_flags[i]);

	printf ("\n\n");
}
