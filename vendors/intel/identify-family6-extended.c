/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel family 6 specific decoding (Core family).
 *  All the CPUs described in this file have cpu->emodel set to 1
 */

#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include "intel.h"

#define ONLYMODEL 1

struct cpu_name_def {
	unsigned int model;
	unsigned int stepping;
	unsigned int mhz;
	unsigned int flags;
	unsigned int cores;	/* 0 matches any core count */
	const char *str;
};

static struct cpu_name_def intel_fam6e_names[] = {
	{ .model = 0x17, .cores = 4,.stepping = 6, .mhz = 3000, .str = "Core 2 quad (QX9650) [C0]" },
	{ .model = 0x17, .cores = 4,.stepping = 6, .mhz = 3200, .str = "Core 2 extreme quad (QX9775) [C0]" },

	{ .model = 0x17, .cores = 4,.stepping = 7, .mhz = 3200, .str = "Core 2 quad (QX9770) [C1]" },
	{ .model = 0x17, .cores = 4,.stepping = 7, .mhz = 2830, .str = "Core 2 quad (Q9550) [C1]" },
	{ .model = 0x17, .cores = 4,.stepping = 7, .mhz = 2660, .str = "Core 2 quad (Q9450) [C1]" },
	{ .model = 0x17, .cores = 4,.stepping = 7, .mhz = 2500, .str = "Core 2 quad (Q9300) [M1]" },
	{ .model = 0x17, .cores = 4,.stepping = 7, .mhz = 2330, .str = "Core 2 quad (Q8200) [M1]" },

	{ .model = 0x17, .cores = 4,.stepping = 10, .mhz = 3000, .str = "Core 2 quad (Q9650) [E0]" },
	{ .model = 0x17, .cores = 4,.stepping = 10, .mhz = 2830, .str = "Core 2 quad (Q9550) [E0]" },
	{ .model = 0x17, .cores = 4,.stepping = 10, .mhz = 2660, .str = "Core 2 quad (Q9400) [R0]" },
	{ .model = 0x17, .cores = 4,.stepping = 10, .mhz = 2500, .str = "Core 2 quad (Q8300) [R0]" },

	{ .model = 0x1c,.stepping = 1, .str = "Atom" },

	{ .model = 0x1c,.stepping = 2, .mhz = 800, .str = "Atom Z500/Z515 [C0]" },
	{ .model = 0x1c,.stepping = 2, .mhz = 1100, .str = "Atom Z510 [C0]" },
	{ .model = 0x1c,.stepping = 2, .mhz = 1330, .str = "Atom Z520 [C0]" },
	{ .model = 0x1c,.stepping = 2, .mhz = 1600, .str = "Atom Z530/N270/QKGY1/QGZR2" },
	{ .model = 0x1c,.stepping = 2, .mhz = 1860, .str = "Atom Z540 [C0]" },
	{ .model = 0x1c,.stepping = 2, .mhz = 2000, .str = "Atom Z550 [C0]" },

	{ .model = 0xe, .stepping = 8, .mhz = 1000, .str = "Core Duo U2400/Solo U1300 [Yonah] [C-0]" },
	{ .model = 0xe, .stepping = 8, .mhz = 1200, .str = "Core Solo U1400 [Yonah] [C-0]" },
	{ .model = 0xe, .stepping = 8, .mhz = 1500, .str = "Core Duo L2300 [Yonah] [C-0]" },
	{ .model = 0xe, .stepping = 8, .mhz = 1600, .str = "Core Solo T1300/Duo T2300(E)/Duo L2400 [Yonah]" },
	{ .model = 0xe, .stepping = 8, .mhz = 1800, .str = "Core Solo T1400/Duo T2400 [Yonah] [C-0]" },
	{ .model = 0xe, .stepping = 8, .mhz = 2000, .str = "Core Solo T1500/Duo T2500 [Yonah] [C-0]" },
	{ .model = 0xe, .stepping = 8, .mhz = 2150, .str = "Core Duo T2600 [Yonah] [C-0]" },

	{ .model = 0xe, .stepping = 12, .mhz = 1200, .str = "Core Duo U2500 [Yonah] [D-0]" },
	{ .model = 0xe, .stepping = 12, .mhz = 1800, .str = "Core Duo L2500 [Yonah] [D-0]" },
	{ .model = 0xe, .stepping = 12, .mhz = 2300, .str = "Core Duo T2700 [Yonah] [D-0]" },

	{ .model = 0xe, .flags = ONLYMODEL, .str = "Core Duo/Solo [Yonah]" },

	{ .model = 0x16, .flags = ONLYMODEL, .str = "Core 2 [Merom-L]" },

	{ .model = 0x17, .stepping = 4, .str = "Core 2 Duo [Penryn]" },
	{ .model = 0x17, .stepping = 6, .cores = 2, .str = "Core 2 Duo P8600 [Penryn]" },

