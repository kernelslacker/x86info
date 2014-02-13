#pragma once

/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD specific prototypes.
 */

#include <linux/types.h>
#include <x86info.h>

void decode_athlon_machine_check(int cpunum);
void dump_athlon_MSR(struct cpudata *cpu);
void dump_k6_MSR (struct cpudata *cpu);
void decode_powernow (struct cpudata *cpu);
void show_amd_bugs(struct cpudata *cpu);
void dump_PSB(struct cpudata *cpu, unsigned int maxfid, unsigned int startvid);

void identify_amd(struct cpudata *cpu);
void display_amd_info(struct cpudata *cpu);
void decode_amd_cacheinfo(struct cpudata *cpu);

void fix_amd_model_name(struct cpudata *cpu, char *cp, size_t len);

#define MSR_CLKCTL	0xc001001b
