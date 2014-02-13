#ifndef _INTEL_H
#define _INTEL_H

void decode_intel_caches (struct cpudata *cpu, int output);
void show_intel_caches(struct cpudata *cpu);
void decode_intel_machine_check(int cpunum, int family);

void dump_p4_msrs(struct cpudata *cpu);
void dump_performance_msrs(struct cpudata *cpu);
void dump_thermal_msrs(struct cpudata *cpu);
void dump_ida_msrs(struct cpudata *cpu);

void decode_microcode(struct cpudata *cpu);

void identify_intel_family6pentium(struct cpudata *cpu);
void identify_intel_family6core(struct cpudata *cpu);
void identify_intel_family15(struct cpudata *cpu);

void identify_intel(struct cpudata *cpu);
void display_basic_intel_info(struct cpudata *cpu);
void display_extended_intel_info(struct cpudata *cpu);

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
