/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Feature flag decoding.
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <x86info.h>

static void flag_decode(unsigned long reg, const char * reg_desc, const char *flags[], const char *flags_desc[])
{
	unsigned int i;

	for (i = 0; i < 32; i++) {
		if (reg & (1UL << i)) {
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


/* CPUID 0x00000006 EAX flags */
static const char *intel_cpuid_06_eax_flags[32] = {
	"dts", "ida", "arat", NULL, "pln", "ecmd", "ptm", "hwp",
	"hwp_notify", "hwp_act_window", "hwp_epp", "hwp_lpr",
	NULL, "hdc",
};

static const char *intel_cpuid_06_eax_flags_desc[32] = {
	"Digital temperature sensor supported",			// 0
	"Intel Dynamic Acceleration Technology (Turbo Boost)",	// 1
	"Always Running APIC Timer",				// 2
	NULL,							// 3
	"Power limit notification controls",			// 4
	"Clock modulation duty cycle extension",		// 5
	"Package thermal management",				// 6
	"HW-controlled performance states (HWP)",		// 7
	"HWP Notification",					// 8
	"HWP Activity Window",					// 9
	"HWP Energy Performance Preference",			// 10
	"HWP Package Level Request",				// 11
	NULL,							// 12
	"Hardware Duty Cycling",				// 13
};

/* CPUID 0x00000007 EBX flags */
static const char *intel_cpuid_07_ebx_flags[32] = {
	"fsgsbase", "tsc_adj", "sgx", "bmi1", "hle", "avx2", "fdp_excp", "smep",
	"bmi2", "erms", "invpcid", "rtm", "pqm", "nofpucs", "mpx", "pqe",
	"avx512f", "avx512dq", "rdseed", "adx", "smap", "avx512ifma",
	"pcommit", "clflushopt", "clwb", "ipt", "avx512pf", "avx512er",
	"avx512cd", "sha", "avx512bw", "avx512vl"
};

static const char *intel_cpuid_07_ebx_flags_desc[32] = {
	"Access to base of %fs and %gs",                                //0
	"TSC_ADJUST MSR",                                               //1
	"Software Guard Extensions",                                    //2
	"Bit Manipulation Instruction Set",                             //3
	"Transactional Synchronization Extensions",                     //4
	"Advanced Vector Extensions 2",                                 //5
	"FPU Data Pointer updated only on x87 exceptions",              //6
	"Supervisor-Mode Execution Prevention",                         //7
	"Bit Manipulation Instruction Set 2",                           //8
	"Enhanced REP MOVSB/STOSB",                                     //9
	"INVPCID instruction",                                          //10
	"Transactional Synchronization Extensions",                     //11
	"Platform Quality of Service Monitoring",                       //12
	"FPU CS and FPU DS deprecated",                                 //13
	"Intel MPX (Memory Protection Extensions)",                     //14
	"Platform Quality of Service Enforcement",                      //15
	"AVX-512 Foundation",                                           //16
	"AVX-512 Doubleword and Quadword Instruct",                     //17
	"RDSEED instruction",                                           //18
	"Intel ADX (Multi-Precision Add-Carry Instruction Extensions)", //19
	"Supervisor Mode Access Prevention",                            //20
	"AVX-512 Integer Fused Multiply-Add Instructions",              //21
	"PCOMMIT instruction",                                          //22
	"CLFLUSHOPT instruction",                                       //23
	"CLWB instruction",                                             //24
	"Intel Processor Trace",                                        //25
	"AVX-512 Prefetch Instructions",                                //26
	"AVX-512 Exponential and Reciprocal Instructions",              //27
	"AVX-512 Conflict Detection Instructions",                      //28
	"Intel SHA extensions",                                         //29
	"AVX-512 Byte and Word Instructions",                           //30
	"AVX-512 Vector Length Extensions"                              //31

};

	/* CPUID 0x80000007 EDX flags */
static const char *intel_cpuid_80000007_edx_flags[32] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	"nonstop_tsc",
};

static const char *intel_cpuid_80000007_edx_flags_desc[32] = {
	NULL,							// 0
	NULL,							// 1
	NULL,							// 2
	NULL,							// 3
	NULL,							// 4
	NULL,							// 5
	NULL,							// 6
	NULL,							// 7
	"Invariant/nonstop/constant TSC",			// 8
};

void show_extra_intel_flags(struct cpudata *cpu)
{
	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

	// Intel CPUID 0x06
	if (cpu->cpuid_level >= 0x06) {
		cpuid(cpu->number, 0x06, &eax, &ebx, &ecx, &edx);
		flag_decode(eax, "6:eax", intel_cpuid_06_eax_flags, intel_cpuid_06_eax_flags_desc);
	}
	// Intel CPUID 0x07
	if (cpu->cpuid_level >= 0x07) {
		ecx = 0;
		cpuid(cpu->number, 0x07, &eax, &ebx, &ecx, &edx);
		flag_decode(ebx, "7:ebx", intel_cpuid_07_ebx_flags, intel_cpuid_07_ebx_flags_desc);
	}
	// Intel CPUID 0x80000007
	if (cpu->maxei >= 0x80000007) {
		cpuid(cpu->number, 0x80000007, &eax, &ebx, &ecx, &edx);
		flag_decode(edx, "80000007:edx", intel_cpuid_80000007_edx_flags, intel_cpuid_80000007_edx_flags_desc);
	}
}

/* CPUID 0x00000001 EDX flags */
static const char *generic_cap_flags[32] = {
	"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
	"cx8", "apic", NULL, "sep", "mtrr", "pge", "mca", "cmov",
	"pat", "pse36", "psn", "clflsh", NULL, "ds", "acpi", "mmx",
	"fxsr", "sse", "sse2", "ss", "ht", "tm", NULL, "pbe"
};

static const char *generic_cap_flags_desc[32] = {
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
static const char *intel_cap_generic_ecx_flags[32] = {
	"sse3", "pclmuldq", "dtes64", "monitor", "ds-cpl", "vmx", "smx", "est",
	"tm2", "ssse3", "cid", "sdbg", "fma", "cx16", "xTPR", "pdcm",
	NULL, "pcid", "dca", "sse4_1", "sse4_2", "x2apic", "movbe", "popcnt",
	"tsc-deadline", "aes", "xsave", "osxsave", "avx", "f16c", "rdrnd", NULL
};
static const char *intel_cap_generic_ecx_flags_desc[32] = {
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
	"Silicon Debug",			    // 11
	"Fused Multiply Add",			    // 12
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
	"TSC Deadline support",			    // 24
	"AES Instruction",			    // 25
	"XSAVE/XSTOR States",			    // 26
	"OS-Enabled Extended State Management",	    // 27
	"AVX instruction extensions",		    // 28
	"F16C (half-precision) Floating Point",	    // 29
	"RDRAND (hardware RNG)",		    // 30
	// 31 unreliably indicates running in a hypervisor
	NULL					    // 31
};

/* CPUID 0x80000001 EDX flags */
static const char *intel_cap_extended_edx_flags[32] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, "SYSCALL", NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, "xd", NULL, NULL, NULL,
	NULL, NULL, "pdpe1gb", "rdtscp", NULL, "em64t", NULL, NULL,
};

static const char *intel_cap_extended_edx_flags_desc[32] = {
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
static const char *intel_cap_extended_ecx_flags[32] = {
	"lahf_lm", NULL, NULL, NULL, NULL, "lzcnt", NULL, NULL,
	"prefetchw", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};
static const char *intel_cap_extended_ecx_flags_desc[32] = {
	"LAHF/SAHF available in 64-bit mode",	    // 0
	NULL,					    // 1
	NULL,					    // 2
	NULL,					    // 3
	NULL,					    // 4
	"LZCNT instruction",			    // 5
	NULL,					    // 6
	NULL,					    // 7
	"PREFETCHW instruction",		    // 8
};

static const char *amd_cap_generic_ecx_flags[32] = {
	"sse3", "pclmulqdq", NULL, "mwait", NULL, NULL, NULL, NULL,
	NULL, "ssse3", NULL, NULL, "fma", "cmpxchg16b", NULL, NULL,
	NULL, NULL, NULL, "sse4_1", "sse4_2", NULL, NULL, "popcnt",
	NULL, "aes", "xsave", "osxsave", "avx", "f16c", NULL, NULL
};

static const char *amd_cap_generic_ecx_flags_desc[32] = {
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
static const char *amd_cap_extended_edx_flags[32] = {
	"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
	"cx8", "apic", NULL, "sep", "mtrr", "pge", "mca", "cmov",
	"pat", "pse36", NULL, "mp", "nx", NULL, "mmxext", "mmx",
	"fxsr", "ffxsr", "page1gb", "rdtscp",
	NULL, "lm", "3dnowext",	"3dnow"
}; /* "mp" defined for CPUs prior to AMD family 0xf */

static const char *amd_cap_extended_ecx_flags[32] = {
	"lahf/sahf", "CmpLegacy", "svm", "ExtApicSpace",
	"LockMovCr0", "abm", "sse4a", "misalignsse",
	"3dnowPref", "osvw", "ibs", "xop",
	"skinit", "wdt", NULL, "lwp",
	"fma4", "tce", NULL, "NodeId",
	NULL, "tbm", "TopoExt", "PerfCtrExtCore",
	"PerfCtrExtNB", NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static const char *centaur_cap_extended_ecx_flags[32] = {
	"sse3", NULL, NULL, NULL, NULL, NULL, NULL, "EPS",
	"tm2", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, "mmxext", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, "3dnowext", "3dnow"
};
static const char *centaur_cap_extended_edx_flags[32] = {
	NULL, NULL, "RNGp", "RNGe", NULL, NULL, "ACEp", "ACEe",
	"ACE2p", "ACE2e", "PHEp", "PHEe", "PMMp", "PMMe", NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static const char *transmeta_cap_flags[32] = {
	"recovery", "longrun", NULL, "lrti", NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


static void decode_feature_flags(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

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

		case VENDOR_NATSEMI:
		case VENDOR_RISE:
		case VENDOR_SIS:
		case VENDOR_UNKNOWN:
		default:
			/* Unknown CPU manufacturer or no special handling needed */
			break;
	}

	printf("\n");
}

static sigjmp_buf out;

static void sigill(__attribute__((__unused__))int sig)
{
	siglongjmp(out, 1);
}

static void test_longnop(void)
{
	int died;

	signal(SIGILL, sigill);

	died = sigsetjmp(out, 1);

	if (!died)
		asm volatile(".byte 0x0f,0x1f,0x00 /* nopl 0(%eax) */");

	printf("Long NOPs supported: %s\n", died ? "no" : "yes");
}

void display_feature_flags(struct cpudata *cpu)
{
	decode_feature_flags(cpu);
	test_longnop();
	printf("\n");
}
