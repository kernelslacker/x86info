#ifndef _AMD_H
#define _AMD_H
void decode_athlon_bluesmoke(int cpunum);
void dump_athlon_MSR(struct cpudata *cpu);
void dump_k6_MSR (struct cpudata *cpu);
void decode_powernow (struct cpudata *cpu);
void show_amd_bugs(struct cpudata *cpu);
void dump_PSB(struct cpudata *cpu, int maxfid, int startvid);

#define MSR_CLKCTL	0xc001001b

#endif /* _AMD_H */
