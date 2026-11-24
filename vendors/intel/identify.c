/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Intel specific parts
 *
 * References:
 *	http://developer.intel.com/
 *	http://microcodes.sourceforge.net/CPUID.htm
 */

#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include "intel.h"

/* Families 0x12 (efamily 3) and 0x13 (efamily 4). */
static const struct {
	unsigned int efamily;
	unsigned int model;
	const char *str;
} intel_efamily_names[] = {
	{ .efamily = 3, .model = 0x1, .str = "[Nova Lake]" },
	{ .efamily = 3, .model = 0x3, .str = "[Nova Lake L]" },
	{ .efamily = 4, .model = 0x1, .str = "[Diamond Rapids]" },
};

static const char *extended_family_name(const struct cpudata *cpu)
{
	unsigned int i;

	for (i = 0; i < sizeof(intel_efamily_names) / sizeof(intel_efamily_names[0]); i++) {
		if (intel_efamily_names[i].efamily == cpu->efamily &&
		    intel_efamily_names[i].model == model(cpu))
			return intel_efamily_names[i].str;
	}
	return "Unknown model.";
}

void identify_intel(struct cpudata *cpu)
{
	unsigned int eax, ebx;

	cpuid(cpu->number, 0x00000001, &eax, &ebx, NULL, NULL);
	cpu->type = (eax >> 12) & 0x3;

	cpu->brand = ebx & 0xff;

	cpu->x86_max_cores = intel_num_cpu_cores(cpu);

	size_intel_caches(cpu);

	switch (cpu->family) {
	case 4:	add_to_cpuname("i486 ");
		switch (cpu->model) {
		case 0:
			add_to_cpuname("DX-25/33");
			break;
		case 1:
			add_to_cpuname("DX-50");
			break;
		case 2:
			add_to_cpuname("SX");
			break;
		case 3:
			add_to_cpuname("487/DX2");
			break;
		case 4:
			add_to_cpuname("SL");
			break;
		case 5:
			add_to_cpuname("SX2");
			break;
		case 7:
			add_to_cpuname("write-back enhanced DX2");
			break;
		case 8:
			add_to_cpuname("DX4");
			cpu->connector = CONN_SOCKET_3;
			break;
		case 9:
			add_to_cpuname("write-back enhanced DX4");
			cpu->connector = CONN_SOCKET_3;
			break;
		}
		break;

	case 5:
		if (cpu->model < 9)
			add_to_cpuname("Pentium ");
		else
			add_to_cpuname("Quark ");

		switch (cpu->model) {
		case 0:
			add_to_cpuname("A-step");
			cpu->connector = CONN_SOCKET_4;
			break;
		case 1:
			add_to_cpuname("60/66");
			cpu->connector = CONN_SOCKET_4;
			break;
		case 2:
			add_to_cpuname("75-200");
			cpu->connector = CONN_SOCKET_5_7;
			break;
		case 3:
			add_to_cpuname("Overdrive");
			cpu->connector = CONN_SOCKET_4;
			break;
		case 4:
			add_to_cpuname("MMX");
			cpu->connector = CONN_SOCKET_7;
			break;
		case 7:
			add_to_cpuname("Mobile");
			cpu->connector = CONN_SOCKET_7;
			break;
		case 8:
			add_to_cpuname("MMX Mobile");
			cpu->connector = CONN_SOCKET_7;
			break;
		case 9:
			add_to_cpuname("X1000");
			break;
		}
		break;

	case 0x6:
		/* Yonah (model 0xe, emodel 0) is only in the extended table */
		if (cpu->emodel == 0 && cpu->model != 0xe)
			identify_intel_family6pentium(cpu);
		else
			identify_intel_family6core(cpu);
		break;

	case 0x7:
		add_to_cpuname("Itanium");
		break;

	case 0xF:
		switch (cpu->efamily) {
		case 0:
			identify_intel_family15(cpu);
			break;
		default:
			add_to_cpuname(extended_family_name(cpu));
		}
		break;
	}
}
