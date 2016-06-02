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

static char nehalem_info_url[] = "http://www.intel.com/design/corei7/documentation.htm";

static char *intel_nameptr;
#define add_to_cpuname(x)   intel_nameptr += snprintf(intel_nameptr, sizeof(x), "%s", x)

void identify_intel_family6core(struct cpudata *cpu)
{
	intel_nameptr = cpu->name;

	switch (model(cpu)) {
	case 7:
		switch (cpu->stepping) {
		case 6:
			// sSpec step CoreFreq Bus cache
			// SLAN3  C0  3.00    1333  12MB (2x6) QX9650
			// SLANY  C0  3.2     1600  12MB (2x6) QX9775

			add_to_cpuname("Core 2 quad ");
			switch (cpu->MHz) {
			case 3000:	add_to_cpuname("Core 2 quad (QX9650) ");
				break;
			case 3200:	add_to_cpuname("Core 2 Extreme quad (QX9775) ");
				break;
			}
			add_to_cpuname("[C0] ");
			break;
		case 7:
			// sSpec step CoreFreq Bus cache
			// SLAWM C1   3.2    1600  12MB (2x6)	QX9770
			// SLAWQ C1   2.83   1600  12MB (2x6)	Q9550
			// SLAWR C1   2.66   1333  12MB (2x6)	Q9450
			// SLAWE M1   2.50   1333  6MB (2x3)	Q9300
			// SLB5M M1   2.33   1333  4MB (2x2)    Q8200
			add_to_cpuname("Core 2 quad ");
			switch (cpu->MHz) {
			case 3200:	add_to_cpuname("(QX9770) [C1] ");
					break;
			case 2830:	add_to_cpuname("(Q9550) [C1] ");
					break;
			case 2660:	add_to_cpuname("(Q9450) [C1] ");
					break;
			case 2500:	add_to_cpuname("(Q9300) [M1] ");
					break;
			case 2330:	add_to_cpuname("(Q8200) [M1] ");
					break;
			}
			break;
		case 10:
			// sSpec step CoreFreq Bus cache
			// SLB8W E0  3.00     1333 12MB (2x6) Q9650
			// SLB8V E0  2.83     1333 12MB (2x6) Q9550
			// SLB6B R0  2.66     1333 6MB  (2x3) Q9400
			// SLB5W R0  2.5      1333 4MB  (2x2) Q8300
			add_to_cpuname("Core 2 quad ");
			switch (cpu->MHz) {
			case 3000:	add_to_cpuname("(Q9650) [E0] ");
					break;
			case 2830:	add_to_cpuname("(Q9550) [E0] ");
					break;
			case 2660:	add_to_cpuname("(Q9400) [R0] ");
					break;
			case 2500:	add_to_cpuname("(Q8300) [R0] ");
					break;
			}
			break;
		}
		break;
	case 10:
		add_to_cpuname("Core i7 ");
		//sSpec step CoreFreq/QuickpathGTs/DDR3	cache
		//SLBCJ C-0	3.2/6.40/1066		8MB
		//SLBCK C-0	2.93/4.80/1066		8MB
		//SLBCH C-0	2.66/4.80/1066		8MB
		break;
	case 12:
		add_to_cpuname("Atom ");
		switch (cpu->stepping) {
		case 1:
			/*
			 * sSpec step TDP Name FSB EFMS  HFM    LFM    Package  MCU
			 * QDTD  B0   2.5   x  533 106C1 1.6GHz 800MHz FCBGA8 M01106C1109
			 * QDTB  B0   2.5   x  533 106C1 1.6GHz 800MHz FCBGA8 M01106C1109
			 * QGFD1 B0     X   X  533 106C1 1.33GHz ----  FCBGA437 M01106C1109
			 */
			break;
		case 2:
			/*
			 * sSpec step TDP Name FSB EFMS  HFM     LFM    Package
			 *
			 *
			 * SLB6Q C0 0.65W Z500 400 106C2 800GHz  600MHz FCBGA8
			 * SLB2C C0    2W Z510 400 106C2 1.1GHz  600MHz FCBGA8
			 * SLGMG C0 0.65W Z515 400 106C2 800MHz  600MHz FCBGA8
			 * SLB2H C0    2W Z520 533 106C2 1.33GHz 800MHz FCBGA8
			 * SLB6P C0    2W Z530 533 106C2 1.60GHz 800MHz FCBGA8
			 * QGZT  C0  2.5W N270 533 106C2 1.60GHz 800MHz FCBGA8
			 * QKGY1 C0    8W  300 533 106C2 1.60GHz ------ FCBGA437
			 * QGZR2 C0    4W  230 533 106C2 1.60GHz -----  FCBGA437
			 * SLB2M C0  2.4W Z540 533 106C2 1.86GHz 800MHz FCBGA8
			 * SLGPT C0  2.4W Z550 533 106C2 2.0GHz  800MHz FCBGA8
			 */
			switch (cpu->MHz) {
			case 800:	add_to_cpuname("Z500/Z515 [SLB6Q/SLGMG][C0]");
					break;
			case 1100:	add_to_cpuname("Z510 [SLB2C][C0]");
					break;
			case 1330:	add_to_cpuname("Z520 [SLB2H][C0]");
					break;
			case 1600:	// could be a Z530,an N270,a QKGY1 or a QGZR2
					break;
			case 1860:	add_to_cpuname("Z540 [SLB2M][C0]");
					break;
			case 2000:	add_to_cpuname("Z550 [SLGPT][C0]");
					break;
			}
			break;
		}
		break;
	case 0xe:
		add_to_cpuname("Core (Yonah)");
		switch (cpu->stepping) {
		case 8:
			switch (cpu->MHz) {
			case 1000:
				// SL99W/SL8W7 533FSB
				add_to_cpuname("Duo U2400/Solo U1300 [C-0]");
				break;
			case 1200:
				// SL8W6 533FSB
				add_to_cpuname("Solo U1400 [C-0]");
				break;
			case 1500:
				// SL8VX 667FSB
				add_to_cpuname("Duo L2300 [C-0]");
				break;
			case 1600:
				// SL9JE/SL9JV/SL8VR/SL8VV/SL8VY/SL8W3/SL8VW 667FSB
				add_to_cpuname("Solo T1300/Duo T2300(E)/Duo L2400");
				break;
			case 1800:
				// SL92X/SL8VQ/SL8VU/SL92V/SL92X 667FSB
				add_to_cpuname("Solo T1400/Duo T2400 [C-0]");
				break;
			case 2000:
				// SL8VP/SL8VT/SL92U/SL92W 667FSB
				add_to_cpuname("Solo T1500/Duo T2500 [C-0]");
				break;
			case 2150:
				// SL8VN/SL8VS 667FSB
				add_to_cpuname("Duo T2600 [C-0]");
				break;
			}
			break;
		case 12:
			switch (cpu->MHz) {
			case 1200:
				// SL99V 533FSB
				add_to_cpuname("Duo U2500 [D-0]");
				break;
			case 1800:
				// SL9JU 667FSB
				add_to_cpuname("Duo L2500 [D-0]");
				break;
			case 2300:
				// SL9JP/SL9K4 667FSB
				add_to_cpuname("Duo T2700 [D-0]");
				break;
			}
		}
		break;

	case 0xf:
		add_to_cpuname("Core 2 (Merom) ");
		break;

	case 0x16:
		add_to_cpuname("Core 2 (Merom-L) ");
		break;

	case 0x17:
		add_to_cpuname("Core 2 Duo ");
		switch (cpu->stepping) {
		case 4:	add_to_cpuname("(Penryn)");
			break;
		case 6:	add_to_cpuname("P8600");
			break;
		}
		break;

	case 0x1a:
		/*
		 * SLBCJ C-0 0x000106A4 3.20 / 6.40/ 1066 8MB
		 * SLBCK C-0 0x000106A4 2.93 / 4.80/ 1066 8MB
		 * SLBCH C-0 0x000106A4 2.66 / 4.80/ 1066 8MB
		 */
		add_to_cpuname("Core i7 (Nehalem-EP) [bloomfield/gainestown]");
		cpu->info_url = strdup(nehalem_info_url);
		switch (cpu->MHz) {
		case 3200:	add_to_cpuname(" [C-0][SLBCJ]");
				break;
		case 2930:	add_to_cpuname(" [C-0][SLBCK]");
				break;
		case 2660:	add_to_cpuname(" [C-0][SLBCH]");
				break;
		}
		break;

	case 0x1c:
		add_to_cpuname("Atom");
		switch (cpu->stepping) {
		case 10:	add_to_cpuname(" D510 [Pineview]");
				break;
		}
		break;

	case 0x1d:
		add_to_cpuname("[Dunnington (MP)]");
		break;

	case 0x1e:
		add_to_cpuname("Core i7 (Nehalem)");
		break;

	case 0x25:
		add_to_cpuname("Core i7 (Westmere)");
		break;

	case 0x26:
		add_to_cpuname("Atom [Lincroft]");
		break;

	case 0x27:
		add_to_cpuname("Atom [Penwell]");
		break;

	case 0x2a:
		add_to_cpuname("Core i7 (SandyBridge)");
		break;

	case 0x2c:
		add_to_cpuname("Core i7 (Westmere-EP)");
		break;

	case 0x2d:
		add_to_cpuname("Core i7 (Sandybridge-X) [Romely-EP]");
		break;

	case 0x2e:
		add_to_cpuname("Core i7 (Nehalem-EX) [Beckton]");
		break;

	case 0x2f:
		add_to_cpuname("Core i7 (Westmere-EX) [Xeon E7]");
		break;

	case 0x35:
		add_to_cpuname("Atom [Cloverview]");
		break;

	case 0x36:
		add_to_cpuname("Atom [Cedarview]");
		break;

	case 0x37:
		add_to_cpuname("Atom [Silvermont]");
		break;

	case 0x3a:
		if (cpu->stepping == 9)
			add_to_cpuname("Core i7 3520M [IvyBridge]");
		break;
	case 0x3c:
		if (cpu->stepping == 3) {
			switch (cpu->MHz) {
			case 3200:
				// 3.2Ghz 8M cache
				add_to_cpuname("Xeon E3-1225 v3 (Haswell)");
				break;
			case 3500:
				// 3.5Ghz 8M cache
				add_to_cpuname("Xeon E3-1270 (Haswell)");
				break;
			}
			break;
		} else
			add_to_cpuname("Core i7 (Haswell)");
		break;
	case 0x3d:
		if (cpu->stepping == 4)
			add_to_cpuname("Core i7-5600U (Broadwell)");
		else
			add_to_cpuname("Core i7 (Broadwell)");
		break;

	case 0x3e:
		add_to_cpuname("Core i7 (IvyBridge-X)");
		break;

	case 0x3f:
		add_to_cpuname("Core i7 (Haswell-X)");
		if (cpu->stepping == 2)
			add_to_cpuname(" [i7-5820K]");
		break;

	case 0x45:
		add_to_cpuname("Core i7 (Haswell ULT)");
		break;

	case 0x46:
		add_to_cpuname("Core i7 (Haswell +GT3e)");
		break;

	case 0x47:
		add_to_cpuname("Core i7 (Broadwell +GT3e)");
		break;

	case 0x4a:
		add_to_cpuname("Atom [Tangier]");
		break;

	case 0x4c:
		add_to_cpuname("Atom [Airmont]");
		break;

	case 0x4d:
		add_to_cpuname("Atom [Silvermont Avoton/Rangely]");
		break;

	case 0x4e:
		add_to_cpuname("Core i7 [Skylake Mobile]");
		break;

	case 0x4f:
		add_to_cpuname("Core i7 [Broadwell-X]");
		break;

	case 0x55:
		add_to_cpuname("Core i7 (Skylake-X)");
		break;

	case 0x56:
		add_to_cpuname("Core i7 (Broadwell Xeon-D)");
		break;

	case 0x57:
		add_to_cpuname("Xeon Phi [Knights Landing]");
		break;

	case 0x5c:
		add_to_cpuname("Atom [Goldmont]");
		break;

	case 0x5e:
		add_to_cpuname("[Skylake desktop]");
		break;

	case 0x5f:
		add_to_cpuname("Atom [Denverton]");
		break;

	case 0x8e:
		add_to_cpuname("[Kabylake mobile]");
		break;

	case 0x9e:
		add_to_cpuname("[Kabylake desktop]");
		break;

	default:
                add_to_cpuname("Unknown model. ");
	}
}
