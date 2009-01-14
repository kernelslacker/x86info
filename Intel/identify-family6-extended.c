/*
 *  (C) 2001,2008 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel family 6 specific decoding (Core family).
 *  All the CPUs described in this file have cpu->emodel set to 1
 */

#include <stdio.h>
#include <string.h>
#include "../x86info.h"
#include "Intel.h"

static char *intel_nameptr;
#define add_to_cpuname(x)   intel_nameptr += snprintf(intel_nameptr, sizeof(x), "%s", x);

void Identify_Intel_family6core(struct cpudata *cpu)
{
	intel_nameptr = cpu->name;

	switch (model(cpu)) {
	case 7:
		switch (cpu->stepping) {
		case 6:
			// sSpec step CoreFreq Bus cache
			// SLAN3  C0  3.00    1333  12MB (2x6) QX9650
			// SLANY  C0  3.2     1600  12MB (2x6) QX9775
/*
sSpec name  package step HFM/LFM/SLFM FSB  IDAT L2Cache
SLAQG T9300 m-FCPGA C-0  2.5/1.2/0.8   800  2.7 6
SLAPV T9300 m-FCBGA C-0  2.5/1.2/0.8   800  2.7 6
SLAPU T8300 m-FCBGA C-0  2.4/1.2/0.8   800  2.6 3
SLAUU T8100 m-FCPGA C-0  2.1/1.2/0.8   800  2.3 3
SLAPT T8100 m-FCBGA C-0  2.1/1.2/0.8   800  2.3 3
SLAPT T8100 m-FCBGA C-0  2.1/1.2/0.8   800  2.3 3
SLAPA T8300 m-FCPGA M-0  2.4/1.2/0.8   800  2.6 3
SLAPR T8300 m-FCBGA M-0  2.4/1.2/0.8   800  2.6 3
SLAP9 T8100 m-FCPGA M-0  2.1/1.2/0.8   800  2.3 3
SLAPS T8100 m-FCBGA M-0  2.1/1.2/0.8   800  2.3 3
SLAVJ T8100 m-FCPGA M-0  2.1/1.2/0.8   800  2.3 3
SLAXG T8100 m-FCPGA M-0  2.1/1.2/0.8   800  2.3 3
SLAZD T8100 m-FCPGA M-0  2.1/1.2/0.8   800  2.3 3
SLAYZ T8100 m-FCPGA M-0  2.1/1.2/0.8   800  2.3 3
SLAZC T8300 m-FCPGA M-0  2.4/1.2/0.8   800  2.6 3
SLAZB T9300 m-FCPGA C-0  2.5/1.2/0.8   800  2.7 6
SLAYY T9300 m-FCPGA C-0  2.5/1.2/0.8   800  2.7 6
SLAZA T9500 m-FCPGA C-0  2.6/1.2/0.8   800  2.8 6
SLAYX T9500 m-FCPGA C-0  2.6/1.2/0.8   800  2.8 6
SLAQJ X9000 m-FCPGA C-0  2.8/1.2/0.8   800  N/A 6
SLAZ3 X9000 m-FCPGA C-0  2.8/1.2/0.8   800  N/A 6
SLB47 T9600 m-FCPGA C-0  2.80/1.6/0.8 1066 2.93 6
SLB43 T9600 m-FCBGA C-0  2.80/1.6/0.8 1066 2.93 6
SLB46 T9400 m-FCPGA C-0  2.53/1.6/0.8 1066 2.66 6
SL3BX T9400 m-FCBGA C-0  2.53/1.6/0.8 1066 2.66 6
SLB4E P9500 m-FCPGA C-0  2.53/1.6/0.8 1066 2.66 6
SL3BW P9500 m-FCBGA C-0  2.53/1.6/0.8 1066 2.66 6
SLB3S P8600 m-FCPGA M-0  2.40/1.6/0.8 1066 2.53 3
SLB4N P8600 m-FCBGA M-0  2.40/1.6/0.8 1066 2.53 3
SLB3R P8400 m-FCPGA M-0  2.26/1.6/0.8 1066 2.40 3
SLB4M P8400 m-FCBGA M-0  2.26/1.6/0.8 1066 2.40 3
SLB3Q P8400 m-FCPGA M-0  2.26/1.6/0.8 1066 2.40 3
SL3BV P8600 m-FCBGA C-0  2.40/1.6/0.8 1066 2.53 3
SL3BU P8400 m-FCBGA C-0  2.26/1.6/0.8 1066 2.40 3
SLB48 X9100 m-FCPGA C-0  3.06/1.6/0.8 1066 N/A  6
SLAYS T8300 m-FCPGA M-0  2.4/1.2/0.8   800  2.6 3
SLAYU T8100 m-FCPGA M-0  2.1/1.2/0.8   800  2.3 3
SLAYP T8100 m-FCPGA M-0  2.1/1.2/0.8   800  2.3 3
SLAYQ T8300 m-FCPGA M-0  2.4/1.2/0.8   800  2.6 3
SLG8E P7350 m-FCBGA C-0  2.00/1.6/0.8 1066 N/A  3
SLB45 P7450 m-FCPGA C-0  2.13/1.6/0.8 1066 N/A  3
SLB44 P7350 m-FCPGA C-0  2.00/1.6/0.8 1066 N/A  3
SLB54 P7450 m-FCPGA M-0  2.13/1.6/0.8 1066 N/A  3
SLB53 P7350 m-FCPGA M-0  2.00/1.6/0.8 1066 N/A  3
SLB5J QX9300 m-FCPGA E-0 2.53/1.6/(n/a) 1066  2.8 45  12
SLB5G Q9100  m-FCPGA E-0 2.26/1.6/(n/a) 1066 2.53 45  12
SLB64 SP9400 m-FCBGA C-0 2.40/1.6/0.8 1066 2.53 25   6
SLB63 SP9300 m-FCBGA C-0 2.26/1.6/0.8 1066 2.40 25   6
SLB66 SL9400 m-FCBGA C-0 1.86/1.6/0.8 1066 2.13 17   6
SLB65 SL9300 m-FCBGA C-0 1.60/0.8/0.8 1066 1.86 17   6
SLB5V SU9400 m-FCBGA M-0 1.40/0.8/0.8  800 1.60 10  3
SLB5Q SU9300 m-FCBGA M-0 1.20/0.8/0.8  800 1.40 10   3
SLGAR SU3300 m-FCBGA M-0 1.20/0.8/(n/a) 800 N/A  5.5  3    (Celeron)
SLGAS   723  m-FCBGA M-0 1.20/(n/a)/(n/a) 800 N/A  10   1    (Celeron)

*/

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
		case 0xa:
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
	case 0xa:
		add_to_cpuname("Core i7 ");
		//sSpec step CoreFreq/QuickpathGTs/DDR3	cache
		//SLBCJ C-0	3.2/6.40/1066		8MB
		//SLBCK C-0	2.93/4.80/1066		8MB
		//SLBCH C-0	2.66/4.80/1066		8MB
		break;
	case 0xc:
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
			 * SLB6Q C0 0.65W Z500 400 106C2 0.8GHz  600Mhz FCBGA8
			 * SLB2C C0    2W Z510 400 106C2 1.1GHz  600Mhz FCBGA8
			 * SLB2H C0    2W Z520 533 106C2 1.33GHz 800Mhz FCBGA8
			 * SLB6P C0    2W Z530 533 106C2 1.60GHz 800Mhz FCBGA8
			 * QGZT  C0  2.5W N270 533 106C2 1.60GHz 800MHz FCBGA8
			 * QKGY1 C0    8W  300 533 106C2 1.60GHz ------ FCBGA 437
			 * QGZR2 C0    4W  230 533 106C2 1.60GHz -----  FCBGA437 M01106C2208
			 * SLB2M C0  2.4W Z540 533 106C2 1.86GHz 800Mhz FCBGA8
			 */
			switch (cpu->MHz) {
			case 800:	add_to_cpuname("Z500 [SLB6Q][C0]")
					break;
			case 1100:	add_to_cpuname("Z510 [SLB2C][C0]")
					break;
			case 1330:	add_to_cpuname("Z520 [SLB2H][C0]")
					break;
			case 1600:	// could be a Z530,an N270,a QKGY1 or a QGZR2
					break;
			case 1860:	add_to_cpuname("Z540 [SLB2M][C0]")
					break;
			}
			break;
		}
		break;
	case 0xe:
		add_to_cpuname("Core ");
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
		case 0xc:
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

	case 23:
		add_to_cpuname("Core 2 Duo ");
		switch (cpu->stepping) {
		case 4:	add_to_cpuname("(Penryn)");
			break;
		case 6:	add_to_cpuname("P8600");
			break;
		}
		break;

	case 26:
		/*
		 * SLBCJ C-0 0x000106A4 3.20 / 6.40/ 1066 8MB
		 * SLBCK C-0 0x000106A4 2.93 / 4.80/ 1066 8MB
		 * SLBCH C-0 0x000106A4 2.66 / 4.80/ 1066 8MB
		 */
		add_to_cpuname("Core i7 (Nehalem)");
		switch (cpu->MHz) {
		case 3200:	add_to_cpuname(" [C-0][SLBCJ]");
				break;
		case 2930:	add_to_cpuname(" [C-0][SLBCK]");
				break;
		case 2660:	add_to_cpuname(" [C-0][SLBCH]");
				break;
		}
		break;


	default:
                add_to_cpuname("Unknown model. ");
	}
}
