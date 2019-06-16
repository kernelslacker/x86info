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
#define ONLYMHZ 2

struct cpu_name_def {
	unsigned int model;
	unsigned int stepping;
	unsigned int mhz;
	unsigned int flags;
	const char *str;
};

static struct cpu_name_def intel_fam6e_names[] = {
	// sSpec step CoreFreq Bus cache
	// SLAN3  C0  3.00    1333  12MB (2x6) QX9650
	// SLANY  C0  3.2     1600  12MB (2x6) QX9775
	{ .model = 7, .stepping = 6, .mhz = 3000, .str = "Core 2 quad (QX9650) [C0]" },
	{ .model = 7, .stepping = 6, .mhz = 3200, .str = "Core 2 extreme quad (QX9775) [C0]" },

	// sSpec step CoreFreq Bus cache
	// SLAWM C1   3.2    1600  12MB (2x6)	QX9770
	// SLAWQ C1   2.83   1600  12MB (2x6)	Q9550
	// SLAWR C1   2.66   1333  12MB (2x6)	Q9450
	// SLAWE M1   2.50   1333  6MB (2x3)	Q9300
	// SLB5M M1   2.33   1333  4MB (2x2)    Q8200
	{ .model = 7, .stepping = 7, .mhz = 3200, .str = "Core 2 quad (QX9770) [C1]" },
	{ .model = 7, .stepping = 7, .mhz = 2830, .str = "Core 2 quad (Q9550) [C1]" },
	{ .model = 7, .stepping = 7, .mhz = 2660, .str = "Core 2 quad (Q9450) [C1]" },
	{ .model = 7, .stepping = 7, .mhz = 2500, .str = "Core 2 quad (Q9300) [M1]" },
	{ .model = 7, .stepping = 7, .mhz = 2330, .str = "Core 2 quad (Q8200) [M1]" },

	// sSpec step CoreFreq Bus cache
	// SLB8W E0  3.00     1333 12MB (2x6) Q9650
	// SLB8V E0  2.83     1333 12MB (2x6) Q9550
	// SLB6B R0  2.66     1333 6MB  (2x3) Q9400
	// SLB5W R0  2.5      1333 4MB  (2x2) Q8300
	{ .model = 7, .stepping = 10, .mhz = 3000, .str = "Core 2 quad (Q9650) [E0]" },
	{ .model = 7, .stepping = 10, .mhz = 2830, .str = "Core 2 quad (Q9550) [E0]" },
	{ .model = 7, .stepping = 10, .mhz = 2660, .str = "Core 2 quad (Q9400) [R0]" },
	{ .model = 7, .stepping = 10, .mhz = 2500, .str = "Core 2 quad (Q8300) [R0]" },

	//sSpec step CoreFreq/QuickpathGTs/DDR3	cache
	//SLBCJ C-0	3.2/6.40/1066		8MB
	//SLBCK C-0	2.93/4.80/1066		8MB
	//SLBCH C-0	2.66/4.80/1066		8MB
	{ .model = 0xa, .flags = ONLYMODEL, .str = "Core i7", },

	// sSpec step TDP Name FSB EFMS  HFM    LFM    Package  MCU
	// QDTD  B0   2.5   x  533 106C1 1.6GHz 800MHz FCBGA8 M01106C1109
	// QDTB  B0   2.5   x  533 106C1 1.6GHz 800MHz FCBGA8 M01106C1109
	// QGFD1 B0     X   X  533 106C1 1.33GHz ----  FCBGA437 M01106C1109
	{ .model = 0xc, .stepping = 1, .str = "Atom" },

