/*
 *  $Id: features.c,v 1.9 2001/12/10 22:53:32 davej Exp $
 *  This file is part of x86info
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Feature flag decoding.
 */

#include <stdio.h>
#include "x86info.h"

void decode_feature_flags (struct cpudata *cpu, int flags, int eflags)
{
	const char *generic_cap_flags[] = {
		"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
		"cx8", "apic", "10", "sep", "mtrr", "pge", "mca", "cmov",
		"pat", "pse36", "psn", "clflsh", "20", "dtes", "acpi", "mmx",
		"fxsr", "xmm", "xmm2", "selfsnoop", "ht", "acc", "ia64", "31"
	};
	const char *amd_cap_flags[] = {
		"0", "1", "2", "3", "4", "5", "6", "7",
		"8", "9", "10", "syscall", "12", "13", "14", "15",
		"16", "17", "18", "19", "20", "21", "mmxext", "23",
		"24", "25", "26", "27", "28", "lm", "3dnow!ext", "3dnow!"
	};
	const char *centaur_cap_flags[] = {
		"0", "1", "2", "3", "4", "5", "6", "7",
		"8", "9", "10", "11", "12", "13", "14", "15",
		"16", "17", "18", "19", "20", "21", "22", "23",
		"24", "25", "26", "27", "28", "lm", "3dnow!ext", "3dnow!"
	};
	const char *transmeta_cap_flags[] = {
		"recovery", "longrun", "2", "lrti", "4", "5", "6", "7",
		"8", "9", "10", "11", "12", "13", "14", "15",
		"16", "17", "18", "19", "20", "21", "22", "23",
		"24", "25", "26", "27", "28", "29", "30", "31"
	};

	int i;

	if (show_flags == 0)
		return;

	printf ("Feature flags:\n");
	for (i = 0; i < 32; i++)
		if (flags & (1 << i))
			printf (" %s", generic_cap_flags[i]);

	printf ("\n");

	/* Vendor specific extensions. */
	switch (cpu->vendor) {

		case VENDOR_AMD:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i))
					printf (" %s", amd_cap_flags[i]);
			}
			break;

		case VENDOR_CENTAUR:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i))
					printf (" %s", centaur_cap_flags[i]);
			}
			break;

		case VENDOR_TRANSMETA:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i))
					printf (" %s", transmeta_cap_flags[i]);
			}
			break;

		case VENDOR_CYRIX:
		case VENDOR_INTEL:
		default:
			/* Unknown CPU manufacturer or no special handling needed */
			break;
	}

	printf ("\n\n");
}
