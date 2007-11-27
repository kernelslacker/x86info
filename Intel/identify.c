/*
 *  $Id: identify.c,v 1.60 2004/08/20 13:05:56 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Intel specific parts
 *
 * References:
 *	http://developer.intel.com/
 *	http://microcodes.sourceforge.net/CPUID.htm
 *
 */

#include <stdio.h>
#include <string.h>
#include "../x86info.h"
#include "Intel.h"

static char *intel_nameptr;
#define add_to_cpuname(x)   intel_nameptr += snprintf(intel_nameptr, sizeof(x), "%s", x);

static char p4_423_datasheet[]="http://developer.intel.com/design/pentium4/datashts/24919805.pdf";
static char p4_478_datasheet[]="http://developer.intel.com/design/pentium4/datashts/24988703.pdf\n\thttp://developer.intel.com/design/pentium4/datashts/29864304.pdf";
static char p4_errata[]="http://developer.intel.com/design/pentium4/specupdt/249199.htm";


void Identify_Intel (struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;
	int reserved;

	cpu->vendor = VENDOR_INTEL;
	intel_nameptr = cpu->name;

	if (cpu->maxi < 1)
		return;

	/* Family/model/type etc */
	cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;
	cpu->type = (eax >> 12) & 0x3;
	cpu->brand = (ebx & 0xf);
	reserved = eax >> 14;

	cpu->flags_ecx = ecx; // Used for identification of Core 2
	cpu->flags_edx = edx;

	decode_Intel_caches(cpu, 0);

	switch (cpu->family) {
	case 4:	add_to_cpuname("i486 ");
			break;
	case 5: add_to_cpuname("Pentium ");
			break;
	}

	switch (tuple(cpu) & 0xff0) {
	case 0x400:		/* Family 4 */
		add_to_cpuname("DX-25/33");
		break;
	case 0x410:
		add_to_cpuname("DX-50");
		break;
	case 0x420:
		add_to_cpuname("SX");
		break;
	case 0x430:
		add_to_cpuname("487/DX2");
		break;
	case 0x440:
		add_to_cpuname("SL");
		break;
	case 0x450:
		add_to_cpuname("SX2");
		break;
	case 0x470:
		add_to_cpuname("write-back enhanced DX2");
		break;
	case 0x480:
		add_to_cpuname("DX4");
		cpu->connector = CONN_SOCKET_3;
		//transistors = 1600000;
		//fab_process = "0.6 micron CMOS";
		//die_size = "345 sq. mm";
		//introduction_date = "March 1994";
		break;
	case 0x490:
		add_to_cpuname("write-back enhanced DX4");
		cpu->connector = CONN_SOCKET_3;
		break;

	case 0x500:
		add_to_cpuname("A-step");
		cpu->connector = CONN_SOCKET_4;
		break;
	case 0x510:
		add_to_cpuname("60/66");
		cpu->connector = CONN_SOCKET_4;
		break;
	case 0x520:
		add_to_cpuname("75-200");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x530:
		add_to_cpuname("Overdrive");
		cpu->connector = CONN_SOCKET_4;
		break;
	case 0x540:
		add_to_cpuname("MMX");
		cpu->connector = CONN_SOCKET_7;
		//transistors = 4500000;
		//fab_process = "0.35 micron CMOS";
		//die_size = "140 sq.mm";
		//introduction_date = "June 1997";
		break;
	case 0x570:
		add_to_cpuname("Mobile");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x580:
		add_to_cpuname("MMX Mobile");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x600:
		add_to_cpuname("Pentium Pro A-Step");
		cpu->connector = CONN_SOCKET_8;
		break;
	case 0x610:
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
	case 0x630:
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
	case 0x640:
		//Does this exist? Its not in Intels spec update.
		cpu->connector = CONN_SLOT_1;
		add_to_cpuname("Pentium II (Deschutes?)");
		break;
	case 0x650:
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
	case 0x660:
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
	case 0x670:
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
		break;
	case 0x680:
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
		break;

	case 0x690:
//		cpu->connector = 
		add_to_cpuname("Pentium M (Banias)");
		break;

	case 0x6A0:
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
			break;
		case 3:
			// FSB=100
			add_to_cpuname("Pentium III Xeon");
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
			break;
		default:
			add_to_cpuname("Unknown CPU");
			break;
		}
		break;
	case 0x6B0:
		switch (cpu->brand) {
			case 1:
			case 3:
				cpu->connector = CONN_SLOT_1;
				add_to_cpuname("Celeron (Tualatin) [tA1/cA2]");
				break;
			case 6:
				cpu->connector = CONN_MICROFCBGA;
				add_to_cpuname("Pentium III-M");
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
		}
		break;
	case 0x6d0:
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

	case 0x6e0:
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

	case 0x6f0:
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

	case 0x700:		/* Family 7 */
		add_to_cpuname("Itanium");
		break;

	case 0xF00:	/* Family 15 */
		cpu->connector = CONN_SOCKET_423;
		cpu->datasheet_url = strdup(p4_423_datasheet);
		cpu->errata_url = strdup(p4_errata);
		add_to_cpuname("Pentium 4");
		switch (cpu->stepping) {
		case 7:
			//SL4QD SL4SF = 1.3GHz
			//SL4SG SL4SC = 1.4GHz
			//SL4SH SL4TY = 1.5GHz
			add_to_cpuname(" [B2]");
			break;
		case 0xA:
			//SL5FW SL5GC 1.3GHz
			//SL4WS SL4X2 SL59U SL5N7 1.4GHz
			//SL4WT SL4X3 SL59V SL5NB 1.5GHz
			//SL4WU SL4X4 SL5US SL5UW 1.6GHz
			//SL57W SL57V SL59X SL5N9 1.7GHz
			//SL4WV SL4X5 SL5UT SL5UV 1.8GHz
			add_to_cpuname(" [C1]");
			break;
		}
		break;

	case 0xF10:
		cpu->connector = CONN_SOCKET_423;
		add_to_cpuname("Pentium 4 (Willamette)");
		cpu->datasheet_url = strdup(p4_423_datasheet);
		cpu->errata_url = strdup(p4_errata);
		switch (cpu->stepping) {
		case 1:
			//400FSB 256K L2
			//SSpec MHz L3
			//SL5G8 1.6 1M
			//SL5S4 1.6 1M
			//SL5FZ 1.4 512K
			//SL5RZ 1.4 512K
			//SL5G2 1.5 512K
			//SL5RW 1.5 512K
			add_to_cpuname(" [C0]");
			break;
		case 2:
			//SL5TG SL5UE 1.4GHz
			//SL5SX SL5TJ SL5UF SL62Y SL5TN 1.5GHz
			//SL5VL SL5UL SL5VH SL5UJ 1.6GHz
			//SL5SY SL5TK SL5UG SL62Z 1.7GHz
			//SL5VM SL5VM SL5VJ SL5UK 1.8GHz
			//SL5VN SL5WH SL5VK SL5WG 1.9GHz
			//SL5SZ SL5TQ SL5TL 2GHz
			add_to_cpuname(" [D0]");
			break;
		case 3:
			//SL6BC SL679 1.6GHz
			//SL6BD SL67A 1.7GHz
			//SL6BE SL78B 1.8GHz
			//SL6BF SL67C 1.9GHz
			add_to_cpuname(" [E0]");
			break;
		}
		break;
	case 0xF20:
		cpu->connector = CONN_SOCKET_478;
		cpu->datasheet_url = strdup(p4_478_datasheet);
		cpu->errata_url = strdup(p4_errata);
		switch (cpu->brand) {
			case 15:
				add_to_cpuname("Celeron (P4 core)");
				break;
			case 7:
			default:
				add_to_cpuname("Pentium 4 (Northwood)");
				break;
		}
		switch (cpu->stepping) {
		case 2:
			//512K L2
			//          L3
			//SL6GZ 1.5 1M
			//SL6KB 1.5 1M
			//SL6H2 1.9 2M
			//SL6KC 1.9 2M
			//SL66Z 2.0 1M
			//SL6KD 2.0 1M
			add_to_cpuname(" [A0]");
			break;
		case 4:
			//SL66B 1.6GHz
			//SL63X SL62P SL6BQ 1.8GHz
			//SL6BR SL5YR 2GHz
			//SL5YS SL6BS SL5ZU 2.2GHz
			//SL6B3 SL67Y 2.26GHz (533MHz FSB)
			//SL6BT SL65R SL67R 2.4GHz (400MHz FSB)
			//SL6B4 SL67Z 2.4GHz (533MHz FSB)
			//SL6B5 SL6B2 2.53GHz (533MHz FSB)
			add_to_cpuname(" [B0]");
			break;
		case 5:
			/*[M0] */
			//SL6Z3 2.4GHz (800FSB)
			//SL6Z5 2.8GHz (800FSB)
			/* P4 Extreme edition.*/
			//SL7AA 3.2GHz (800FSB) 2MB L3 cache
			//SL7CH 3.4GHz (800FSB) 2MB L3 cache

			/* 400FSB B1 512K L2 */
			//SL6YJ 2.0 1M L3
			//SL6Z6 2.0 1M L3
			//SL6Z2 2.5 1M L3
			//SL6Z7 2.5 1M L3
			//SL6YL 2.8 2M L3
			//SL6Z8 2.8 2M L3
			add_to_cpuname(" [M0]");
			break;
		case 6:
			//400FSB 512K L2
			//SL79V 3.0 4M L3
			//SL79Z 2.7 2M L3
			//SL7A5 2.2 2M L3
			add_to_cpuname(" [C0]");
			break;
		case 7:
			//SL6HL SL6K6 2.8GHz (533MHz FSB)
			//SL6LA SL6S6 1.8GHz
			//SL6GQ SL6S7 SL6E7 2GHz
			//SL6GR SL6SB SL6EB 2.2GHz
			//SL6DU SL6RY SL6EE 2.26GHz (533FSB)
			//SL6EF SL6DV SL6S9 SL6RZ SL6E9 2.4GHz (533FSB)
			//SL6SA 2.5GHz (400FSB)
			//SL6EG SL6S2 SL6DW 2.53GHz (533FSB)
			//SL6SB 2.6GHz (400FSB)
			//SL6S3 SL6SK 2.66GHz (533FSB)
			//SL6S4 SL6SL 2.8GHz (533FSB)
			//SL6S5 SL6K7 SL6SM SL6JJ 3.06GHz (533FSB)
			add_to_cpuname(" [C1]");
			break;
		case 9:
			//SL6QL 1.8GHz
			//SL6QM SL6PK 2.0GHz
			//SL6QN SL6PL 2.2GHz
			//SL6QR SL6PB 2.26GHz (533FSB)
			//SL6QP SL6PM 2.4GHz
			//SL6QB SL6PC 2.4GHz (533FSB)
			//SL6WF SL6WR 2.4GHz (800FSB)
			//SL6QQ 2.5GHz
			//SL6Q9 SL6PD 2.53GHz (533FSB)
			//SL6QR 2.6GHz
			//SL6WH SL6WS 2.6GHz (800FSB)
			//SL6QA SL6PE 2.66GHz (533FSB)
			//SL6QB SL6PF 2.8GHz (533FSB)
			//SL6WJ SL6WT 2.8GHz (800FSB)
			//SL6WU SL6WK 3GHz (800FSB)
			//SL6QC SL6PG 3.06GHz (533FSB)
			//SL6WG SL6WE 3.2GHz (800FSB)
			//SL793 3.4GHz (800FSB)
			add_to_cpuname(" [D1]");
			break;
		}
		break;
	case 0xF30:
		switch (cpu->stepping) {
		case 3:
			/*
			sspec speed   fsb l2 90nm
			SL7D7 2.26GHz 533 512K
			SL7FY 2.4GHz  800 1M
			SL7E8 2.4GHz  533 1M
			SL7E9 2.66GHz 533 1M
			SL7D8 2.8GHz  533 1M
			SL79K 2.8GHz  800 1M
			SL79L 3.0GHz  800 1M
			SL79M 3.2GHz  800 1M
			SL7B8 3.2GHz  800 1M
			SL7B9 3.4GHz  800 1M
			SL7AJ 3.4GHz  800 1M

			process = "0.09u";
			125 million transistors
			112mm2 die size
			pipeline_stages=31
			*/
			add_to_cpuname("Pentium 4 (Prescott) [C0]");
			break;
		case 4:
			/*
			1M L2 90nm
			sspec speed   fsb
			SL7E2 2.8GHz  533
			SL7E3 2.8GHz  800
			SL7KA 2.8GHz  800
			SL7K9 2.8GHz  533
			SL7E4 3.0GHz  800
			SL7KB 3.0GHz  800
			SL7L4 3.0GHz  800
			SL7L5 3.2GHz  800
			SL7E5 3.2GHz  800
			SL7KC 3.2GHz  800
			SL7E6 3.4GHz  800
			SL7KD 3.4GHz  800
			SL7YP 2.4GHz  533
			SL7YU 2.66GHz 533
			SL7J4 2.8GHz  533
			SL7J5 2.8GHz  800
			SL7KH 2.8GHz  533
			SL7KJ 2.8GHz  800
			SL7YV 2.93GHz 533
			SL7J6 3.0GHz  800
			SL7KK 3.0GHz  800
			SL7J7 3.2GHz  800
			SL7KL 3.2GHz  800
			SL7LA 3.2GHz  800
			SL7J8 3.4GHz  800
			SL7KM 3.4GHz  800
			SL7L8 3.4GHz  800
			SL7J9 3.6GHz  800
			SL7KN 3.6GHz  800
			SL7L9 3.6GHz  800
			 */
			add_to_cpuname("Pentium 4 (Prescott) [D0]");
			break;
		}
		break;

	case 0xF40:
		add_to_cpuname("Pentium 4 ");
		switch (cpu->stepping) {
		case 1:
			/*
			 1M L2 90nm
			SL88F 2.4GHz  533
			SL8B3 2.66GHz 533
			SL88G 2.8GHz  533
			SL88H 2.8GHz  800
			SL7PL 2.8GHz  800
			SL7PK 2.8GHz  533
			SL7PM 3GHz    800
			SL88J 3GHz    800
			SL7PN 3.2GHz  800
			SL88K 3.2GHz  800
			SL88L 3.4GHz  800
			SL7PP 3.4GHz  800
			SL7PT 2.66GHz 533
			SL82P 2.8GHz  800
			SL7PR 2.8GHz  800
			SL8HX 2.8GHz  800
			SL85U 2.66GHz 533
			SL8J8 2.66GHz 533
			SL85V 2.93GHz 533
			SL8J9 2.93GHz 533
			SL87L 3.06GHz 533
			SL8JA 3.06GHz 533
			SL82X 3.0GHz  800
			SL7PU 3.0GHz  800
			SL8HZ 3.0GHz  800
			SL7PW 3.2GHz  800
			SL7PX 3.2GHz  800
			SL82Z 3.2GHz  800
			SL8J2 3.2GHz  800
			SL7PY 3.4GHz  800
			SL7PZ 3.4GHz  800
			SL833 3.4GHz  800
			SL7ZW 3.4GHz  800
			SL8J5 3.4GHz  800
			SL84X 3.6GHz  800
			SL7Q2 3.6GHz  800
			SL7NZ 3.6GHz  800
			SL8J6 3.6GHz  800
			SL82U 3.8GHz  800
			SL84Y 3.8GHz  800
			SL7P2 3.8GHz  800
			SL8J7 3.8GHz  800
			 */
			/*
			 8MB L3 [C-0]
			 SL8EY 3.3GHz 667
			 SL8EW 3GHz   667
			 4MB L3
			 SL8ED 2.8GHz 667
			 */
			add_to_cpuname("(Prescott) [E0]");
			break;
		case 3:
			/*
			 2M L2 90nm
			 SL7Z9 3.0GHz  800
			 SL7Z8 3.2GHz  800
			 SL8Z7 3.4GHz  800
			 SL7Z5 3.6GHz  800
			 SL7Z4 3.73GHz 800
			 SL7Z3 3.8GHz  800
			 */
			add_to_cpuname("(Prescott) [N0]");
			break;
		case 4:
			/*
			   1Mx2 L2	800MHz FSB
			   SL88T	2.8GHz
			   SL88S	3GHz
			   SL88R	3.2GHz
			   SL8FK	3.2GHz
			 */
			add_to_cpuname("Extreme Edition [A0]");
			break;
		default:
			add_to_cpuname("D (Foster)");
			break;
		}
		break;

	case 0xF50:
		cpu->connector = CONN_SOCKET_603;
//		cpu->datasheet_url = strdup(p4_478_datasheet);
//		cpu->errata_url = strdup(p4_errata);
		add_to_cpuname("Pentium 4 Xeon (Foster)");
		break;
	default:
		add_to_cpuname("Unknown CPU");
		break;
	}

	switch (cpu->type) {
	case 0:
		add_to_cpuname(" Original OEM");
		break;
	case 1:
		add_to_cpuname(" Overdrive");
		break;
	case 2:
		add_to_cpuname(" Dual-capable");
		break;
	case 3:
		add_to_cpuname(" Reserved");
		break;
	}
}