	// sSpec step TDP Name FSB EFMS  HFM     LFM    Package
	// SLB6Q C0 0.65W Z500 400 106C2 800GHz  600MHz FCBGA8
	// SLB2C C0    2W Z510 400 106C2 1.1GHz  600MHz FCBGA8
	// SLGMG C0 0.65W Z515 400 106C2 800MHz  600MHz FCBGA8
	// SLB2H C0    2W Z520 533 106C2 1.33GHz 800MHz FCBGA8
	// SLB6P C0    2W Z530 533 106C2 1.60GHz 800MHz FCBGA8
	// QGZT  C0  2.5W N270 533 106C2 1.60GHz 800MHz FCBGA8
	// QKGY1 C0    8W  300 533 106C2 1.60GHz ------ FCBGA437
	// QGZR2 C0    4W  230 533 106C2 1.60GHz -----  FCBGA437
	// SLB2M C0  2.4W Z540 533 106C2 1.86GHz 800MHz FCBGA8
	// SLGPT C0  2.4W Z550 533 106C2 2.0GHz  800MHz FCBGA8
	{ .model = 0xc, .stepping = 2, .mhz = 800, .str = "Atom Z500/Z515 [C0]" },
	{ .model = 0xc, .stepping = 2, .mhz = 1100, .str = "Atom Z510 [C0]" },
	{ .model = 0xc, .stepping = 2, .mhz = 1330, .str = "Atom Z520 [C0]" },
	{ .model = 0xc, .stepping = 2, .mhz = 1600, .str = "Atom Z530/N270/QKGY1/QGZR2" },
	{ .model = 0xc, .stepping = 2, .mhz = 1860, .str = "Atom Z540 [C0]" },
	{ .model = 0xc, .stepping = 2, .mhz = 2000, .str = "Atom Z550 [C0]" },

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

	{ .model = 0xf, .flags = ONLYMODEL, .str = "Core 2 [Merom]" },

	{ .model = 0x16, .flags = ONLYMODEL, .str = "Core 2 [Merom-L]" },

	{ .model = 0x17, .stepping = 4, .str = "Core 2 Duo [Penryn]" },
	{ .model = 0x17, .stepping = 6, .str = "Core 2 Duo P8600 [Penryn]" },

	// SLBCJ C-0 0x000106A4 3.20 / 6.40/ 1066 8MB
	// SLBCK C-0 0x000106A4 2.93 / 4.80/ 1066 8MB
	// SLBCH C-0 0x000106A4 2.66 / 4.80/ 1066 8MB
	{ .model = 0x1a, .stepping = 4, .mhz = 3200, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [C-0] [SLBCJ]" },
	{ .model = 0x1a, .stepping = 4, .mhz = 2930, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [C-0] [SLBCK]" },
	{ .model = 0x1a, .stepping = 4, .mhz = 2660, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [C-0] [SLBCH]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 3330, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBEQ]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 3200, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBEU]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 3060, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBEN]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 2660, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBEJ]" },
	{ .model = 0x1a, .stepping = 5, .mhz = 2800, .str = "Core i7 [Nehalem-EP] [bloomfield/gainestown] [D-0] [SLBKP]" },

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
	{ .model = 0x7a, .flags = ONLYMODEL, .str = "Atom [Gemini Lake]" },
	{ .model = 0x7d, .flags = ONLYMODEL, .str = "[Icelake Desktop]" },
	{ .model = 0x7e, .flags = ONLYMODEL, .str = "[Icelake Mobile]" },

	{ .model = 0x85, .flags = ONLYMODEL, .str = "Xeon Phi [Knights Mill]" },
	{ .model = 0x86, .flags = ONLYMODEL, .str = "Atom [Tremont/Jacobsville]" },

	{ .model = 0x8e, .flags = ONLYMODEL, .str = "[Kabylake mobile]" },
	{ .model = 0x9e, .flags = ONLYMODEL, .str = "[Kabylake desktop]" },
};


static const char unknown[] = "Unknown model.";

static const char * get_namestring(struct cpudata *cpu)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(intel_fam6e_names); i++) {
		if (model(cpu) != intel_fam6e_names[i].model)
			continue;

		if (intel_fam6e_names[i].flags & ONLYMODEL)
			return intel_fam6e_names[i].str;

		if (intel_fam6e_names[i].mhz > 0) {
			if (intel_fam6e_names[i].mhz != cpu->MHz)
				continue;
			if (intel_fam6e_names[i].flags & ONLYMHZ)
				return intel_fam6e_names[i].str;
		}

		if (intel_fam6e_names[i].stepping != cpu->stepping)
			continue;

		return intel_fam6e_names[i].str;

	}
	return unknown;
}

void identify_intel_family6core(struct cpudata *cpu)
{
	//TODO: make cpu->name a ptr.
	strncpy(cpu->name, get_namestring(cpu), CPU_NAME_LEN);
}
