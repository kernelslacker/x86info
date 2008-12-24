/*
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD specific prototypes.
 */

#ifndef _AMD_H
#define _AMD_H
extern void decode_athlon_bluesmoke(int cpunum);
extern void dump_athlon_MSR(struct cpudata *cpu);
extern void dump_k6_MSR (struct cpudata *cpu);
extern void decode_powernow (struct cpudata *cpu);
extern void show_amd_bugs(struct cpudata *cpu);
extern void dump_PSB(struct cpudata *cpu, unsigned int maxfid, unsigned int startvid);

#define MSR_CLKCTL	0xc001001b

#endif /* _AMD_H */
