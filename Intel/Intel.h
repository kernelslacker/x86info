#ifndef _INTEL_H
#define _INTEL_H
void decode_Intel_caches (struct cpudata *cpu, int output);
void show_Intel_caches(struct cpudata *cpu);
void decode_Intel_bluesmoke(int cpunum, int family);
void dump_p4_MSRs(struct cpudata *cpu);
void dump_p6_MSRs(struct cpudata *cpu);
void dump_centrino_MSRs(struct cpudata *cpu);
void decode_microcode(struct cpudata *cpu);

#define MSR_IA32_PLATFORM_ID	0x17
#define MSR_IA32_UCODE_REV		0x8b
#endif /* _INTEL_H */
