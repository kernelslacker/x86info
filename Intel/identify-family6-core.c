/*
 *  (C) 2001,2008 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel family 6 specific decoding (Core family).
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

	case 0xf:
		add_to_cpuname("Core 2 ");
		// Do a numerical hack, because they aren't exactly 2100Mhz etc.
		// FIXME: Come up with a better way to do this, easiest if
		// Intel gives us an Extreme chip to compare against others ;-)
		if(cpu->MHz/100 >= 29) {
			add_to_cpuname("Extreme ");
		} else {
			if (cpu->stepping == 7) {
				add_to_cpuname("Quad (Kentsfield)");
			} else {
				add_to_cpuname("Duo ");
			}
		}
		// Check for Thermal Monitor 2 feature bit, because only the
		// non-mobile processors have it
		// TODO: Clean up Feature bit handling
		if(cpu->flags_ecx & (1 << 8)) {
			cpu->connector = CONN_LGA775;
			switch (cpu->MHz) {
			case 1800:
				// SL9SA 1066FSB 2MB L2
				add_to_cpuname("E6300");
				break;
			case 2150:
				// SL9S9 1066FSB 2MB L2
				add_to_cpuname("E6400");
				break;
			case 2400:
				// SL9S8 1066FSB 4MB L2
				add_to_cpuname("E6600");
				break;
			case 2600:
				// SL9S7 1066FSB 4MB L2
				add_to_cpuname("E6700");
				break;
			case 2950:
				// SL9S5 1066FSB 4MB L2
				add_to_cpuname("E6800/X6800");
				break;
			}
		} else {
			cpu->connector = CONN_MICROFCBGA;
			add_to_cpuname("Mobile ");
			switch (cpu->MHz) {
			case 1600:
				// SL9SH/SL9SQ 1.6GHz 667FSB
				add_to_cpuname("T5500");
				break;
			case 1800:
				// SL9SG/SL9SP 1.8GHz 667FSB
				add_to_cpuname("T5600");
			case 2000:
				// SL9SF/SL9SL 667FSB
				add_to_cpuname("T7200");
				break;
			case 2100:
				// SL9SE/SL9SK 667FSB
				add_to_cpuname("T7400");
				break;
			case 2300:
				// SL9SD/SL9SJ 667FSB
				add_to_cpuname("T7600");
				break;
			}
		}
		// TODO: Check that the Mobile chips really are stepping 6 as well.
		// The Sept 06 Core 2 Intel Errata documentation says there are
		// at least B1 and B2 steppings.
		switch(cpu->stepping) {
		case 2:
			// 2M L2
			// SL9TB E4300 1.86GHz
			// SLA3F E4400 2GHz
			// SL9TA E6300 1.86GHz
			// SL9T9 E6400 2.13GHz
			add_to_cpuname(" [L2]");
			break;
		// TODO: B1 as stepping 5 is a 100% guess
		case 5:
			add_to_cpuname(" [B1]");
			break;
		case 6:
			// 2M L2
			// SL9SA E6300 1.86GHz
			// SL9S9 E6400 2.13GHz
			//
			// 4M L2
			// SLA4U E6320 1.86GHz
			// SLA4T E6420 2.13GHz
			// SL9S8 E6600 2.4GHz
			// SL9ZL E6700 2.66GHz
			// SL9ZF E6700 2.66GHz
			// SL9S5 X6800 2.93GHz
			add_to_cpuname(" [B2]");
			break;
		case 7:
			// SL9UK [B3] 2.93GHz  QX6800
			// SL9UL [B3] 2.66GHz  QX6700
			// SL9UM [B3] 2.4GHz   Q6600
			// SLACP [G0] 2.93GHz  QX6800
			break;
		case 0xa:
			// sCode Procname (IDA/HFM)
			// SLA43 T7700 (2.6GHz/2.4GHz)
			// SLA3M T7700 (2.6GHz/2.4GHz)
			// SLA44 T7500 (2.4GHz/2.2GHz)
			// SLA3N T7500 (2.4GHz/2.2GHz)
			// SLA45 T7300 (2.4GHz/2.0GHz)
			// SLA3P T7300 (2.2GHz/2.0GHz)
			// SLA3R L7500 (1.8GHz/1.6GHz)
			// SLA3S L7300 (1.6GHz/1.4GHz)
			// SLA33 X7900 (2.8GHz)
			// SLA6Z X7800 (2.6GHz)
			add_to_cpuname(" [E1]");
			break;
		case 0xb:
			// SLAA5 2.33GHz E6540
			// SLA9X 2.33GHz E6550
			// SLA9V 2.66GHz E6750
			// SLA9U 3GHz    E6850
			// SLAFN 3GHz    QX6850
			// SLACQ 2.66GHz Q6700
			// SLACR 2.4GHz  Q6600
			//
			// Xeon 7200/7300 series
			// SLA67 2.93GHz X7350 [8M L2]
			// SLA68 2.4GHz  E7340 [8M L2]
			// SLA77 2.4GHz  E7330 [8M L2]
			// SLA69 2.13GHz E7320 [4M L2]
			// SLA6A 1.6GHz  E7310 [4M L2]
			// SLA6B 1.86GHz L7345 [8M L2]
			// SLA6C 2.93GHz E7220 [8M L2]
			// SLA6D 2.4GHz  E7210 [8M L2]
			add_to_cpuname(" [G0]");
			break;
		case 0xd:
			// SLA98 E4400 2GHz
			// SLA95 E4500 2GHz
			add_to_cpuname(" [M0]");
			break;
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
	default:
                add_to_cpuname("Unknown model. ");
	}
}
