/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Feature flag decoding.
 */

#include <stdio.h>
#include <x86info.h>

static void flag_decode(unsigned long reg, const char * reg_desc, const struct flag_info flags[])
{
	unsigned int i;

	for (i = 0; i < 32; i++) {
		const char *name = flags[i].name;
		const char *desc = flags[i].desc;

		if (!(reg & (1UL << i)))
			continue;

		if (!opts.verbose) {
			if (name)
				printf(" %s", name);
			else
				printf(" [%s:%u]", reg_desc, i);
		} else if (desc) {
			if (name)
				printf(" %-8s\t%s\n", name, desc);
			else
				printf(" [%s:%u]     \t%s\n", reg_desc, i, desc);
		} else {
			if (name)
				printf(" %s\n", name);
			else
				printf(" [%s:%u]\n", reg_desc, i);
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
static const struct flag_info cpuid_06_eax_flags[32] = {
	[0] = { "dts", "Digital temperature sensor supported" },
	[1] = { "ida", "Intel Dynamic Acceleration Technology (Turbo Boost)" },
	[2] = { "arat", "Always Running APIC Timer" },
	[4] = { "pln", "Power limit notification controls" },
	[5] = { "ecmd", "Clock modulation duty cycle extension" },
	[6] = { "ptm", "Package thermal management" },
	[7] = { "hwp", "HW-controlled performance states (HWP)" },
	[8] = { "hwp_notify", "HWP Notification" },
	[9] = { "hwp_act_window", "HWP Activity Window" },
	[10] = { "hwp_epp", "HWP Energy Performance Preference" },
	[11] = { "hwp_lpr", "HWP Package Level Request" },
	[13] = { "hdc", "Hardware Duty Cycling" },
	[15] = { "hwp_highest_perf_change", "HWP highest performance change" },
	[16] = { "hwp_peci_override", "HWP PECI override" },
	[17] = { "hwp_flexible", "Flexible HWP" },
	[18] = { "hwp_fast_access", "Fast access, low latency and posted HWP requests" },
	[19] = { "hfi", "Hardware Feedback Interface" },
	[20] = { "hwp_ignore_idle", "Ignore HWP requests from idle logical processors" },
	[22] = { "hwp_ctl", "IA32_HWP_CTL MSR" },
	[24] = { "dpti", "Directed Package Thermal Interrupt" },
};

/* CPUID 0x00000007 ECX flags (subleaf 0) */
static const struct flag_info cpuid_07_ecx_flags[32] = {
	[0] = { "prefetchwt1", "PREFETCHWT1 instruction" },
	[1] = { "avx512vbmi", "AVX-512 Vector Bit Manipulation Instructions" },
	[2] = { "umip", "User-Mode Instruction Prevention (UMIP)" },
	[3] = { "pku", "Protection Keys for User-mode pages" },
	[4] = { "ospke", "OS has set CR4.PKE" },
	[5] = { "waitpkg", "TPAUSE/UMONITOR/UMWAIT instructions" },
	[6] = { "avx512vbmi2", "AVX-512 Vector Bit Manipulation Instructions 2" },
	[7] = { "cet_ss", "CET Shadow Stack" },
	[8] = { "gfni", "Galois Field New Instructions" },
	[9] = { "vaes", "Vector AES (VAES)" },
	[10] = { "vpclmulqdq", "VPCLMULQDQ instruction" },
	[11] = { "avx512vnni", "AVX-512 Vector Neural Network Instructions" },
	[12] = { "avx512bitalg", "AVX-512 Bit Algorithm Instructions" },
	[13] = { "tme", "Total Memory Encryption" },
	[14] = { "avx512vpopcntdq", "AVX-512 Vector Population Count D/Q" },
	[16] = { "la57", "5-level paging (57-bit VA)" },
	[22] = { "rdpid", "Read Processor ID (RDPID)" },
	[23] = { "kl", "Key Locker" },
	[24] = { "bus_lock_detect", "Bus Lock Detection" },
	[25] = { "cldemote", "Cache Line Demote (CLDEMOTE)" },
	[27] = { "movdiri", "MOVDIRI instruction" },
	[28] = { "movdir64b", "MOVDIR64B instruction" },
	[29] = { "enqcmd", "ENQCMD instruction" },
	[30] = { "sgx_lc", "SGX Launch Configuration" },
	[31] = { "pks", "Protection Keys for Supervisor-mode" },
};

/* CPUID 0x00000007 EDX flags (subleaf 0) */
static const struct flag_info cpuid_07_edx_flags[32] = {
	[2] = { "avx512_4vnniw", "AVX-512 4-register Neural Network Instructions" },
	[3] = { "avx512_4fmaps", "AVX-512 4-register Multiply Accumulation Single" },
	[4] = { "fsrm", "Fast Short REP MOVSB" },
	[5] = { "uintr", "User Interrupts" },
	[8] = { "avx512vp2intersect", "AVX-512 VP2INTERSECT Instructions" },
	[9] = { "srbds_ctrl", "SRBDS mitigation control MSR" },
	[10] = { "md_clear", "VERW clears CPU buffers (MD_CLEAR)" },
	[11] = { "rtm_always_abort", "RTM always aborts" },
	[13] = { "rtm_force_abort", "RTM force abort MSR" },
	[14] = { "serialize", "SERIALIZE instruction" },
	[15] = { "hybrid", "Hybrid CPU (big/little cores)" },
	[16] = { "tsxldtrk", "TSX Suspend Load Address Tracking" },
	[18] = { "pconfig", "Platform Configuration (PCONFIG)" },
	[19] = { "arch_lbr", "Architectural Last Branch Records" },
	[20] = { "ibt", "Indirect Branch Tracking (CET IBT)" },
	[22] = { "amx_bf16", "AMX BFloat16" },
	[23] = { "avx512_fp16", "AVX-512 FP16" },
	[24] = { "amx_tile", "AMX Tile Architecture" },
	[25] = { "amx_int8", "AMX Int8" },
	[26] = { "ibrs_ibpb", "IBRS / IBPB indirect branch controls" },
	[27] = { "stibp", "Single Thread Indirect Branch Predictors (STIBP)" },
	[28] = { "l1d_flush", "L1D cache flush" },
	[29] = { "arch_capabilities", "IA32_ARCH_CAPABILITIES MSR" },
	[30] = { "core_capabilities", "IA32_CORE_CAPABILITIES MSR" },
	[31] = { "ssbd", "Speculative Store Bypass Disable (SSBD)" },
};

/* CPUID 0x00000007 EAX flags (subleaf 1) */
static const struct flag_info cpuid_07_1_eax_flags[32] = {
	[0] = { "sha512", "SHA512 instructions" },
	[1] = { "sm3", "SM3 instructions" },
	[2] = { "sm4", "SM4 instructions" },
	[4] = { "avx_vnni", "AVX Vector Neural Network Instructions" },
	[5] = { "avx512_bf16", "AVX-512 BF16 Instructions" },
	[6] = { "lass", "Linear Address Space Separation" },
	[7] = { "cmpccxadd", "CMPccXADD instructions" },
	[8] = { "arch_perfmon_ext", "Architectural PerfMon extension (leaf 0x23)" },
	[10] = { "fzrm", "Fast Zero-length REP MOVSB" },
	[11] = { "fsrs", "Fast Short REP STOSB" },
	[12] = { "fsrc", "Fast Short REP CMPSB/SCASB" },
	[17] = { "fred", "Flexible Return and Event Delivery (FRED)" },
	[18] = { "lkgs", "LKGS instruction" },
	[19] = { "wrmsrns", "WRMSRNS instruction" },
	[20] = { "nmi_src", "NMI-source reporting" },
	[21] = { "amx_fp16", "AMX FP16 Instructions" },
	[22] = { "hreset", "HRESET instruction" },
	[23] = { "avx_ifma", "AVX Integer Fused Multiply-Add" },
	[26] = { "lam", "Linear Address Masking (LAM)" },
	[27] = { "msrlist", "RDMSRLIST/WRMSRLIST instructions" },
	[31] = { "movrs", "MOVRS instructions" },
};

/* CPUID 0x00000007 EDX flags (subleaf 1) */
/* CPUID 0x00000007 subleaf 1 EBX flags */
static const struct flag_info cpuid_07_1_ebx_flags[32] = {
	[0] = { "intel_ppin", "Protected Processor Inventory Number" },
};

static const struct flag_info cpuid_07_1_edx_flags[32] = {
	[4] = { "avx_vnni_int8", "AVX-VNNI-INT8 instructions" },
	[5] = { "avx_ne_convert", "AVX-NE-CONVERT instructions" },
	[8] = { "amx_complex", "AMX-COMPLEX instructions" },
	[10] = { "avx_vnni_int16", "AVX-VNNI-INT16 instructions" },
	[14] = { "prefetchit0_1", "PREFETCHIT0/1 instructions" },
	[15] = { "user_msr", "User-mode MSR access (URDMSR/UWRMSR)" },
	[18] = { "cet_sss", "CET supervisor shadow stacks safe to use" },
	[19] = { "avx10", "AVX10" },
	[21] = { "apx_f", "Advanced Performance Extensions (APX_F)" },
};

/* CPUID 0x00000007 EBX flags */
static const struct flag_info cpuid_07_ebx_flags[32] = {
	[0] = { "fsgsbase", "Access to base of %fs and %gs" },
	[1] = { "tsc_adj", "TSC_ADJUST MSR" },
	[2] = { "sgx", "Software Guard Extensions" },
	[3] = { "bmi1", "Bit Manipulation Instruction Set" },
	[4] = { "hle", "Hardware Lock Elision" },
	[5] = { "avx2", "Advanced Vector Extensions 2" },
	[6] = { "fdp_excp", "FPU Data Pointer updated only on x87 exceptions" },
	[7] = { "smep", "Supervisor-Mode Execution Prevention" },
	[8] = { "bmi2", "Bit Manipulation Instruction Set 2" },
	[9] = { "erms", "Enhanced REP MOVSB/STOSB" },
	[10] = { "invpcid", "INVPCID instruction" },
	[11] = { "rtm", "Transactional Synchronization Extensions" },
	[12] = { "pqm", "Platform Quality of Service Monitoring" },
	[13] = { "nofpucs", "FPU CS and FPU DS deprecated" },
	[14] = { "mpx", "Intel MPX (Memory Protection Extensions)" },
	[15] = { "pqe", "Platform Quality of Service Enforcement" },
	[16] = { "avx512f", "AVX-512 Foundation" },
	[17] = { "avx512dq", "AVX-512 Doubleword and Quadword Instructions" },
	[18] = { "rdseed", "RDSEED instruction" },
	[19] = { "adx", "Intel ADX (Multi-Precision Add-Carry Instruction Extensions)" },
	[20] = { "smap", "Supervisor Mode Access Prevention" },
	[21] = { "avx512ifma", "AVX-512 Integer Fused Multiply-Add Instructions" },
	[22] = { "pcommit", "PCOMMIT instruction" },
	[23] = { "clflushopt", "CLFLUSHOPT instruction" },
	[24] = { "clwb", "CLWB instruction" },
	[25] = { "ipt", "Intel Processor Trace" },
	[26] = { "avx512pf", "AVX-512 Prefetch Instructions" },
	[27] = { "avx512er", "AVX-512 Exponential and Reciprocal Instructions" },
	[28] = { "avx512cd", "AVX-512 Conflict Detection Instructions" },
	[29] = { "sha", "Intel SHA extensions" },
	[30] = { "avx512bw", "AVX-512 Byte and Word Instructions" },
	[31] = { "avx512vl", "AVX-512 Vector Length Extensions" },
};

	/* CPUID 0x80000007 EDX flags */
const struct flag_info cpuid_80000007_edx_flags[32] = {
	[0] = { "ts", "Temperature sensing diode present" },
	[1] = { "fid", "Frequency ID control" },
	[2] = { "vid", "Voltage ID control" },
	[3] = { "ttp", "Thermal Trip" },
	[4] = { "tm", "Thermal Monitoring" },
	[5] = { "stc", "Software Thermal Control" },
	[6] = { "100mhzsteps", "100MHz multiplier control" },
	[7] = { "hwpstate", "Hardware P-state control" },
	[8] = { "nonstop_tsc", "Invariant/nonstop/constant TSC" },
	[9] = { "cpb", "Core Performance Boost" },
	[10] = { "eff_freq_ro", "read-only Effective Frequency Interface" },
	[11] = { "proc_feedback", "Processor Feedback Interface" },
	[12] = { "acc_power", "Processor Power Reporting" },
	[13] = { "connected_standby", "Connected Standby" },
	[14] = { "rapl", "RAPL" },
};

/* CPUID 0x0000000d subleaf 1 EAX flags */
static const struct flag_info cpuid_0d_1_eax_flags[32] = {
	[0] = { "xsaveopt", NULL },
	[1] = { "xsavec", NULL },
	[2] = { "xgetbv1", NULL },
	[3] = { "xsaves", NULL },
	[4] = { "xfd", NULL },
};

/* CPUID 0x00000007 EDX flags (subleaf 2) */
static const struct flag_info cpuid_07_2_edx_flags[32] = {
	[0] = { "intel_psfd", NULL },
	[1] = { "ipred_ctrl", NULL },
	[2] = { "rrsba_ctrl", NULL },
	[3] = { "ddp_ctrl", NULL },
	[4] = { "bhi_ctrl", NULL },
	[5] = { "mcdt_no", NULL },
	[6] = { "uclock_disable", NULL },
	[7] = { "monitor_mitg_no", NULL },
};

/* CPUID 0x80000007 EBX flags */
static const struct flag_info cpuid_80000007_ebx_flags[32] = {
	[0] = { "overflow_recov", NULL },
	[1] = { "succor", NULL },
	[3] = { "smca", NULL },
};

/* CPUID 0x80000008 EBX flags */
static const struct flag_info cpuid_80000008_ebx_flags[32] = {
	[0] = { "clzero", NULL },
	[1] = { "irperf", NULL },
	[2] = { "xsaveerptr", NULL },
	[3] = { "invlpgb", NULL },
	[4] = { "rdpru", NULL },
	[6] = { "mba", NULL },
	[8] = { "mcommit", NULL },
	[9] = { "wbnoinvd", NULL },
	[12] = { "ibpb", NULL },
	[13] = { "wbinvd_int", NULL },
	[14] = { "ibrs", NULL },
	[15] = { "stibp", NULL },
	[16] = { "ibrs_always_on", NULL },
	[17] = { "stibp_always_on", NULL },
	[18] = { "ibrs_fast", NULL },
	[19] = { "ibrs_same_mode", NULL },
	[20] = { "no_efer_lmsle", NULL },
	[21] = { "tlb_flush_nested", NULL },
	[23] = { "amd_ppin", NULL },
	[24] = { "amd_ssbd", NULL },
	[25] = { "virt_ssbd", NULL },
	[26] = { "amd_ssb_no", NULL },
	[27] = { "cppc", NULL },
	[28] = { "amd_psfd", NULL },
	[29] = { "btc_no", NULL },
	[30] = { "ibpb_ret", NULL },
	[31] = { "branch_sampling", NULL },
};

/* CPUID 0x8000001f EAX flags */
static const struct flag_info cpuid_8000001f_eax_flags[32] = {
	[0] = { "sme", NULL },
	[1] = { "sev", NULL },
	[2] = { "vm_page_flush", NULL },
	[3] = { "sev_es", NULL },
	[4] = { "sev_snp", NULL },
	[5] = { "vmpl", NULL },
	[6] = { "rmpquery", NULL },
	[7] = { "vmpl_sss", NULL },
	[8] = { "secure_tsc", NULL },
	[9] = { "v_tsc_aux", NULL },
	[10] = { "sme_coherent", NULL },
	[11] = { "req_64bit_hypervisor", NULL },
	[12] = { "restricted_injection", NULL },
	[13] = { "alternate_injection", NULL },
	[14] = { "debug_swap", NULL },
	[15] = { "disallow_host_ibs", NULL },
	[16] = { "virt_transparent_enc", NULL },
	[17] = { "vmgexit_parameter", NULL },
	[18] = { "virt_tom_msr", NULL },
	[19] = { "virt_ibs", NULL },
	[21] = { "rmpread", NULL },
	[23] = { "segmented_rmp", NULL },
	[24] = { "vmsa_reg_protection", NULL },
	[25] = { "smt_protection", NULL },
	[27] = { "allowed_sev_features", NULL },
	[28] = { "svsm", NULL },
	[29] = { "nested_virt_snp_msr", NULL },
	[30] = { "hv_inuse_wr_allowed", NULL },
};

/* CPUID 0x80000020 EBX flags */
static const struct flag_info cpuid_80000020_ebx_flags[32] = {
	[1] = { "mba", NULL },
	[2] = { "smba", NULL },
	[3] = { "bmec", NULL },
	[4] = { "l3rr", NULL },
	[5] = { "abmc", NULL },
	[6] = { "sdciae", NULL },
};

/* CPUID 0x80000021 EAX flags (AMD APM volume 3, 24594 rev. 3.37). */
static const struct flag_info cpuid_80000021_eax_flags[32] = {
	[0] = { "no_nested_data_bp", NULL },
	[1] = { "fsgs_non_serializing", NULL },
	[2] = { "lfence_serializing", NULL },
	[3] = { "smm_page_cfg_lock", NULL },
	[4] = { "tlbsync_sfw_no", NULL },
	[5] = { "verw_clear", NULL },
	[6] = { "null_sel_clr_base", NULL },
	[7] = { "upper_addr_ignore", NULL },
	[8] = { "auto_ibrs", NULL },
	[9] = { "no_smm_ctl_msr", NULL },
	[10] = { "fsrs", NULL },
	[11] = { "fsrc", NULL },
	[12] = { "pmc2_precise_retire", NULL },
	[13] = { "prefetch_ctl_msr", NULL },
	[14] = { "l2_tlb_size_x32", NULL },
	[15] = { "amd_ermsb", NULL },
	[16] = { "opcode_reclaim", NULL },
	[17] = { "user_cpuid_disable", NULL },
	[18] = { "epsf", NULL },
	[19] = { "fast_rep_scasb", NULL },
	[20] = { "prefetchi", NULL },
	[21] = { "fp512_downgrade", NULL },
	[22] = { "wl_feedback", NULL },
	[23] = { "avx512_bmm", NULL },
	[24] = { "eraps", NULL },
	[27] = { "sbpb", NULL },
	[28] = { "ibpb_brtype", NULL },
	[29] = { "srso_no", NULL },
	[30] = { "srso_user_kernel_no", NULL },
	[31] = { "srso_bp_spec_reduce", NULL },
};

/* CPUID 0x80000021 ECX flags */
static const struct flag_info cpuid_80000021_ecx_flags[32] = {
	[1] = { "tsa_sq_no", NULL },
	[2] = { "tsa_l1_no", NULL },
	[8] = { "btb_ctx_isolation", NULL },
};

/* CPUID 0x80000022 EAX flags */
static const struct flag_info cpuid_80000022_eax_flags[32] = {
	[0] = { "perfmon_v2", NULL },
	[1] = { "lbr_v2", NULL },
	[2] = { "lbr_pmc_freeze", NULL },
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
		flag_decode(eax, "6:eax", cpuid_06_eax_flags);
	}
	// CPUID 0x07 subleaf 0
	if (cpu->cpuid_level >= 0x07) {
		unsigned int max_subleaf;
		cpuid_count(cpu->number, 0x07, 0, &eax, &ebx, &ecx, &edx);
		max_subleaf = eax;
		flag_decode(ebx, "7:ebx", cpuid_07_ebx_flags);
		flag_decode(ecx, "7:ecx", cpuid_07_ecx_flags);
		flag_decode(edx, "7:edx", cpuid_07_edx_flags);
		// CPUID 0x07 subleaf 1
		if (max_subleaf >= 1) {
			cpuid_count(cpu->number, 0x07, 1, &eax, &ebx, &ecx, &edx);
			flag_decode(eax, "7.1:eax", cpuid_07_1_eax_flags);
			flag_decode(ebx, "7.1:ebx", cpuid_07_1_ebx_flags);
			flag_decode(edx, "7.1:edx", cpuid_07_1_edx_flags);
		}
		// CPUID 0x07 subleaf 2
		if (max_subleaf >= 2) {
			cpuid_count(cpu->number, 0x07, 2, &eax, &ebx, &ecx, &edx);
			flag_decode(edx, "7.2:edx", cpuid_07_2_edx_flags);
		}
	}
	// CPUID 0x0d subleaf 1
	if (cpu->cpuid_level >= 0x0d) {
		cpuid_count(cpu->number, 0x0d, 1, &eax, &ebx, &ecx, &edx);
		flag_decode(eax, "d.1:eax", cpuid_0d_1_eax_flags);
	}
	// CPUID 0x80000007
	if (cpu->maxei >= 0x80000007)
		flag_decode(cpu->apm_edx, "80000007:edx", cpuid_80000007_edx_flags);

	if (cpu->vendor != VENDOR_AMD)
		return;

	if (cpu->maxei >= 0x80000007) {
		cpuid(cpu->number, 0x80000007, NULL, &ebx, NULL, NULL);
		flag_decode(ebx, "80000007:ebx", cpuid_80000007_ebx_flags);
	}
	if (cpu->maxei >= 0x80000008) {
		cpuid(cpu->number, 0x80000008, NULL, &ebx, NULL, NULL);
		flag_decode(ebx, "80000008:ebx", cpuid_80000008_ebx_flags);
	}
	if (cpu->maxei >= 0x8000001f) {
		cpuid(cpu->number, 0x8000001f, &eax, NULL, NULL, NULL);
		flag_decode(eax, "8000001f:eax", cpuid_8000001f_eax_flags);
	}
	if (cpu->maxei >= 0x80000020) {
		cpuid_count(cpu->number, 0x80000020, 0, NULL, &ebx, NULL, NULL);
		flag_decode(ebx, "80000020:ebx", cpuid_80000020_ebx_flags);
	}
	if (cpu->maxei >= 0x80000021) {
		cpuid(cpu->number, 0x80000021, &eax, NULL, &ecx, NULL);
		flag_decode(eax, "80000021:eax", cpuid_80000021_eax_flags);
		flag_decode(ecx, "80000021:ecx", cpuid_80000021_ecx_flags);
	}
	if (cpu->maxei >= 0x80000022) {
		cpuid_count(cpu->number, 0x80000022, 0, &eax, NULL, NULL, NULL);
		flag_decode(eax, "80000022:eax", cpuid_80000022_eax_flags);
	}
}

/* CPUID 0x00000001 EDX flags */
static const struct flag_info generic_cap_flags[32] = {
	[0] = { "fpu", "Onboard FPU" },
	[1] = { "vme", "Virtual Mode Extensions" },
	[2] = { "de", "Debugging Extensions" },
	[3] = { "pse", "Page Size Extensions" },
	[4] = { "tsc", "Time Stamp Counter" },
	[5] = { "msr", "Model-Specific Registers" },
	[6] = { "pae", "Physical Address Extensions" },
	[7] = { "mce", "Machine Check Exception" },
	[8] = { "cx8", "CMPXCHG8 instruction" },
	[9] = { "apic", "Onboard APIC" },
	[11] = { "sep", "SYSENTER/SYSEXIT instructions" },
	[12] = { "mtrr", "Memory Type Range Registers" },
	[13] = { "pge", "Page Global Enable" },
	[14] = { "mca", "Machine Check Architecture" },
	[15] = { "cmov", "CMOV instruction" },
	[16] = { "pat", "Page Attribute Table" },
	[17] = { "pse36", "36-bit PSEs" },
	[18] = { "psn", "Processor serial number" },
	[19] = { "clflsh", "CLFLUSH instruction" },
	[21] = { "ds", "Debug Trace Store" },
	[22] = { "acpi", "ACPI via MSR" },
	[23] = { "mmx", "MMX support" },
	[24] = { "fxsr", "FXSAVE and FXRSTOR instructions" },
	[25] = { "sse", "SSE support" },
	[26] = { "sse2", "SSE2 support" },
	[27] = { "ss", "CPU self snoop" },
	[28] = { "ht", "Hyper-Threading" },
	[29] = { "tm", "Thermal Monitor" },
	[31] = { "pbe", "Pending Break Enable" },
};

/* CPUID 0x00000001 ECX flags */
static const struct flag_info cap_generic_ecx_flags[32] = {
	[0] = { "sse3", "Streaming SIMD Extensions 3" },
	[1] = { "pclmulqdq", "PCLMULDQ Instruction" },
	[2] = { "dtes64", "64-Bit Debug Store" },
	[3] = { "monitor", "MONITOR/MWAIT" },
	[4] = { "ds-cpl", "CPL Qualified Debug Store" },
	[5] = { "vmx", "Virtual Machine Extensions" },
	[6] = { "smx", "Safer Mode Extensions" },
	[7] = { "est", "Enhanced Intel SpeedStep Technology" },
	[8] = { "tm2", "Thermal Monitor 2" },
	[9] = { "ssse3", "Supplemental Streaming SIMD Extensions 3" },
	[10] = { "cid", "L1 Context ID" },
	[11] = { "sdbg", "Silicon Debug" },
	[12] = { "fma", "Fused Multiply Add" },
	[13] = { "cx16", "CMPXCHG16B" },
	[14] = { "xTPR", "xTPR Update Control" },
	[15] = { "pdcm", "Perfmon and Debug Capability" },
	[17] = { "pcid", "Process-context identifiers" },
	[18] = { "dca", "Direct Cache Access" },
	[19] = { "sse4_1", "Streaming SIMD Extensions 4.1" },
	[20] = { "sse4_2", "Streaming SIMD Extensions 4.2" },
	[21] = { "x2apic", "Extended xAPIC Support" },
	[22] = { "movbe", "MOVBE Instruction" },
	[23] = { "popcnt", "POPCNT Instruction" },
	[24] = { "tsc-deadline", "TSC Deadline support" },
	[25] = { "aes", "AES Instruction" },
	[26] = { "xsave", "XSAVE/XRSTOR States" },
	[27] = { "osxsave", "OS-Enabled Extended State Management" },
	[28] = { "avx", "AVX instruction extensions" },
	[29] = { "f16c", "F16C (half-precision) Floating Point" },
	[30] = { "rdrnd", "RDRAND (hardware RNG)" },
};
/* CPUID 0x80000001 EDX flags */
static const struct flag_info cap_extended_edx_flags[32] = {
	[0] = { "fpu", NULL },
	[1] = { "vme", NULL },
	[2] = { "de", NULL },
	[3] = { "pse", NULL },
	[4] = { "tsc", NULL },
	[5] = { "msr", NULL },
	[6] = { "pae", NULL },
	[7] = { "mce", NULL },
	[8] = { "cx8", NULL },
	[9] = { "apic", NULL },
	[11] = { "syscall", "SYSCALL/SYSRET instructions" },
	[12] = { "mtrr", NULL },
	[13] = { "pge", NULL },
	[14] = { "mca", NULL },
	[15] = { "cmov", NULL },
	[16] = { "pat", NULL },
	[17] = { "pse36", NULL },
	[19] = { "mp", NULL },
	[20] = { "nx", "Execution Disable Bit" },
	[22] = { "mmxext", NULL },
	[23] = { "mmx", NULL },
	[24] = { "fxsr", NULL },
	[25] = { "ffxsr", NULL },
	[26] = { "page1gb", "1-GByte pages" },
	[27] = { "rdtscp", "RDTSCP and IA32_TSC_AUX" },
	[29] = { "lm", "64-bit Long Mode" },
	[30] = { "3dnowext", NULL },
	[31] = { "3dnow", NULL },
};

/* CPUID 0x80000001 ECX flags */
static const struct flag_info cap_extended_ecx_flags[32] = {
	[0] = { "lahf_lm", "LAHF/SAHF available in 64-bit mode" },
	[1] = { "CmpLegacy", NULL },
	[2] = { "svm", NULL },
	[3] = { "ExtApicSpace", NULL },
	[4] = { "LockMovCr0", NULL },
	[5] = { "abm", "LZCNT/ABM instructions" },
	[6] = { "sse4a", NULL },
	[7] = { "misalignsse", NULL },
	[8] = { "3dnowprefetch", "PREFETCHW instruction" },
	[9] = { "osvw", NULL },
	[10] = { "ibs", NULL },
	[11] = { "xop", NULL },
	[12] = { "skinit", NULL },
	[13] = { "wdt", NULL },
	[15] = { "lwp", NULL },
	[16] = { "fma4", NULL },
	[17] = { "tce", NULL },
	[19] = { "NodeId", NULL },
	[21] = { "tbm", NULL },
	[22] = { "TopoExt", NULL },
	[23] = { "PerfCtrExtCore", NULL },
	[24] = { "PerfCtrExtNB", NULL },
	[26] = { "DataBPExt", NULL },
	[27] = { "PTSC", "Performance time-stamp counter" },
	[28] = { "PerfCtrExtLLC", NULL },
	[29] = { "MwaitExtended", NULL },
	[30] = { "AdMskExtn", NULL },
};

static const struct flag_info centaur_cap_extended_edx_flags[32] = {
	[2] = { "RNGp", NULL },
	[3] = { "RNGe", NULL },
	[6] = { "ACEp", NULL },
	[7] = { "ACEe", NULL },
	[8] = { "ACE2p", NULL },
	[9] = { "ACE2e", NULL },
	[10] = { "PHEp", NULL },
	[11] = { "PHEe", NULL },
	[12] = { "PMMp", NULL },
	[13] = { "PMMe", NULL },
};

static const struct flag_info transmeta_cap_flags[32] = {
	[0] = { "recovery", NULL },
	[1] = { "longrun", NULL },
	[3] = { "lrti", NULL },
};


static void decode_feature_flags(const struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	printf("Feature flags:\n");
	flag_decode(cpu->flags_edx, "1:edx", generic_cap_flags);

	/* Vendor specific extensions. */
	switch (cpu->vendor) {
		case VENDOR_AMD:
		case VENDOR_INTEL:
			flag_decode(cpu->flags_ecx, "1:ecx", cap_generic_ecx_flags);
			printf("\n");
			if (cpu->maxei < 0x80000001)
				break;
			printf("Extended feature flags:\n");
			flag_decode(cpu->eflags_edx, "80000001:edx", cap_extended_edx_flags);
			flag_decode(cpu->eflags_ecx, "80000001:ecx", cap_extended_ecx_flags);
			break;

		case VENDOR_CENTAUR:
			printf("\n");
			printf("Extended feature flags:\n");
			flag_decode(cpu->flags_ecx, "1:ecx", cap_generic_ecx_flags);
			if (cpu->maxei >= 0x80000001)
				flag_decode(cpu->eflags_edx, "80000001:edx", cap_extended_edx_flags);
			if (cpu->maxei2 >= 0xc0000001) {
				cpuid(cpu->number, 0xc0000001, &eax, &ebx, &ecx, &edx);
				flag_decode(edx, "c0000001:edx", centaur_cap_extended_edx_flags);
			}
			break;

		case VENDOR_TRANSMETA:
			printf("\n");
			printf("Extended feature flags:\n");
			cpuid(cpu->number, 0x80860000, &eax, &ebx, &ecx, &edx);
			if (eax >= 0x80860001) {
				cpuid(cpu->number, 0x80860001, &eax, &ebx, &ecx, &edx);
				flag_decode(edx, "80860001:edx", transmeta_cap_flags);
			}
			break;

		case VENDOR_CYRIX:
			printf("\n");
			break;

		case VENDOR_NATSEMI:
		case VENDOR_RISE:
		case VENDOR_SIS:
			break;

		case VENDOR_UNKNOWN:
		default:
			/* Unknown manufacturer: use the architectural ECX bits */
			flag_decode(cpu->flags_ecx, "1:ecx", cap_generic_ecx_flags);
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
