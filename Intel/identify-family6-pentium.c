/*
 *  (C) 2001,2008 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel family 6 specific decoding. (Pentium family)
 */

#include <stdio.h>
#include <string.h>
#include "../x86info.h"
#include "Intel.h"

static char *intel_nameptr;
#define add_to_cpuname(x)   intel_nameptr += snprintf(intel_nameptr, sizeof(x), "%s", x);

void Identify_Intel_family6pentium(struct cpudata *cpu)
{
	intel_nameptr = cpu->name;

	switch (model(cpu)) {
	case 0x0:
		add_to_cpuname("Pentium Pro A-Step");
		cpu->connector = CONN_SOCKET_8;
		break;
	case 0x1:
		add_to_cpuname("Pentium Pro");
		cpu->connector = CONN_SOCKET_8;
		switch (cpu->stepping) {
		case 1:
			add_to_cpuname(" [B0]");
			switch (cpu->MHz) {
				case 133:
					//sSpec# Q0812, Q0815
					break;
				case 150:
					//sSpec# Q0813, Q0816, SY002, SY011, SY014
					break;
			}
			break;
		case 2:
			add_to_cpuname(" [C0]");
			//sSpec# Q0822, Q0825, Q0826, SY010
			break;
		case 6:
			add_to_cpuname(" [sA0]");
			switch (cpu->MHz) {
				case 166:
					//sSpec# Q0864
					break;
				case 180:
					//sSpec# SY012, Q0858, Q0860, Q0873, Q0910
					break;
				case 200:
					//cache = 256 sSpec# SY013, Q0859, Q0874
					//cache = 512 sSpec# Q0865
					break;
			}
			break;
		case 7:
			add_to_cpuname(" [sA1]");
			switch (cpu->MHz) {
				case 166:
					//sSpec# SY034, SY047, Q0918, Q0929, Q935
					break;
				case 180:
					//sSpec# SY031, SY039, SU103, Q0871, Q0907
					break;
				case 200:
					//cache = 256 sSpec# SY032, SY040, SL245, SL247, SU104, Q076, Q0872, Q0908, Q0909
					//cache = 512 sSpec# SY048, Q0920, Q0924, Q932 , Q936
					break;
			}
			break;
		case 9:
			add_to_cpuname(" [sB1]");
			switch (cpu->MHz) {
				case 166:
					//sSpec# Q008, Q009, SL2FJ, SL22X
					break;
				case 180:
					//sSpec# SL22S, SL22U, SL23L, Q033, Q035
					break;
				case 200:
					//cache = 256 sSpec# L22T, SL22V, SL23M,SL254,SL255,Q034,Q036 ,Q083 ,Q084
					//cache = 512 sSpec# Q010, Q011, SL22Z
					//cache = 1MB sSpec# SL259, SL25A
					break;
			}
			break;
		}
		break;
	case 0x3:
		add_to_cpuname("Pentium II ");
		cpu->connector = CONN_SLOT_1;
		switch (cpu->stepping) {
		case 2:
			add_to_cpuname("Overdrive [tdB0]");
			break;
		case 3:
			add_to_cpuname("(Klamath) [C0]");
			switch (cpu->MHz) {
				case 233:
					//sSpec# SL264, SL268, SL28K
					break;
				case 266:
					//sSpec# SL265, SL269, SL28L
					break;
				case 300:
					//sSpec# SL28R, SL2MZ
					break;
			}
			break;
		case 4:
			add_to_cpuname("(Klamath) [C1]");
			switch (cpu->MHz) {
				case 233:
					//sSpec# SL2HD, SL2HF, SL2QA
					break;
				case 266:
					//sSpec# SL2HC, SL2HE, SL2QB
					break;
				case 300:
					//sSpec# SL2HA, SL2QC
					break;
			}
			break;
		}
		break;
	case 0x4:
		//Does this exist? Its not in Intels spec update.
		cpu->connector = CONN_SLOT_1;
		add_to_cpuname("Pentium II (Deschutes?)");
		break;
	case 0x5:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->cachesize_L2) {
			case 0:
				add_to_cpuname("Celeron (Covington)");
				break;

			case 256:
				add_to_cpuname("Mobile Pentium II (Dixon)");
				break;

			case 512:
				switch (cpu->stepping) {
				case 0:
					add_to_cpuname("Pentium II [dA0]");
					switch (cpu->MHz) {
						case 266:
							//sSpec# SL2K9
							break;
						case 333:
							//sSpec# SL2KA, SL2QF
							break;
					}
					break;
				case 1:
					add_to_cpuname("Pentium II (Deschutes) [dA1]");
					switch (cpu->MHz) {
						case 300:
							//66 bus sSpec# SL35V, SL2VY
							break;
						case 333:
							//66 bus sSpec# SL2QH, SL2S5, SL2ZP
							break;
						case 350:
							//100Bus - sSpec# SL2ZQ, SL2S6, SL2SF
							break;
						case 400:
							//100Bus - sSpec# Sl2S7, SL2SH
							break;
					}
					break;
				case 2:
					add_to_cpuname("Pentium II (Deschutes) [dB0]");
					switch (cpu->MHz) {
						case 266:
							//66Bus sSpec# SL33D, SL2W7
							break;
						case 300:
							//66Bus - SL2YK, SL2W8
							break;
						case 333:
							//66Bus - SL2KE, SL2TV
							break;
						case 350:
							//100Bus - SL2WZ, SL2U3, SL2U4, SL356, SL37F, SL3FN
							break;
						case 400:
							//100Bus - SL2YM, SL37G, SL2U5, SL2U6, SL357, SL3EE, SL3F9
							break;
						case 450:
							//100Bus - SL2WB, SL37H, SL2U7, SL358
							break;
					}
					break;
				case 3:
					add_to_cpuname("Pentium II (Deschutes) [dB1]");
					switch (cpu->MHz) {
						case 350:
							//100Bus - SL38M, SL36U, SL3J2
							break;
						case 400:
							//100Bus - SL38N, SL38Z, SL3D5
							break;
					}
					break;
				default:
					add_to_cpuname("Pentium II");
					break;
				}
		}
		break;
	case 0x6:
		cpu->connector = CONN_SOCKET_370;
		if (cpu->cachesize_L2 == 128) {
			add_to_cpuname("Celeron (Mendocino)");
			break;
		}
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname("Celeron-A [mA0]");
			break;
		case 5:
			add_to_cpuname("Celeron-A [mB0]");
			break;
		case 0xA:
			add_to_cpuname("Mobile Pentium II [mdA0]");
			break;
		default:
			add_to_cpuname("Celeron / Mobile Pentium II");
			break;
		}
		break;
	case 0x7:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->stepping) {
		case 2:
			// Core=500 FSB=100
			// SL2XU SL3C9 (l2=512)
			// SL2XV SL3CA (l2=1MB)
			// SL2XW SL3CB (l2=2MB)
			add_to_cpuname("Pentium III (Katmai) [kB0]");
			break;
		case 3:
			// Core=550 FSB=100
			// SL3FK SL3D9 SL3AJ SL3Y4 SL3FR SL3LM (l2=512)
			// SL3DA SL3CE SL3TW SL3LN (l2=1mb)
			// SL3DB SL3CF SL3LP (l2=2mb)
			//
			// Core 500 FSB=100
			// SL385 (l2=512)
			// SL386 (l2=1MB)
			// SL387 (l2=2MB)
			add_to_cpuname("Pentium III (Katmai) [kC0]");
			break;
		default:
			add_to_cpuname("Pentium III/Pentium III Xeon");
			break;
		}
		decode_serial_number(cpu);
		break;
	case 0x8:
		switch (cpu->brand) {
		case 2:
			add_to_cpuname("Pentium III/Pentium III-M (Coppermine)");
			switch (cpu->stepping) {
			case 1:
				add_to_cpuname(" [cA2]");
				break;
			case 3:
				add_to_cpuname(" [cB0]");
				break;
			case 6:
				add_to_cpuname(" [cC0]");
				break;
			case 0xA:
				add_to_cpuname(" [cD0]");
				break;
			}
			break;
		case 3:
			add_to_cpuname("Pentium III Xeon");
			switch (cpu->stepping) {
			case 1:
				// l2=256KB FSB=133
				// Core=600		SL3BJ SL3BK SL3SS
				// Core=667		SL3BL SL3DC SL3ST
				// Core=733		SL3SF SL3SG SL3SU
				// Core=800		SL3V2 SL3V3 SL3VU
				add_to_cpuname(" [A2]");
				break;
			case 3:
				// l2=256 FSB=133
				// Core=600		SL3WM SL3WN
				// Core=667		SL3WP SL3WQ
				// Core=733		SL3WR SL3WS
				// Core=800		SL3WT SL3WU
				// Core=866		SL3WV SL3WW SL4PZ
				// Core=933		SL3WX SL3WY
				add_to_cpuname(" [B0]");
				break;
			case 6:
				// l2=256 FSB=133
				// Core=733		SL4H6 SL4H7
				// Core=800		SL4H8 SL4H9
				// Core=866		SL4HA SL4HB SL4U2
				// Core=933		SL4HC SL4HD SL4R9
				// Core=1000	SL4HE SL4HF
				add_to_cpuname(" [C0]");
				break;
			}
			break;

		case 8:
			// cpu->connector = CONN_BGA2; - Could also be Micro-PGA2
			add_to_cpuname("Mobile Pentium III");
			break;

		default:
			cpu->connector = CONN_SOCKET_370_FCPGA;
			if (cpu->cachesize_L2 == 128) {
				add_to_cpuname("Celeron");
			} else {
				add_to_cpuname("Pentium III");
			}
			add_to_cpuname(" (Coppermine)");
			switch (cpu->stepping) {
			case 1:
				add_to_cpuname(" [cA2]");
				break;
			case 3:
				add_to_cpuname(" [cB0]");
				break;
			case 6:
				add_to_cpuname(" [cC0]");
				break;
			case 0xA:
				add_to_cpuname(" [cD0]");
				break;
			}
			break;
		}
		decode_serial_number(cpu);
		break;

	case 0x9:
