/*
 *  $Id: features.c,v 1.14 2001/12/10 23:56:15 davej Exp $
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
		"cx8", "apic", NULL, "sep", "mtrr", "pge", "mca", "cmov",
		"pat", "pse36", "psn", "clflsh", NULL, "dtes", "acpi", "mmx",
		"fxsr", "xmm", "xmm2", "selfsnoop", "ht", "acc", "ia64", NULL
	};
	const char *amd_cap_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "syscall", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, "mmxext", NULL,
		NULL, NULL, NULL, NULL, NULL, "lm", "3dnowext", "3dnow"
	};
	const char *centaur_cap_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, "mmxext", NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, "3dnowext", "3dnow"
	};
	const char *transmeta_cap_flags[] = {
		"recovery", "longrun", NULL, "lrti", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};

	int i;

	if (show_flags == 0)
		return;

	printf ("Feature flags:\n");
	for (i = 0; i < 32; i++)
		if (flags & (1 << i))
			printf (" %s", generic_cap_flags[i]);

	printf ("\n");

	if (eflags == 0) {
		printf ("\n");
		return;
	}

	/* Vendor specific extensions. */
	switch (cpu->vendor) {

		case VENDOR_AMD:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i) && amd_cap_flags[i])
					printf (" %s", amd_cap_flags[i]);
			}
			break;

		case VENDOR_CENTAUR:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i) && centaur_cap_flags[i])
					printf (" %s", centaur_cap_flags[i]);
			}
			break;

		case VENDOR_TRANSMETA:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i) && transmeta_cap_flags[i])
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
