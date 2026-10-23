#pragma once

void dump_C3_MSR (struct cpudata *cpu);

enum c3_power {
	C3_LONGHAUL,
	C3_POWERSAVER,
};
void decode_c3_power(struct cpudata *cpu, enum c3_power gen);
void identify_centaur(struct cpudata *cpu);
void display_centaur_info(struct cpudata *cpu);