	{ .model = 0x1a, .stepping = 4, .mhz = 3200, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [C-0] [SLBCJ]" },
	{ .model = 0x1a, .stepping = 4, .mhz = 2930, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [C-0] [SLBCK]" },
	{ .model = 0x1a, .stepping = 4, .mhz = 2660, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [C-0] [SLBCH]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 3330, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBEQ]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 3200, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBEU]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 3060, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBEN]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 2660, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBEJ]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 2800, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBKP]" },

	{ .model = 0x1a, .flags = ONLYMODEL, .str = "Core i7 (Nehalem)" },

	{ .model = 0x1c, .stepping = 10, .str = "Atom D510 [Pineview]" },
	{ .model = 0x1d, .flags = ONLYMODEL, .str = "Core i7 [Dunnington (MP)]" },
	{ .model = 0x1e, .flags = ONLYMODEL, .str = "Core i7 (Nehalem)" },
	{ .model = 0x1f, .flags = ONLYMODEL, .str = "Core i7 (Nehalem)" },

	{ .model = 0x25, .flags = ONLYMODEL, .str = "Core i7 (Westmere)" },
	{ .model = 0x26, .flags = ONLYMODEL, .str = "Atom [Lincroft]" },
	{ .model = 0x27, .flags = ONLYMODEL, .str = "Atom [Penwell]" },
	{ .model = 0x2a, .flags = ONLYMODEL, .str = "Core i7 (SandyBridge)" },
	{ .model = 0x2c, .flags = ONLYMODEL, .str = "Core i7 (Westmere-EP)" },
	{ .model = 0x2d, .flags = ONLYMODEL, .str = "Core i7 (Sandybridge-X) [Romely-EP]" },
	{ .model = 0x2e, .flags = ONLYMODEL, .str = "Core i7 (Nehalem-EX) [Beckton]" },
	{ .model = 0x2f, .flags = ONLYMODEL, .str = "Core i7 (Westmere-EX) [Xeon E7]" },

	{ .model = 0x35, .flags = ONLYMODEL, .str = "Atom [Cloverview]" },
	{ .model = 0x36, .flags = ONLYMODEL, .str = "Atom [Cedarview]" },
	{ .model = 0x37, .flags = ONLYMODEL, .str = "Atom [Silvermont]" },
	{ .model = 0x3a, .stepping = 9, .str = "Core i7 3520M [IvyBridge]" },
	{ .model = 0x3a, .flags = ONLYMODEL, .str = "Core i7 [IvyBridge]" },
	// 8M cache
	{ .model = 0x3c, .stepping = 3, .mhz = 3200, .str = "Xeon E3-1225 v3 [Haswell]" },
	{ .model = 0x3c, .stepping = 3, .mhz = 3500, .str = "Xeon E3-1270 [Haswell]" },
	{ .model = 0x3c, .stepping = 3, .mhz = 4000, .str = "4790K [Haswell: Devils Canyon]" },
	{ .model = 0x3c, .stepping = 3, .str = "Xeon E3 [Haswell]" },
	{ .model = 0x3d, .stepping = 4, .str = "Core i7-5600U [Broadwell]" },
	{ .model = 0x3d, .flags = ONLYMODEL, .str = "Core i7 [Broadwell])" },
	{ .model = 0x3e, .flags = ONLYMODEL, .str = "Core i7 [IvyBridge-X]" },
	{ .model = 0x3f, .stepping = 2, .str = "Core i7-5820K [Haswell-X]" },
	{ .model = 0x3f, .flags = ONLYMODEL, .str = "Core i7 [Haswell-X]" },

	{ .model = 0x45, .flags = ONLYMODEL, .str = "Core i7 (Haswell ULT)" },
	{ .model = 0x46, .flags = ONLYMODEL, .str = "Core i7 (Haswell +GT3e)" },
	{ .model = 0x47, .flags = ONLYMODEL, .str = "Core i7 (Broadwell +GT3e)" },
	{ .model = 0x4a, .flags = ONLYMODEL, .str = "Atom [Tangier]" },
	{ .model = 0x4c, .flags = ONLYMODEL, .str = "Atom [Airmont]" },
	{ .model = 0x4d, .flags = ONLYMODEL, .str = "Atom [Silvermont Avoton/Rangely]" },
	{ .model = 0x4e, .flags = ONLYMODEL, .str = "Core i7 [Skylake Mobile]" },
	{ .model = 0x4f, .flags = ONLYMODEL, .str = "Core i7 [Broadwell-X]" },

	{ .model = 0x55, .flags = ONLYMODEL, .str = "Core i7 (Skylake-X)" },
	{ .model = 0x56, .flags = ONLYMODEL, .str = "Core i7 (Broadwell Xeon-D)" },
	{ .model = 0x57, .flags = ONLYMODEL, .str = "Xeon Phi [Knights Landing]" },
	{ .model = 0x5a, .flags = ONLYMODEL, .str = "Atom E3500" },
	{ .model = 0x5c, .flags = ONLYMODEL, .str = "Atom [Goldmont/Apollo Lake]" },
	{ .model = 0x5d, .flags = ONLYMODEL, .str = "Atom X3-C3000 [Silvermont]" },
	{ .model = 0x5e, .flags = ONLYMODEL, .str = "Core-i7 [Skylake desktop]" },
	{ .model = 0x5f, .flags = ONLYMODEL, .str = "Atom [Denverton]" },

	{ .model = 0x66, .flags = ONLYMODEL, .str = "Atom [Cannonlake]" },
	{ .model = 0x6a, .flags = ONLYMODEL, .str = "[Icelake-X]" },
	{ .model = 0x6c, .flags = ONLYMODEL, .str = "[Icelake Xeon-D]" },

	{ .model = 0x75, .flags = ONLYMODEL, .str = "Atom [Airmont NP]" },
	{ .model = 0x7a, .flags = ONLYMODEL, .str = "Atom [Goldmont]" },
	{ .model = 0x7d, .flags = ONLYMODEL, .str = "[Icelake Desktop]" },
	{ .model = 0x7e, .flags = ONLYMODEL, .str = "[Icelake Mobile]" },

	{ .model = 0x85, .flags = ONLYMODEL, .str = "Xeon Phi [Knights Mill]" },
	{ .model = 0x86, .flags = ONLYMODEL, .str = "Atom [Tremont/Jacobsville]" },
	{ .model = 0x8a, .flags = ONLYMODEL, .str = "[Lakefield]" },
	{ .model = 0x8c, .flags = ONLYMODEL, .str = "[Tiger Lake L]" },
	{ .model = 0x8d, .flags = ONLYMODEL, .str = "[Tiger Lake]" },
	{ .model = 0x8e, .flags = ONLYMODEL, .str = "[Kabylake mobile]" },
	{ .model = 0x8f, .flags = ONLYMODEL, .str = "[Sapphire Rapids]" },

	{ .model = 0x96, .flags = ONLYMODEL, .str = "[Tremont]" },
	{ .model = 0x97, .flags = ONLYMODEL, .str = "[Alder Lake]" },
	{ .model = 0x9a, .flags = ONLYMODEL, .str = "[Alder Lake L]" },
	{ .model = 0x9c, .flags = ONLYMODEL, .str = "[Tremont L]" },
	{ .model = 0x9d, .flags = ONLYMODEL, .str = "[Icelake NNPI]" },
	{ .model = 0x9e, .flags = ONLYMODEL, .str = "[Kabylake desktop]" },

	{ .model = 0xa5, .flags = ONLYMODEL, .str = "[Comet Lake]" },
	{ .model = 0xa6, .flags = ONLYMODEL, .str = "[Comet Lake]" },
	{ .model = 0xa7, .flags = ONLYMODEL, .str = "[Rocket Lake]" },
	{ .model = 0xaa, .flags = ONLYMODEL, .str = "[Meteor Lake L]" },
	{ .model = 0xac, .flags = ONLYMODEL, .str = "[Meteor Lake]" },
	{ .model = 0xad, .flags = ONLYMODEL, .str = "[Granite Rapids X]" },
	{ .model = 0xae, .flags = ONLYMODEL, .str = "[Granite Rapids D]" },
	{ .model = 0xaf, .flags = ONLYMODEL, .str = "[Sierra Forest]" },

	{ .model = 0xb6, .flags = ONLYMODEL, .str = "[Grand Ridge]" },
	{ .model = 0xb7, .flags = ONLYMODEL, .str = "[Raptor Lake]" },
	{ .model = 0xba, .flags = ONLYMODEL, .str = "[Raptor Lake P]" },
	{ .model = 0xbd, .flags = ONLYMODEL, .str = "[Lunar Lake]" },
	{ .model = 0xbe, .flags = ONLYMODEL, .str = "[Alder Lake N]" },
	{ .model = 0xbf, .flags = ONLYMODEL, .str = "[Raptor Lake S]" },

	{ .model = 0xc5, .flags = ONLYMODEL, .str = "[Arrow Lake]" },
	{ .model = 0xcf, .flags = ONLYMODEL, .str = "[Emerald Rapids]" },
};


static const char unknown[] = "Unknown model.";

static const char * get_namestring(struct cpudata *cpu)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(intel_fam6e_names); i++) {
		if (model(cpu) != intel_fam6e_names[i].model)
			continue;

		if (intel_fam6e_names[i].cores &&
		    intel_fam6e_names[i].cores != cpu->nr_cores)
			continue;

		if (intel_fam6e_names[i].flags & ONLYMODEL)
			return intel_fam6e_names[i].str;

		if (intel_fam6e_names[i].mhz > 0) {
			ensure_MHz(cpu);
			if (!MHz_matches(cpu->MHz, intel_fam6e_names[i].mhz))
				continue;
		}

		if (intel_fam6e_names[i].stepping != cpu->stepping)
			continue;

		return intel_fam6e_names[i].str;

	}
	return unknown;
}

void identify_intel_family6core(struct cpudata *cpu)
{
	snprintf(cpu->name, CPU_NAME_LEN, "%s", get_namestring(cpu));
}
