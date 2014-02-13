/*
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD specific prototypes.
 */

#ifndef _AMD_H
#define _AMD_H
void decode_athlon_machine_check(int cpunum);
void dump_athlon_MSR(struct cpudata *cpu);
void dump_k6_MSR (struct cpudata *cpu);
void decode_powernow (struct cpudata *cpu);
void show_amd_bugs(struct cpudata *cpu);
void dump_PSB(struct cpudata *cpu, unsigned int maxfid, unsigned int startvid);

void Identify_AMD(struct cpudata *cpu);
void display_AMD_info(struct cpudata *cpu);
void decode_AMD_cacheinfo(struct cpudata *cpu);

#define MSR_CLKCTL	0xc001001b

#endif /* _AMD_H */
