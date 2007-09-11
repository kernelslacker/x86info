/*
 *  This file is part of x86info
 *  (C) 2001-2006 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Feature flag decoding.
 */

#include <stdio.h>
#include "x86info.h"

void flag_decode(unsigned long reg, const char *flags[])
{
	unsigned int i;

	for (i=0; i<32; i++) {
		if (reg & (1<<i)) {
			if (flags[i])
				printf(" %s", flags[i]);
			else
				printf(" [%u]", i);
		}
	}
}


void decode_feature_flags(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;
	unsigned int i;

	/* CPUID 0x00000001 EDX flags */
	const char *generic_cap_flags[] = {
		"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
		"cx8", "apic", NULL, "sep", "mtrr", "pge", "mca", "cmov",
		"pat", "pse36", "psn", "clflsh", NULL, "ds", "acpi", "mmx",
		"fxsr", "sse", "sse2", "ss", "ht", "tm", NULL, "pbe"
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
		"Processor serial number",	/* reserved on AMD */
		"CLFLUSH instruction",
		NULL,
		"Debug Trace Store",		/* reserved on AMD */
		"ACPI via MSR",			/* reserved on AMD */
		"MMX support",
		"FXSAVE and FXRESTORE instructions",
		"SSE support",
		"SSE2 support",
		"CPU self snoop",		/* reserved on AMD */
		"Hyper-Threading",
		"Thermal Monitor",		/* reserved on AMD */
		NULL,
		"Pending Break Enable"		/* reserved on AMD */
	};
	/* CPUID 0x00000001 ECX flags */
	const char *intel_cap_generic_ecx_flags[] = {
		"sse3", NULL, NULL, "monitor", "ds-cpl", "vmx", NULL, "est",
		"tm2", "ssse3", "cntx-id", NULL, NULL, "cx16", "xTPR", NULL,
		NULL, NULL, "dca", NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};
	/* CPUID 0x80000001 EDX flags */
	const char *intel_cap_extended_edx_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "SYSCALL", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, "xd", NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, "em64t", NULL, NULL,
	};
	/* CPUID 0x80000001 ECX flags */
	const char *intel_cap_extended_ecx_flags[] = {
		"lahf_lm", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	};

	const char *amd_cap_generic_ecx_flags[] = {
		"sse3", NULL, NULL, "mwait", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, "cmpxchg16b", NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, "popcnt",
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};
	const char *amd_cap_extended_edx_flags[] = {
		"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
		"cx8", "apic", NULL, "sep", "mtrr", "pge", "mca", "cmov",
		"pat", "pse36", NULL, "mp", "nx", NULL, "mmxext", "mmx",
		"fxsr", "ffxsr", "page1gb", "rdtscp",
		NULL, "lm", "3dnowext",	"3dnow"
	}; /* "mp" defined for CPUs prior to AMD family 0xf */

	const char *amd_cap_extended_ecx_flags[] = {
		"lahf/sahf", "CmpLegacy", "svm", "ExtApicSpace",
		"LockMovCr0", "abm", "sse4a", "misalignsse",
		"3dnowPref", "osvw", "ibs", NULL, "skinit", "wdt", NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};

	const char *centaur_cap_extended_ecx_flags[] = {
		"sse3", NULL, NULL, NULL, NULL, NULL, NULL, "EPS",
		"tm2", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, "mmxext", NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, "3dnowext", "3dnow"
	};
	const char *centaur_cap_extended_edx_flags[] = {
		NULL, NULL, "RNGp", "RNGe", NULL, NULL, "ACEp", "ACEe",
		"ACE2p", "ACE2e", "PHEp", "PHEe", "PMMp", "PMMe", NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};

	const char *transmeta_cap_flags[] = {
		"recovery", "longrun", NULL, "lrti", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};

	cpuid(cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	cpu->flags_ecx = ecx;
	cpu->flags_edx = edx;
	if (cpu->maxei >= 0x80000001) {
		cpuid(cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
		cpu->eflags_ecx = ecx;
		cpu->eflags_edx = edx;
	}

	if (silent != 0)
		return;

	printf ("Feature flags:\n");
	for (i=0; i<32; i++) {
		if (cpu->flags_edx & (1 << i)) {
			if (!(generic_cap_flags_desc[i]))
				printf(" [%u]", i);
			if (verbose)
				printf ("\t%s\n", generic_cap_flags_desc[i]);
			else
				printf (" %s", generic_cap_flags[i]);
		}
	}

	/* Vendor specific extensions. */
	switch (cpu->vendor) {
		case VENDOR_AMD:
			for (i=0; i<32; i++) {
				if (cpu->flags_ecx & (1 << i) && amd_cap_generic_ecx_flags[i])
					printf (" %s", amd_cap_generic_ecx_flags[i]);
			}
			printf("\n");
			if (cpu->maxei < 0x80000001)
				break;
			printf ("Extended feature flags:\n");
			flag_decode(cpu->eflags_edx, amd_cap_extended_edx_flags);
			flag_decode(cpu->eflags_ecx, amd_cap_extended_ecx_flags);
			printf("\n");
			break;

		case VENDOR_CENTAUR:
			printf ("\n");
			printf ("Extended feature flags:\n");
			flag_decode(cpu->flags_ecx, centaur_cap_extended_ecx_flags);
			cpuid(cpu->number, 0xc0000000, &eax, &ebx, &ecx, &edx);
			if (eax >=0xc0000001) {
				cpuid(cpu->number, 0xc0000001, &eax, &ebx, &ecx, &edx);
				cpu->flags_edx = edx;
				flag_decode(cpu->flags_edx, centaur_cap_extended_edx_flags);
			}
			break;

		case VENDOR_TRANSMETA:
			printf ("\n");
			printf ("Extended feature flags:\n");
			flag_decode(cpu->flags_ecx, transmeta_cap_flags);
			break;

		case VENDOR_CYRIX:
			printf ("\n");
			break;

		case VENDOR_INTEL:
			printf ("\n");
			printf ("Extended feature flags:\n");
			flag_decode(cpu->flags_ecx, intel_cap_generic_ecx_flags);
			if (cpu->maxei < 0x80000001)
				break;
			printf ("\n");
			flag_decode(cpu->eflags_edx, intel_cap_extended_edx_flags);
			flag_decode(cpu->eflags_ecx, intel_cap_extended_ecx_flags);
			break;

		default:
			/* Unknown CPU manufacturer or no special handling needed */
			break;
	}

	printf ("\n");
}
