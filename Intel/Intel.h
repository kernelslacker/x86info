#ifndef _INTEL_H
#define _INTEL_H
void decode_Intel_caches (struct cpudata *cpu);
void decode_Intel_bluesmoke(int cpunum, int family);
void dump_p4_MSRs(struct cpudata *cpu);
void dump_p6_MSRs(struct cpudata *cpu);
#endif /* _INTEL_H */
