/*
 *  $Id: features.c,v 1.25 2003/11/04 02:02:43 davej Exp $
 *  This file is part of x86info
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Feature flag decoding.
 */

#include <stdio.h>
#include "x86info.h"

void decode_feature_flags (struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;
	int i;

	const char *generic_cap_flags[] = {
		"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
		"cx8", "apic", NULL, "sep", "mtrr", "pge", "mca", "cmov",
		"pat", "pse36", "psn", "clflsh", NULL, "dtes", "acpi", "mmx",
		"fxsr", "sse", "sse2", "selfsnoop", "ht", "acc", "ia64", "pbe"
	};
	const char *generic_cap_flags_desc[] = {
		"Onboard FPU",
		"Virtual Mode Extensions",
		"Debugging Extensions",
		"Page Size Extensions",
		"Time Stamp Counter",
		"Model-Specific Registers",
		"Physical Address Extensions",
		"Machine Check Architecture",
		"CMPXCHG8 instruction",
		"Onboard APIC",
		NULL,
		"SYSENTER/SYSEXIT",
		"Memory Type Range Registers",
		"Page Global Enable",
		"Machine Check Architecture",
		"CMOV instruction",
		"Page Attribute Table",
		"36-bit PSEs",
		"Processor serial number",
		"CLFLUSH instruction",
		NULL,
		"Debug Trace Store",
		"ACPI via MSR",
		"MMX support",
		"FXSAVE and FXRESTORE instructions",
		"SSE support",
		"SSE2 support",
		"CPU self snoop",
		"Hyper-Threading",
		"Automatic clock Control",
		"IA-64 processor",
		"Pending Break Enable"
	};
	const char *intel_cap_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, "est",
		"tm2", NULL, "cntx-id", NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};
	const char *amd_cap_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "syscall", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "mp", NULL, NULL, "mmxext", NULL,
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

	cpuid(cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	cpu->flags = edx;
	if (cpu->vendor==VENDOR_INTEL) {
		cpu->eflags = ecx;
	} else {
		if (cpu->maxei >= 0x80000001) {
			cpuid(cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
			cpu->eflags = edx;
		}
	}

	if (silent != 0)
		return;

	printf ("Feature flags:\n");
	for (i = 0; i < 32; i++) {
		if (cpu->flags & (1 << i)) {
			if (verbose) {
				printf ("\t%s\n", generic_cap_flags_desc[i]);
			} else {
				printf (" %s", generic_cap_flags[i]);
			}
		}
	}
	printf ("\n");

	/* Vendor specific extensions. */
	switch (cpu->vendor) {
		case VENDOR_AMD:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (cpu->eflags & (1 << i) && amd_cap_flags[i])
					printf (" %s", amd_cap_flags[i]);
			}
			break;

		case VENDOR_CENTAUR:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (cpu->eflags & (1 << i) && centaur_cap_flags[i])
					printf (" %s", centaur_cap_flags[i]);
			}
			break;

		case VENDOR_TRANSMETA:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (cpu->eflags & (1 << i) && transmeta_cap_flags[i])
					printf (" %s", transmeta_cap_flags[i]);
			}
			break;

		case VENDOR_CYRIX:
			break;

		case VENDOR_INTEL:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (cpu->eflags & (1 << i) && intel_cap_flags[i])
					printf (" %s", intel_cap_flags[i]);
			}
			break;

		default:
			/* Unknown CPU manufacturer or no special handling needed */
			break;
	}

	printf ("\n");
}
