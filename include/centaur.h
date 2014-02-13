#ifndef _CENTAUR_H
#define _CENTAUR_H
void dump_C3_MSR (struct cpudata *cpu);
void decode_longhaul(struct cpudata *cpu);
void decode_powersaver(struct cpudata *cpu);
void identify_centaur(struct cpudata *cpu);
void display_centaur_info(struct cpudata *cpu);

#endif /* _CENTAUR_H */
