/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Feature flag decoding.
 */

#include <stdio.h>
#include <x86info.h>

static void flag_decode(unsigned long reg, const char * reg_desc, const char *flags[], const char *flags_desc[])
{
	unsigned int i;

	for (i = 0; i < 32; i++) {
		if (reg & (1UL << i)) {
			if (!opts.verbose) {
			    if (flags[i])
				    printf(" %s", flags[i]);
			    else
				    printf(" [%s:%u]", reg_desc, i);
			} else {
			    if (flags[i])
				    printf(" %-8s", flags[i]);
			    else
				    printf(" [%s:%u]     ", reg_desc, i);
			    if (flags_desc && flags_desc[i])
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
	if (cpu->maxei >= 0x80000007) {
		cpuid(cpu->number, 0x80000007, NULL, NULL, NULL, &edx);
		cpu->apm_edx = edx;
	}
}


/* CPUID 0x00000006 EAX flags (Intel programming reference 319433-056). */
static const char *cpuid_06_eax_flags[32] = {
	"dts", "ida", "arat", NULL, "pln", "ecmd", "ptm", "hwp",
	"hwp_notify", "hwp_act_window", "hwp_epp", "hwp_lpr",
	NULL, "hdc", NULL, "hwp_highest_perf_change",
	"hwp_peci_override", "hwp_flexible", "hwp_fast_access", "hfi",
	"hwp_ignore_idle", NULL, "hwp_ctl", NULL,
	"dpti",
};

static const char *cpuid_06_eax_flags_desc[32] = {
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
	NULL,							// 14
	"HWP highest performance change",			// 15
	"HWP PECI override",					// 16
	"Flexible HWP",						// 17
	"Fast access, low latency and posted HWP requests",	// 18
	"Hardware Feedback Interface",				// 19
	"Ignore HWP requests from idle logical processors",	// 20
	NULL,							// 21
	"IA32_HWP_CTL MSR",					// 22
	NULL,							// 23
	"Directed Package Thermal Interrupt",			// 24
};

/* CPUID 0x00000007 ECX flags (subleaf 0) */
static const char *cpuid_07_ecx_flags[32] = {
	"prefetchwt1", "avx512vbmi", "umip", "pku", "ospke", "waitpkg", "avx512vbmi2", "cet_ss",
	"gfni", "vaes", "vpclmulqdq", "avx512vnni", "avx512bitalg", "tme", "avx512vpopcntdq", NULL,
	"la57", NULL, NULL, NULL, NULL, NULL, "rdpid", "kl",
	"bus_lock_detect", "cldemote", NULL, "movdiri", "movdir64b", "enqcmd", "sgx_lc", "pks"
};

static const char *cpuid_07_ecx_flags_desc[32] = {
	"PREFETCHWT1 instruction",			// 0
	"AVX-512 Vector Bit Manipulation Instructions",	// 1
	"User-Mode Instruction Prevention (UMIP)",	// 2
	"Protection Keys for User-mode pages",		// 3
	"OS has set CR4.PKE",				// 4
	"TPAUSE/UMONITOR/UMWAIT instructions",		// 5
	"AVX-512 Vector Bit Manipulation Instructions 2",// 6
	"CET Shadow Stack",				// 7
	"Galois Field New Instructions",		// 8
	"Vector AES (VAES)",				// 9
	"VPCLMULQDQ instruction",			// 10
	"AVX-512 Vector Neural Network Instructions",	// 11
	"AVX-512 Bit Algorithm Instructions",		// 12
	"Total Memory Encryption",			// 13
	"AVX-512 Vector Population Count D/Q",		// 14
	NULL,						// 15
	"5-level paging (57-bit VA)",			// 16
	NULL, NULL, NULL, NULL, NULL,			// 17-21 (MAWAU field)
	"Read Processor ID (RDPID)",			// 22
	"Key Locker",					// 23
	"Bus Lock Detection",				// 24
	"Cache Line Demote (CLDEMOTE)",			// 25
	NULL,						// 26
	"MOVDIRI instruction",				// 27
	"MOVDIR64B instruction",			// 28
	"ENQCMD instruction",				// 29
	"SGX Launch Configuration",			// 30
	"Protection Keys for Supervisor-mode",		// 31
};

/* CPUID 0x00000007 EDX flags (subleaf 0) */
static const char *cpuid_07_edx_flags[32] = {
	NULL, NULL, "avx512_4vnniw", "avx512_4fmaps", "fsrm", "uintr", NULL, NULL,
	"avx512vp2intersect", "srbds_ctrl", "md_clear", "rtm_always_abort", NULL, "rtm_force_abort", "serialize", "hybrid",
	"tsxldtrk", NULL, "pconfig", "arch_lbr", "ibt", NULL, "amx_bf16", "avx512_fp16",
	"amx_tile", "amx_int8", "ibrs_ibpb", "stibp", "l1d_flush", "arch_capabilities", "core_capabilities", "ssbd"
};

static const char *cpuid_07_edx_flags_desc[32] = {
	NULL,							// 0
	NULL,							// 1
	"AVX-512 4-register Neural Network Instructions",	// 2
	"AVX-512 4-register Multiply Accumulation Single",	// 3
	"Fast Short REP MOVSB",					// 4
	"User Interrupts",					// 5
	NULL,							// 6
	NULL,							// 7
	"AVX-512 VP2INTERSECT Instructions",			// 8
	"SRBDS mitigation control MSR",				// 9
	"VERW clears CPU buffers (MD_CLEAR)",			// 10
	"RTM always aborts",					// 11
	NULL,							// 12
	"RTM force abort MSR",					// 13
	"SERIALIZE instruction",				// 14
	"Hybrid CPU (big/little cores)",			// 15
	"TSX Suspend Load Address Tracking",			// 16
	NULL,							// 17
	"Platform Configuration (PCONFIG)",			// 18
	"Architectural Last Branch Records",			// 19
	"Indirect Branch Tracking (CET IBT)",			// 20
	NULL,							// 21
	"AMX BFloat16",						// 22
	"AVX-512 FP16",						// 23
	"AMX Tile Architecture",				// 24
	"AMX Int8",						// 25
	"IBRS / IBPB indirect branch controls",			// 26
	"Single Thread Indirect Branch Predictors (STIBP)",	// 27
	"L1D cache flush",					// 28
	"IA32_ARCH_CAPABILITIES MSR",				// 29
	"IA32_CORE_CAPABILITIES MSR",				// 30
	"Speculative Store Bypass Disable (SSBD)",		// 31
};

/* CPUID 0x00000007 EAX flags (subleaf 1) */
static const char *cpuid_07_1_eax_flags[32] = {
	"sha512", "sm3", "sm4", NULL, "avx_vnni", "avx512_bf16", "lass", "cmpccxadd",
	"arch_perfmon_ext", NULL, "fzrm", "fsrs", "fsrc", NULL, NULL, NULL,
	NULL, "fred", "lkgs", "wrmsrns", "nmi_src", "amx_fp16", "hreset", "avx_ifma",
	NULL, NULL, "lam", "msrlist", NULL, NULL, NULL, "movrs"
};

static const char *cpuid_07_1_eax_flags_desc[32] = {
	"SHA512 instructions",				// 0
	"SM3 instructions",				// 1
	"SM4 instructions",				// 2
	NULL,						// 3
	"AVX Vector Neural Network Instructions",	// 4
	"AVX-512 BF16 Instructions",			// 5
	"Linear Address Space Separation",		// 6
	"CMPccXADD instructions",			// 7
	"Architectural PerfMon extension (leaf 0x23)",	// 8
	NULL,						// 9
	"Fast Zero-length REP MOVSB",			// 10
	"Fast Short REP STOSB",				// 11
	"Fast Short REP CMPSB/SCASB",			// 12
	NULL,						// 13
	NULL,						// 14
	NULL,						// 15
	NULL,						// 16
	"Flexible Return and Event Delivery (FRED)",	// 17
	"LKGS instruction",				// 18
	"WRMSRNS instruction",				// 19
	"NMI-source reporting",				// 20
	"AMX FP16 Instructions",			// 21
	"HRESET instruction",				// 22
	"AVX Integer Fused Multiply-Add",		// 23
	NULL,						// 24
	NULL,						// 25
	"Linear Address Masking (LAM)",			// 26
	"RDMSRLIST/WRMSRLIST instructions",		// 27
	NULL,						// 28
	NULL,						// 29
	NULL,						// 30
	"MOVRS instructions",				// 31
};

/* CPUID 0x00000007 EDX flags (subleaf 1) */
static const char *cpuid_07_1_edx_flags[32] = {
	NULL, NULL, NULL, NULL, "avx_vnni_int8", "avx_ne_convert", NULL, NULL,
	"amx_complex", NULL, "avx_vnni_int16", NULL, NULL, NULL, "prefetchit0_1", "user_msr",
	NULL, NULL, "cet_sss", "avx10", NULL, "apx_f", NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static const char *cpuid_07_1_edx_flags_desc[32] = {
	[4] = "AVX-VNNI-INT8 instructions",
	[5] = "AVX-NE-CONVERT instructions",
	[8] = "AMX-COMPLEX instructions",
	[10] = "AVX-VNNI-INT16 instructions",
	[14] = "PREFETCHIT0/1 instructions",
	[15] = "User-mode MSR access (URDMSR/UWRMSR)",
	[18] = "CET supervisor shadow stacks safe to use",
	[19] = "AVX10",
	[21] = "Advanced Performance Extensions (APX_F)",
};

/* CPUID 0x00000007 EBX flags */
static const char *cpuid_07_ebx_flags[32] = {
	"fsgsbase", "tsc_adj", "sgx", "bmi1", "hle", "avx2", "fdp_excp", "smep",
	"bmi2", "erms", "invpcid", "rtm", "pqm", "nofpucs", "mpx", "pqe",
	"avx512f", "avx512dq", "rdseed", "adx", "smap", "avx512ifma",
	"pcommit", "clflushopt", "clwb", "ipt", "avx512pf", "avx512er",
	"avx512cd", "sha", "avx512bw", "avx512vl"
};

static const char *cpuid_07_ebx_flags_desc[32] = {
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
static const char *cpuid_80000007_edx_flags[32] = {
	"ts", "fid", "vid", "ttp", "tm", "stc", "100mhzsteps", "hwpstate",
	"nonstop_tsc", "cpb", "eff_freq_ro", "proc_feedback",
	"acc_power", "connected_standby", "rapl",
};

const char *cpuid_80000007_edx_flags_desc[32] = {
	"Temperature sensing diode present",			// 0
	"Frequency ID control",					// 1
	"Voltage ID control",					// 2
	"Thermal Trip",						// 3
	"Thermal Monitoring",					// 4
	"Software Thermal Control",				// 5
	"100MHz multiplier control",				// 6
	"Hardware P-state control",				// 7
	"Invariant/nonstop/constant TSC",			// 8
	"Core Performance Boost",				// 9
	"read-only Effective Frequency Interface",		// 10
	"Processor Feedback Interface",				// 11
	"Processor Power Reporting",				// 12
	"Connected Standby",					// 13
	"RAPL",							// 14
};

/* CPUID 0x0000000d subleaf 1 EAX flags */
static const char *cpuid_0d_1_eax_flags[32] = {
	"xsaveopt", "xsavec", "xgetbv1", "xsaves", "xfd",
};

/* CPUID 0x00000007 EDX flags (subleaf 2) */
static const char *cpuid_07_2_edx_flags[32] = {
	"intel_psfd", "ipred_ctrl", "rrsba_ctrl", "ddp_ctrl",
	"bhi_ctrl", "mcdt_no", "uclock_disable", "monitor_mitg_no",
};

/* CPUID 0x80000007 EBX flags */
static const char *cpuid_80000007_ebx_flags[32] = {
	"overflow_recov", "succor", NULL, "smca",
};

/* CPUID 0x80000008 EBX flags */
static const char *cpuid_80000008_ebx_flags[32] = {
	"clzero", "irperf", "xsaveerptr", "invlpgb",
	"rdpru", NULL, "mba", NULL,
	"mcommit", "wbnoinvd", NULL, NULL,
	"ibpb", "wbinvd_int", "ibrs", "stibp",
	"ibrs_always_on", "stibp_always_on", "ibrs_fast", "ibrs_same_mode",
	"no_efer_lmsle", "tlb_flush_nested", NULL, "amd_ppin",
	"amd_ssbd", "virt_ssbd", "amd_ssb_no", "cppc",
	"amd_psfd", "btc_no", "ibpb_ret", "branch_sampling",
};

/* CPUID 0x8000001f EAX flags */
static const char *cpuid_8000001f_eax_flags[32] = {
	"sme", "sev", "vm_page_flush", "sev_es",
	"sev_snp", "vmpl", "rmpquery", "vmpl_sss",
	"secure_tsc", "v_tsc_aux", "sme_coherent", "req_64bit_hypervisor",
	"restricted_injection", "alternate_injection", "debug_swap", "disallow_host_ibs",
	"virt_transparent_enc", "vmgexit_parameter", "virt_tom_msr", "virt_ibs",
	NULL, "rmpread", NULL, "segmented_rmp",
	"vmsa_reg_protection", "smt_protection", NULL, "allowed_sev_features",
	"svsm", "nested_virt_snp_msr", "hv_inuse_wr_allowed", NULL,
};

/* CPUID 0x80000020 EBX flags */
static const char *cpuid_80000020_ebx_flags[32] = {
	NULL, "mba", "smba", "bmec", "l3rr", "abmc", "sdciae",
};

/* CPUID 0x80000021 EAX flags (AMD APM volume 3, 24594 rev. 3.37). */
static const char *cpuid_80000021_eax_flags[32] = {
	"no_nested_data_bp", "fsgs_non_serializing", "lfence_serializing", "smm_page_cfg_lock",
	"tlbsync_sfw_no", "verw_clear", "null_sel_clr_base", "upper_addr_ignore",
	"auto_ibrs", "no_smm_ctl_msr", "fsrs", "fsrc",
	"pmc2_precise_retire", "prefetch_ctl_msr", "l2_tlb_size_x32", "amd_ermsb",
	"opcode_reclaim", "user_cpuid_disable", "epsf", "fast_rep_scasb",
	"prefetchi", "fp512_downgrade", "wl_feedback", "avx512_bmm",
	"eraps", NULL, NULL, "sbpb",
	"ibpb_brtype", "srso_no", "srso_user_kernel_no", "srso_bp_spec_reduce",
};

/* CPUID 0x80000021 ECX flags */
static const char *cpuid_80000021_ecx_flags[32] = {
	NULL, "tsa_sq_no", "tsa_l1_no", NULL, NULL, NULL, NULL, NULL,
	"btb_ctx_isolation",
};

/* CPUID 0x80000022 EAX flags */
static const char *cpuid_80000022_eax_flags[32] = {
	"perfmon_v2", "lbr_v2", "lbr_pmc_freeze",
};

static void show_extra_flags(const struct cpudata *cpu)
{
	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

	if (cpu->cpuid_level < 0x06 && cpu->maxei < 0x80000007)
		return;
	printf("\nAdditional feature flags:\n");

	// CPUID 0x06
	if (cpu->cpuid_level >= 0x06) {
		cpuid(cpu->number, 0x06, &eax, &ebx, &ecx, &edx);
		flag_decode(eax, "6:eax", cpuid_06_eax_flags, cpuid_06_eax_flags_desc);
	}
	// CPUID 0x07 subleaf 0
	if (cpu->cpuid_level >= 0x07) {
		unsigned int max_subleaf;
		cpuid_count(cpu->number, 0x07, 0, &eax, &ebx, &ecx, &edx);
		max_subleaf = eax;
		flag_decode(ebx, "7:ebx", cpuid_07_ebx_flags, cpuid_07_ebx_flags_desc);
		flag_decode(ecx, "7:ecx", cpuid_07_ecx_flags, cpuid_07_ecx_flags_desc);
		flag_decode(edx, "7:edx", cpuid_07_edx_flags, cpuid_07_edx_flags_desc);
		// CPUID 0x07 subleaf 1
		if (max_subleaf >= 1) {
			cpuid_count(cpu->number, 0x07, 1, &eax, &ebx, &ecx, &edx);
			flag_decode(eax, "7.1:eax", cpuid_07_1_eax_flags, cpuid_07_1_eax_flags_desc);
			flag_decode(edx, "7.1:edx", cpuid_07_1_edx_flags, cpuid_07_1_edx_flags_desc);
		}
		// CPUID 0x07 subleaf 2
		if (max_subleaf >= 2) {
			cpuid_count(cpu->number, 0x07, 2, &eax, &ebx, &ecx, &edx);
			flag_decode(edx, "7.2:edx", cpuid_07_2_edx_flags, NULL);
		}
	}
	// CPUID 0x0d subleaf 1
	if (cpu->cpuid_level >= 0x0d) {
		cpuid_count(cpu->number, 0x0d, 1, &eax, &ebx, &ecx, &edx);
		flag_decode(eax, "d.1:eax", cpuid_0d_1_eax_flags, NULL);
	}
	// CPUID 0x80000007
	if (cpu->maxei >= 0x80000007)
		flag_decode(cpu->apm_edx, "80000007:edx", cpuid_80000007_edx_flags, cpuid_80000007_edx_flags_desc);

	if (cpu->vendor != VENDOR_AMD)
		return;

	if (cpu->maxei >= 0x80000007) {
		cpuid(cpu->number, 0x80000007, NULL, &ebx, NULL, NULL);
		flag_decode(ebx, "80000007:ebx", cpuid_80000007_ebx_flags, NULL);
	}
	if (cpu->maxei >= 0x80000008) {
		cpuid(cpu->number, 0x80000008, NULL, &ebx, NULL, NULL);
		flag_decode(ebx, "80000008:ebx", cpuid_80000008_ebx_flags, NULL);
	}
	if (cpu->maxei >= 0x8000001f) {
		cpuid(cpu->number, 0x8000001f, &eax, NULL, NULL, NULL);
		flag_decode(eax, "8000001f:eax", cpuid_8000001f_eax_flags, NULL);
	}
	if (cpu->maxei >= 0x80000020) {
		cpuid_count(cpu->number, 0x80000020, 0, NULL, &ebx, NULL, NULL);
		flag_decode(ebx, "80000020:ebx", cpuid_80000020_ebx_flags, NULL);
	}
	if (cpu->maxei >= 0x80000021) {
		cpuid(cpu->number, 0x80000021, &eax, NULL, &ecx, NULL);
		flag_decode(eax, "80000021:eax", cpuid_80000021_eax_flags, NULL);
		flag_decode(ecx, "80000021:ecx", cpuid_80000021_ecx_flags, NULL);
	}
	if (cpu->maxei >= 0x80000022) {
		cpuid_count(cpu->number, 0x80000022, 0, &eax, NULL, NULL, NULL);
		flag_decode(eax, "80000022:eax", cpuid_80000022_eax_flags, NULL);
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
static const char *cap_generic_ecx_flags[32] = {
	"sse3", "pclmulqdq", "dtes64", "monitor", "ds-cpl", "vmx", "smx", "est",
	"tm2", "ssse3", "cid", "sdbg", "fma", "cx16", "xTPR", "pdcm",
	NULL, "pcid", "dca", "sse4_1", "sse4_2", "x2apic", "movbe", "popcnt",
	"tsc-deadline", "aes", "xsave", "osxsave", "avx", "f16c", "rdrnd", NULL
};
static const char *cap_generic_ecx_flags_desc[32] = {
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
static const char *cap_extended_edx_flags[32] = {
	"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
	"cx8", "apic", NULL, "syscall", "mtrr", "pge", "mca", "cmov",
	"pat", "pse36", NULL, "mp", "nx", NULL, "mmxext", "mmx",
	"fxsr", "ffxsr", "page1gb", "rdtscp",
	NULL, "lm", "3dnowext",	"3dnow"
}; /* "mp" defined for CPUs prior to AMD family 0xf */

static const char *cap_extended_edx_flags_desc[32] = {
	[11] = "SYSCALL/SYSRET instructions",
	[20] = "Execution Disable Bit",
	[26] = "1-GByte pages",
	[27] = "RDTSCP and IA32_TSC_AUX",
	[29] = "64-bit Long Mode",
};

/* CPUID 0x80000001 ECX flags */
static const char *cap_extended_ecx_flags[32] = {
	"lahf_lm", "CmpLegacy", "svm", "ExtApicSpace",
	"LockMovCr0", "abm", "sse4a", "misalignsse",
	"3dnowprefetch", "osvw", "ibs", "xop",
	"skinit", "wdt", NULL, "lwp",
	"fma4", "tce", NULL, "NodeId",
	NULL, "tbm", "TopoExt", "PerfCtrExtCore",
	"PerfCtrExtNB", NULL, "DataBPExt", "PTSC", "PerfCtrExtLLC", "MwaitExtended", "AdMskExtn", NULL
};

static const char *cap_extended_ecx_flags_desc[32] = {
	[0] = "LAHF/SAHF available in 64-bit mode",
	[5] = "LZCNT/ABM instructions",
	[8] = "PREFETCHW instruction",
	[27] = "Performance time-stamp counter",
};

static const char *centaur_cap_ecx_flags[32] = {
	"sse3", NULL, NULL, NULL, NULL, NULL, NULL, "EPS",
	"tm2", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static const char *centaur_cap_80000001_edx_flags[32] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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


static void decode_feature_flags(const struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	printf("Feature flags:\n");
	flag_decode(cpu->flags_edx, "1:edx", generic_cap_flags, generic_cap_flags_desc);

	/* Vendor specific extensions. */
	switch (cpu->vendor) {
		case VENDOR_AMD:
		case VENDOR_INTEL:
			flag_decode(cpu->flags_ecx, "1:ecx", cap_generic_ecx_flags, cap_generic_ecx_flags_desc);
			printf("\n");
			if (cpu->maxei < 0x80000001)
				break;
			printf("Extended feature flags:\n");
			flag_decode(cpu->eflags_edx, "80000001:edx", cap_extended_edx_flags, cap_extended_edx_flags_desc);
			flag_decode(cpu->eflags_ecx, "80000001:ecx", cap_extended_ecx_flags, cap_extended_ecx_flags_desc);
			break;

		case VENDOR_CENTAUR:
			printf("\n");
			printf("Extended feature flags:\n");
			flag_decode(cpu->flags_ecx, "1:ecx", centaur_cap_ecx_flags, NULL);
			if (cpu->maxei >= 0x80000001)
				flag_decode(cpu->eflags_edx, "80000001:edx", centaur_cap_80000001_edx_flags, NULL);
			if (cpu->maxei2 >= 0xc0000001) {
				cpuid(cpu->number, 0xc0000001, &eax, &ebx, &ecx, &edx);
				flag_decode(edx, "c0000001:edx", centaur_cap_extended_edx_flags, NULL);
			}
			break;

		case VENDOR_TRANSMETA:
			printf("\n");
			printf("Extended feature flags:\n");
			cpuid(cpu->number, 0x80860000, &eax, &ebx, &ecx, &edx);
			if (eax >= 0x80860001) {
				cpuid(cpu->number, 0x80860001, &eax, &ebx, &ecx, &edx);
				flag_decode(edx, "80860001:edx", transmeta_cap_flags, NULL);
			}
			break;

		case VENDOR_CYRIX:
			printf("\n");
			break;

		case VENDOR_NATSEMI:
		case VENDOR_RISE:
		case VENDOR_SIS:
		case VENDOR_UNKNOWN:
		default:
			/* Unknown CPU manufacturer or no special handling needed */
			break;
	}

	show_extra_flags(cpu);

	printf("\n");
}

void display_feature_flags(const struct cpudata *cpu)
{
	decode_feature_flags(cpu);
	if (!bind_cpu(cpu->number)) {
		int longnop = host_longnop();

		printf("Long NOPs supported: %s\n",
		       longnop < 0 ? "not recorded" : longnop ? "yes" : "no");
	}
	printf("\n");
}
