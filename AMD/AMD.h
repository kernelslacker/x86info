#ifndef _AMD_H
#define _AMD_H
void decode_athlon_bluesmoke(int cpunum);
void dump_athlon_MSR(struct cpudata *cpu);
void dump_k6_MSR (struct cpudata *cpu);
#endif /* _AMD_H */
