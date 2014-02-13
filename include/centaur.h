#pragma once

void dump_C3_MSR (struct cpudata *cpu);
void decode_longhaul(struct cpudata *cpu);
void decode_powersaver(struct cpudata *cpu);
void identify_centaur(struct cpudata *cpu);
void display_centaur_info(struct cpudata *cpu);