//		cpu->connector = 
		add_to_cpuname("Pentium M (Banias)");
		break;

	case 0xa:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->brand) {
		case 0:
			add_to_cpuname("Pentium II (Deschutes)");
			break;
		case 1:
			add_to_cpuname("Celeron");
			break;
		case 2:
			add_to_cpuname("Pentium III");
			decode_serial_number(cpu);
			break;
		case 3:
			// FSB=100
			add_to_cpuname("Pentium III Xeon");
			decode_serial_number(cpu);
			switch (cpu->stepping) {
				case 0:
					// Core=700
					//    L2=1MB		SL3U4 SL3U5 SL4GD SL4GE
					//    L2=2MB		SL3WZ SL3X2 SL4GF SL4GG
					add_to_cpuname(" [A0]");
					break;
				case 1:
					// Core=700
					//    L2=1MB		SL49P SL49Q SL4RZ
					//    L2=2MB		SL49R SL49S SL4R3
					add_to_cpuname(" [A1]");
					break;
				case 4:
					// Core=700
					//    L2=1MB		SL4XU SL5D4 SL4XV
					//    L2=2MB		SL4XW SL5D5 SL4XX
					// Core=900
					//    L2=2MB		SL4XY SL4XZ SL5D3
					add_to_cpuname(" [B0]");
					break;
			}
			break;
		case 4:
			add_to_cpuname("Pentium III (Cascades)");
			decode_serial_number(cpu);
			break;
		default:
			add_to_cpuname("Unknown CPU");
			break;
		}
		break;

	case 0xb:
		switch (cpu->brand) {
			case 1:
			case 3:
				cpu->connector = CONN_SLOT_1;
				add_to_cpuname("Celeron (Tualatin) [tA1/cA2]");
				break;
			case 6:
				cpu->connector = CONN_MICROFCBGA;
				add_to_cpuname("Pentium III-M");
				decode_serial_number(cpu);
				break;
			default:
				cpu->connector = CONN_SLOT_1;
				switch (cpu->stepping) {
				case 1:
					add_to_cpuname("Pentium III (Tualatin) [tA1/cA2]");
					break;
				case 4:
					add_to_cpuname("Pentium III [B-1]");
					break;
				default:
					add_to_cpuname("Unknown CPU");
					break;
				}
				decode_serial_number(cpu);
				break;
		}
		break;

	case 0xd:
		add_to_cpuname("Pentium M ");
		cpu->connector = CONN_MICROFCBGA;
		switch (cpu->stepping) {
			case 1:	add_to_cpuname("(Dothan) [A-1]");
					break;

			case 2:	add_to_cpuname("(Dothan) [A-2]");
					break;

			/*
				S-spec	Processor	Hi-Freq	Low-Freq
						Number
				SL7EM	755			2.0GHz	600MHz
				SL7EL	755			2.0GHz	600MHz
				SL7EN	745			1.8GHz	600MHz
				SL7EQ	745			1.8GHz	600MHz
				SL7EP	735			1.7GHz	600MHz
				SL7ER	735			1.7GHz	600MHz
				SL7EG	725			1.6GHz	600MHz
				SL7F2	725			1.6GHz	600MHz
				SL7GL	715			1.5GHz	600MHz
				SL7GK	715			1.5GHz	600MHz
				SL7VC	738			1.4GHz	600MHz
				SL7F4	733			1.1GHz	600MHz
				SL7VD	733			1.1GHz	600MHz
				SL7V2	723			1.0GHz	600MHz
			 */
			case 6:	add_to_cpuname("(Dothan) [B-1]");
					break;

			/*
			   FSB=533 2MB L2 90nm
				SL86G	730			1.6GHz	800MHz
				SL7SA	740			1.73GHz	800MHz
				SL7S9	750			1.86GHz	800MHz
				SL7SM	760			2.0GHz	800MHz
				SL7SL	770			2.13GHz	800MHz
				SL7VB	780			2.26GHz	800MHz
				SL86M	730			1.6GHz	800MHz
				SL7S8	740			1.73GHz	800MHz
				SL7SR	750			1.86GHz	800MHz
				Sl7SQ	760			2.0GHz	800MHz
				SL7SP	770			2.13GHz	800MHz
				SL7SN	780			2.26GHz	800MHz
				SL86B	740			1.73GHz	800MHz
				SL86A	750			1.86GHz	800MHz
				SL869	760			2.0GHz	800MHz
				SL868	770			2.13GHz	800MHz
				SL8QK	780			2.26GHz	800MHz
			   FSB=400
				SL8QF	778			1.6GHz	600MHz
				SL89X	758			1.5GHz	600MHz
				SL8A3	723			1.0GHz	600MHz
				SL8LM	733J		1.1GHz	600MHz
				SL8A2	733J		1.1GHz	600MHz
				SL89Z	753			1.2GHz	600MHz
				SL8LL	753			1.2GHz	600MHz
				SL8QG	778			1.6GHz	600MHz
				SL89M	758			1.5GHz	600MHz
				SL89R	723			1.0GHz	600MHz
				SL8LT	733J		1.1GHz	600MHz
				SL89Q	733J		1.1GHz	600MHz
				SL89P	753			1.2GHz	600MHz
				SL8LS	753			1.2GHz	600MHz
				SL89N	738			1.4GHz	600MHz
				SL89Y	738			1.4GHz	600MHz
		     */
			case 8:	add_to_cpuname("(Dothan) [C-0]");
					break;
		}
		break;
	/*
	 * ARGH. Intel made some Core CPUs without setting the efamily or emodel to 1.
	 */
	case 0xf:
		add_to_cpuname("Core 2 ");
		switch (cpu->stepping) {
		/*
		 * 2M Level2 cache
		 * SL9TB L2 E4300 1.8GHz/800Mhz
		 * SLA3F L2 E4400 2GHz/800MHz
		 * SL9TA L2 E6300 1.86GHz/1066MHz
		 * SL9T9 L2 E6400 2.13GHz/1066MHz
		 */
		case 2:	add_to_cpuname("Duo [L2]");
			break;
		case 5:
			add_to_cpuname("Duo ");
			break;
		/*
		 * 2MB L2 cache
		 * - SL9SA B2 E6300 1.86GHz/1066MHz
		 * - SL9S9 B2 E6400 2.13GHz/1066MHz
		 * 4MB L2 cache
		 * - SLA4U B2 E6320 1.86GHz/1066MHz
		 * - SLA4T B2 E6420 2.13GHz/1066MHz
		 * - SL9S8 B2 E6600 2.4GHz/1066MHz
		 * - SL9ZL B2 E6600 2.4GHz/1066MHz
		 * - SL9S7 B2 E6700 2.66GHz/1066MHz
		 * - SL9ZF B2 E6700 2.66GHz/1066MHz
		 * - SLAA5 G0 E6540 2.33GHz/1.333MHz
		 * - SLA9X G0 E6550 2.33GHz/1.333MHz
		 * - SLA9V G0 E6750 2.66GHz/1.333MHz
		 * - SLA9U G0 E6850 3GHz/1.333MHz
		 * - SL9S5 B2 X6800 2.93GHz/1066MHz
		 */
		case 6:
			add_to_cpuname("Duo ");
			switch (cpu->cachesize_L2) {
			case 2048:
				add_to_cpuname("[B2]");
				if (cpu->MHz/100 == 1860)
					add_to_cpuname("[SL9SA] E6300");
				if (cpu->MHz/100 == 2130)
					add_to_cpuname("[SL9S9] E6400");
				break;
			case 4096:
				break;
			}
			break;
		/*
		 * All quad-core.
		 * SL9UK B3 8MB QX6800 2.93GHz/1066MHz
		 * SL9UL B3 8MB QX6700 2.66GHz/1066MHz
		 * SL9UM B3 8MB QX6600 2.4GHz/1066MHz
		 * SLACP G0 8MB QX6800 2.93GHz/1066MHz
		 */
		case 7:
			add_to_cpuname("Quad (Kentsfield)");
			break;
		/* sCode Procname (IDA/HFM)
		 * SLA43 T7700 (2.6GHz/2.4GHz)
		 * SLA3M T7700 (2.6GHz/2.4GHz)
		 * SLA44 T7500 (2.4GHz/2.2GHz)
		 * SLA3N T7500 (2.4GHz/2.2GHz)
		 * SLA45 T7300 (2.4GHz/2.0GHz)
		 * SLA3P T7300 (2.2GHz/2.0GHz)
		 * SLA3R L7500 (1.8GHz/1.6GHz)
		 * SLA3S L7300 (1.6GHz/1.4GHz)
		 * SLA33 X7900 (2.8GHz)
		 * SLA6Z X7800 (2.6GHz)
		 */
		case 0xa:
			add_to_cpuname(" [E1]");
			break;
		/*
		 * SLALT G0 2M E4700 2.6GHz/800MHz
		 *
		 * SLAFN G0 8M QX6850 3GHz/1333MHz
		 * SLACQ G0 8M Q6700 2.66GHz/1066MHz
		 * SLACR G0 8M Q6600 2.4GHz/1066MHz
		 */
		case 0xb:
			if (cpu->cachesize_L2 == 2048) {
				add_to_cpuname("Duo [G0]");
			} else
				add_to_cpuname("Quad ");
			break;
		/*
		 * SLA98 M0 2M E4400 2GHz/800MHz
		 * SLA95 M0 2M E4500 2.2GHz/800MHz
		 * SLA94 M0 2M E4600 2.4GHz/800MHz
		 */
		case 0xd:
			add_to_cpuname("Duo [M0]");
			break;
		}
		break;

	default:
                add_to_cpuname("Unknown model. ");
	}
}
