#ifndef _IDT_H
#define _IDT_H
void dump_C3_MSR (struct cpudata *cpu);
void decode_longhaul(struct cpudata *cpu);
void decode_powersaver(struct cpudata *cpu);
#endif /* _IDT_H */
