#ifndef _INTEL_H
#define _INTEL_H

void decode_Intel_caches (struct cpudata *cpu, int output);
void show_Intel_caches(struct cpudata *cpu);
void decode_Intel_machine_check(int cpunum, int family);

void dump_p4_MSRs(struct cpudata *cpu);
void dump_performance_MSRs(struct cpudata *cpu);
void dump_thermal_MSRs(struct cpudata *cpu);
void dump_IDA_MSRs(struct cpudata *cpu);

void decode_microcode(struct cpudata *cpu);

void Identify_Intel_family6pentium(struct cpudata *cpu);
void Identify_Intel_family6core(struct cpudata *cpu);
void Identify_Intel_family15(struct cpudata *cpu);

void Identify_Intel(struct cpudata *cpu);
void display_basic_Intel_info(struct cpudata *cpu);
void display_extended_Intel_info(struct cpudata *cpu);

void decode_serial_number(struct cpudata *cpu);
void get_intel_topology(struct cpudata *cpu);

#define MSR_IA32_PLATFORM_ID	0x17
#define MSR_IA32_UCODE_REV		0x8b

#define MSR_IA32_PERF_STATUS		0x198
#define MSR_IA32_PERF_CTL		0x199
#define MSR_IA32_THERM_CONTROL		0x19a
#define MSR_IA32_THERM_STATUS		0x19c
#define MSR_PM_THERM2_CTL		0x19d
#define MSR_IA32_MISC_ENABLE		0x1a0
#define MSR_IA32_ENERGY_PERF_BIAS	0x1b0

#endif /* _INTEL_H */
