/*
 *  (C) 2001-2006 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Feature flag decoding.
 */

#include <stdio.h>
#include "x86info.h"

static void flag_decode(unsigned long reg, const char * reg_desc, const char *flags[], const char *flags_desc[])
{
	unsigned int i;

	for (i=0; i<32; i++) {
		if (reg & (1<<i)) {
			if (!verbose) {
			    if (flags[i])
				    printf(" %s", flags[i]);
			    else
				    printf(" [%s:%u]", reg_desc, i);
			} else {
			    if (flags[i])
				    printf(" %-8s", flags[i]);
			    else
				    printf(" [%s:%u]     ", reg_desc, i);
			    if (flags_desc)
				    printf("\t%s\n", flags_desc[i]);
			    else
				    printf("\n");
			}
		}
	}
}


void get_feature_flags(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	cpuid(cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	cpu->flags_ecx = ecx;
	cpu->flags_edx = edx;
	if (cpu->maxei >= 0x80000001) {
		cpuid(cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
		cpu->eflags_ecx = ecx;
		cpu->eflags_edx = edx;
	}
}

void show_extra_intel_flags(struct cpudata *cpu)
{
	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
	/* CPUID 0x00000006 EAX flags */
	const char *intel_cpuid_06_eax_flags[] = {
		"dts", "ida", "arat", NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};
	const char *intel_cpuid_06_eax_flags_desc[] = {
		"Digital temperature sensor supported",			// 0
		"Intel Dynamic Acceleration Technology (Turbo Boost)",	// 1
		"Always Running APIC Timer",				// 2
		NULL,							// 3
		NULL,							// 4
		NULL,							// 5
		NULL,							// 6
		NULL,							// 7
		NULL,							// 8
		NULL,							// 9
		NULL,							// 10
		NULL,							// 11
		NULL,							// 12
		NULL,							// 13
		NULL,							// 14
		NULL,							// 15
		NULL,							// 16
		NULL,							// 17
		NULL,							// 18
		NULL,							// 19
		NULL,							// 20
		NULL,							// 21
		NULL,							// 22
		NULL,							// 23
		NULL,							// 24
		NULL,							// 25
		NULL,							// 26
		NULL,							// 27
		NULL,							// 28
		NULL,							// 29
		NULL,							// 30
		NULL							// 31
	};

	/* CPUID 0x80000007 EDX flags */
	const char *intel_cpuid_80000007_edx_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		"nonstop_tsc", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};
	const char *intel_cpuid_80000007_edx_flags_desc[] = {
		NULL,							// 0
		NULL,							// 1
		NULL,							// 2
		NULL,							// 3
		NULL,							// 4
		NULL,							// 5
		NULL,							// 6
		NULL,							// 7
		"Invariant/nonstop/constant TSC",			// 8
		NULL,							// 9
		NULL,							// 10
		NULL,							// 11
		NULL,							// 12
		NULL,							// 13
		NULL,							// 14
		NULL,							// 15
		NULL,							// 16
		NULL,							// 17
		NULL,							// 18
		NULL,							// 19
		NULL,							// 20
		NULL,							// 21
		NULL,							// 22
		NULL,							// 23
		NULL,							// 24
		NULL,							// 25
		NULL,							// 26
		NULL,							// 27
		NULL,							// 28
		NULL,							// 29
		NULL,							// 30
		NULL							// 31
	};

	// Intel CPUID 0x06
	if (cpu->cpuid_level >= 0x06) {
		cpuid(cpu->number, 0x06, &eax, &ebx, &ecx, &edx);
		flag_decode(eax, "6:eax", intel_cpuid_06_eax_flags, intel_cpuid_06_eax_flags_desc);
	}
	// Intel CPUID 0x80000007
	if (cpu->maxei >= 0x80000007) {
		cpuid(cpu->number, 0x80000007, &eax, &ebx, &ecx, &edx);
		flag_decode(edx, "80000007:edx", intel_cpuid_80000007_edx_flags, intel_cpuid_80000007_edx_flags_desc);
	}
}

void show_feature_flags(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	/* CPUID 0x00000001 EDX flags */
	const char *generic_cap_flags[] = {
		"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
		"cx8", "apic", NULL, "sep", "mtrr", "pge", "mca", "cmov",
		"pat", "pse36", "psn", "clflsh", NULL, "ds", "acpi", "mmx",
		"fxsr", "sse", "sse2", "ss", "ht", "tm", NULL, "pbe"
	};
	const char *generic_cap_flags_desc[] = {
		"Onboard FPU",						// 0
		"Virtual Mode Extensions",				// 1
		"Debugging Extensions",					// 2
		"Page Size Extensions",					// 3
		"Time Stamp Counter",					// 4
		"Model-Specific Registers",				// 5
		"Physical Address Extensions",				// 6
		"Machine Check Exception",				// 7
		"CMPXCHG8 instruction",					// 8
		"Onboard APIC",						// 9
		NULL,							// 10
		"SYSENTER/SYSEXIT instructions",			// 11
		"Memory Type Range Registers",				// 12
		"Page Global Enable",					// 13
		"Machine Check Architecture",				// 14
		"CMOV instruction",					// 15
		"Page Attribute Table",					// 16
		"36-bit PSEs",						// 17
		"Processor serial number",	/* reserved on AMD */	// 18
		"CLFLUSH instruction",					// 19
		NULL,							// 20
		"Debug Trace Store",		/* reserved on AMD */	// 21
		"ACPI via MSR",			/* reserved on AMD */	// 22
		"MMX support",						// 23
		"FXSAVE and FXRSTOR instructions",			// 24
		"SSE support",						// 25
		"SSE2 support",						// 26
		"CPU self snoop",		/* reserved on AMD */	// 27
		"Hyper-Threading",					// 28
		"Thermal Monitor",		/* reserved on AMD */	// 29
		NULL,							// 30
		"Pending Break Enable"		/* reserved on AMD */	// 31
	};
	/* CPUID 0x00000001 ECX flags */
	const char *intel_cap_generic_ecx_flags[] = {
		"sse3", "pclmuldq", "dtes64", "monitor", "ds-cpl", "vmx", "smx", "est",
		"tm2", "ssse3", "cid", NULL, NULL, "cx16", "xTPR", "pdcm",
		NULL, "pcid", "dca", "sse4_1", "sse4_2", "x2apic", "movbe", "popcnt",
		NULL, "aes", "xsave", "osxsave", "avx", NULL, NULL, NULL
	};
	const char *intel_cap_generic_ecx_flags_desc[] = {
		"Streaming SIMD Extensions 3",		    // 0
		"PCLMULDQ Instruction",			    // 1
		"64-Bit Debug Store",			    // 2
		"MONITOR/MWAIT",			    // 3
		"CPL Qualified Debug Store",		    // 4
		"Virtual Machine Extensions",		    // 5
		"Safer Mode Extensions",		    // 6
		"Enhanced Intel SpeedStep Technology",	    // 7
		"Thermal Monitor 2",			    // 8
		"Supplemental Streaming SIMD Extensions 3", // 9
		"L1 Context ID",			    // 10
		NULL,					    // 11
		NULL,					    // 12
		"CMPXCHG16B",				    // 13
		"xTPR Update Control",			    // 14
		"Perfmon and Debug Capability",		    // 15
		NULL,					    // 16
		"Process-context identifiers",		    // 17
		"Direct Cache Access",			    // 18
		"Streaming SIMD Extensions 4.1",	    // 19
		"Streaming SIMD Extensions 4.2",	    // 20
		"Extended xAPIC Support",		    // 21
		"MOVBE Instruction",			    // 22
		"POPCNT Instruction",			    // 23
		NULL,					    // 24
		"AES Instruction",			    // 25
		"XSAVE/XSTOR States",			    // 26
		"OS-Enabled Extended State Management",	    // 27
		"AVX instruction extensions",		    // 28
		NULL,					    // 29
		NULL,					    // 30
		NULL					    // 31
	};
	/* CPUID 0x80000001 EDX flags */
	const char *intel_cap_extended_edx_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "SYSCALL", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, "xd", NULL, NULL, NULL,
		NULL, NULL, "pdpe1gb", "rdtscp", NULL, "em64t", NULL, NULL,
	};
	const char *intel_cap_extended_edx_flags_desc[] = {
		NULL,					    // 0
		NULL,					    // 1
		NULL,					    // 2
		NULL,					    // 3
		NULL,					    // 4
		NULL,					    // 5
		NULL,					    // 6
		NULL,					    // 7
		NULL,					    // 8
		NULL,					    // 9
		NULL,					    // 10
		"SYSCALL/SYSRET instructions",		    // 11
		NULL,					    // 12
		NULL,					    // 13
		NULL,					    // 14
		NULL,					    // 15
		NULL,					    // 16
		NULL,					    // 17
		NULL,					    // 18
		NULL,					    // 19
		"Execution Disable Bit",		    // 20
		NULL,					    // 21
		NULL,					    // 22
		NULL,					    // 23
		NULL,					    // 24
		NULL,					    // 25
		"1-GByte pages",			    // 26
		"RDTSCP and IA32_TSC_AUX",		    // 27
		NULL,					    // 28
		"Intel 64 Instruction Set Architecture",    // 29
		NULL,					    // 30
		NULL					    // 31
	};
	/* CPUID 0x80000001 ECX flags */
	const char *intel_cap_extended_ecx_flags[] = {
		"lahf_lm", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	};
	const char *intel_cap_extended_ecx_flags_desc[] = {
		"LAHF/SAHF available in 64-bit mode",	    // 0
		NULL,					    // 1
		NULL,					    // 2
		NULL,					    // 3
		NULL,					    // 4
		NULL,					    // 5
		NULL,					    // 6
		NULL,					    // 7
		NULL,					    // 8
		NULL,					    // 9
		NULL,					    // 10
		NULL,					    // 11
		NULL,					    // 12
		NULL,					    // 13
		NULL,					    // 14
		NULL,					    // 15
		NULL,					    // 16
		NULL,					    // 17
		NULL,					    // 18
		NULL,					    // 19
		NULL,					    // 19
		NULL,					    // 20
		NULL,					    // 22
		NULL,					    // 23
		NULL,					    // 24
		NULL,					    // 25
		NULL,					    // 26
		NULL,					    // 27
		NULL,					    // 28
		NULL,					    // 29
		NULL,					    // 30
		NULL					    // 31
	};

	const char *amd_cap_generic_ecx_flags[] = {
		"sse3", NULL, NULL, "mwait", NULL, NULL, NULL, NULL,
		NULL, "ssse3", NULL, NULL, NULL, "cmpxchg16b", NULL, NULL,
		NULL, NULL, NULL, "sse4_1", NULL, NULL, NULL, "popcnt",
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};
	const char *amd_cap_generic_ecx_flags_desc[] = {
		"Streaming SIMD Extensions 3",		    // 0
		NULL,					    // 1
		NULL,					    // 2
		"MONITOR/MWAIT instructions",		    // 3
		NULL,					    // 4
		NULL,					    // 5
		NULL,					    // 6
		NULL,					    // 7
		NULL,					    // 8
		"Supplemental Streaming SIMD Extensions 3", // 9
		NULL,					    // 10
		NULL,					    // 11
		NULL,					    // 12
		"CMPXCHG16B instruction",		    // 13
		NULL,					    // 14
		NULL,					    // 15
		NULL,					    // 16
		NULL,					    // 17
		NULL,					    // 18
		"Streaming SIMD Extensions 4.1",	    // 19
		NULL,					    // 20
		NULL,					    // 22
		"POPCNT instruction",			    // 23
		NULL,					    // 24
		NULL,					    // 25
		NULL,					    // 26
		NULL,					    // 27
		NULL,					    // 28
		NULL,					    // 29
		NULL,					    // 30
		NULL					    // 31
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

	printf("Feature flags:\n");
	flag_decode(cpu->flags_edx, "1:edx", generic_cap_flags, generic_cap_flags_desc);

	/* Vendor specific extensions. */
	switch (cpu->vendor) {
		case VENDOR_AMD:
			flag_decode(cpu->flags_ecx, "1:ecx", amd_cap_generic_ecx_flags, amd_cap_generic_ecx_flags_desc);
			printf("\n");
			if (cpu->maxei < 0x80000001)
				break;
			printf("Extended feature flags:\n");
			flag_decode(cpu->eflags_edx, "80000001:edx", amd_cap_extended_edx_flags, NULL);
			flag_decode(cpu->eflags_ecx, "80000001:ecx", amd_cap_extended_ecx_flags, NULL);
			printf("\n");
			break;

		case VENDOR_CENTAUR:
			printf("\n");
			printf("Extended feature flags:\n");
			flag_decode(cpu->flags_ecx, "1:ecx", centaur_cap_extended_ecx_flags, NULL);
			cpuid(cpu->number, 0xc0000000, &eax, &ebx, &ecx, &edx);
			if (eax >=0xc0000001) {
				cpuid(cpu->number, 0xc0000001, &eax, &ebx, &ecx, &edx);
				cpu->flags_edx = edx;
				flag_decode(cpu->flags_edx, "1:edx", centaur_cap_extended_edx_flags, NULL);
			}
			break;

		case VENDOR_TRANSMETA:
			printf("\n");
			printf("Extended feature flags:\n");
			flag_decode(cpu->flags_ecx, "1:ecx", transmeta_cap_flags, NULL);
			break;

		case VENDOR_CYRIX:
			printf("\n");
			break;

		case VENDOR_INTEL:
			flag_decode(cpu->flags_ecx, "1:ecx", intel_cap_generic_ecx_flags, intel_cap_generic_ecx_flags_desc);
			printf("\n");
			if (cpu->maxei < 0x80000001)
				break;
			printf("Extended feature flags:\n");
			flag_decode(cpu->eflags_edx, "80000001:edx", intel_cap_extended_edx_flags, intel_cap_extended_edx_flags_desc);
			flag_decode(cpu->eflags_ecx, "80000001:ecx", intel_cap_extended_ecx_flags, intel_cap_extended_ecx_flags_desc);
			show_extra_intel_flags(cpu);
			break;

		default:
			/* Unknown CPU manufacturer or no special handling needed */
			break;
	}

	printf("\n");
}
