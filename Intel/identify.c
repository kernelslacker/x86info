/*
 *  $Id: identify.c,v 1.52 2003/05/03 01:07:15 davej Exp $
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

static char p4_423_datasheet[]="http://developer.intel.com/design/pentium4/datashts/24919805.pdf";
static char p4_478_datasheet[]="http://developer.intel.com/design/pentium4/datashts/24988703.pdf\n\thttp://developer.intel.com/design/pentium4/datashts/29864304.pdf";
static char p4_errata[]="http://developer.intel.com/design/pentium4/specupdt/24919928.pdf";


void Identify_Intel (struct cpudata *cpu)
{
	char *nameptr;
	unsigned long eax, ebx, ecx, edx;
	int reserved;

	cpu->vendor = VENDOR_INTEL;
	nameptr = cpu->name;

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

	decode_Intel_caches(cpu);

	switch (cpu->family) {
	case 4:	nameptr += sprintf (cpu->name, "%s", "i486 ");
			break;
	case 5: nameptr += sprintf (cpu->name, "%s", "Pentium ");
			break;
	}

	switch (tuple(cpu) & 0xff0) {
	case 0x400:		/* Family 4 */
		nameptr+=sprintf (nameptr, "%s", "DX-25/33");
		break;
	case 0x410:
		nameptr+=sprintf (nameptr, "%s", "DX-50");
		break;
	case 0x420:
		nameptr+=sprintf (nameptr, "%s", "SX");
		break;
	case 0x430:
		nameptr+=sprintf (nameptr, "%s", "487/DX2");
		break;
	case 0x440:
		nameptr+=sprintf (nameptr, "%s", "SL");
		break;
	case 0x450:
		nameptr+=sprintf (nameptr, "%s", "SX2");
		break;
	case 0x470:
		nameptr+=sprintf (nameptr, "%s", "write-back enhanced DX2");
		break;
	case 0x480:
		nameptr+=sprintf (nameptr, "%s", "DX4");
		cpu->connector = CONN_SOCKET_3;
		//transistors = 1600000;
		//fab_process = "0.6 micron CMOS";
		//die_size = "345 sq. mm";
		//introduction_date = "March 1994";
		break;
	case 0x490:
		nameptr+=sprintf (nameptr, "%s", "write-back enhanced DX4");
		cpu->connector = CONN_SOCKET_3;
		break;

	case 0x500:
		nameptr+=sprintf (nameptr, "%s", "A-step");
		cpu->connector = CONN_SOCKET_4;
		break;
	case 0x510:
		nameptr+=sprintf (nameptr, "%s", "60/66");
		cpu->connector = CONN_SOCKET_4;
		break;
	case 0x520:
		nameptr+=sprintf (nameptr, "%s", "75-200");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x530:
		nameptr+=sprintf (nameptr, "%s", "Overdrive");
		cpu->connector = CONN_SOCKET_4;
		break;
	case 0x540:
		nameptr+=sprintf (nameptr, "%s", "MMX");
		cpu->connector = CONN_SOCKET_7;
		//transistors = 4500000;
		//fab_process = "0.35 micron CMOS";
		//die_size = "140 sq.mm";
		//introduction_date = "June 1997";
		break;
	case 0x570:
		nameptr+=sprintf (nameptr, "%s", "Mobile");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x580:
		nameptr+=sprintf (nameptr, "%s", "MMX Mobile");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x600:
		nameptr+=sprintf (cpu->name, "%s", "Pentium Pro A-Step");
		cpu->connector = CONN_SOCKET_8;
		break;
	case 0x610:
		nameptr+=sprintf (cpu->name, "%s", "Pentium Pro");
		cpu->connector = CONN_SOCKET_8;
		switch (cpu->stepping) {
		case 1:
			nameptr+=sprintf (nameptr, "%s", " [B0]");
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
			nameptr+=sprintf (nameptr, "%s", " [C0]");
			//sSpec# Q0822, Q0825, Q0826, SY010
			break;
		case 6:
			nameptr+=sprintf (nameptr, "%s", " [sA0]");
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
			nameptr+=sprintf (nameptr, "%s", " [sA1]");
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
			nameptr+=sprintf (nameptr, "%s", " [sB1]");
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
		nameptr+=sprintf (cpu->name, "%s", "Pentium II ");
		cpu->connector = CONN_SLOT_1;
		switch (cpu->stepping) {
		case 2:
			nameptr+=sprintf (nameptr, "%s", "Overdrive [tdB0]");
			break;
		case 3:
			nameptr+=sprintf (nameptr, "%s", "(Klamath) [C0]");
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
			nameptr+=sprintf (nameptr, "%s", "(Klamath) [C1]");
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
		nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes?)");
		break;
	case 0x650:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->cachesize_L2) {
			case 0:
				nameptr+=sprintf (cpu->name, "%s", "Celeron (Covington)");
				break;

			case 256:
				nameptr+=sprintf (cpu->name, "%s", "Mobile Pentium II (Dixon)");
				break;

			case 512:
				switch (cpu->stepping) {
				case 0:
					nameptr+=sprintf (cpu->name, "%s", "Pentium II [dA0]");
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
					nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes) [dA1]");
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
					nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes) [dB0]");
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
					nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes) [dB1]");
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
					nameptr+=sprintf (cpu->name, "%s", "Pentium II");
					break;
				}
		}
		break;
	case 0x660:
		cpu->connector = CONN_SOCKET_370;
		if (cpu->cachesize_L2 == 128) {
			nameptr+=sprintf (cpu->name, "%s", "Celeron (Mendocino)");
			break;
		}
		switch (cpu->stepping) {
		case 0:
			nameptr+=sprintf (cpu->name, "%s", "Celeron-A [mA0]");
			break;
		case 5:
			nameptr+=sprintf (cpu->name, "%s", "Celeron-A [mB0]");
			break;
		case 0xA:
			nameptr+=sprintf (cpu->name, "%s", "Mobile Pentium II [mdA0]");
			break;
		default:
			nameptr+=sprintf (cpu->name, "%s", "Celeron / Mobile Pentium II");
			break;
		}
		break;
	case 0x670:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->stepping) {
		case 2:
			nameptr+=sprintf (cpu->name, "%s", "Pentium III (Katmai) [kB0]");
			break;
		case 3:
			nameptr+=sprintf (cpu->name, "%s", "Pentium III (Katmai) [kC0]");
			break;
		default:
			nameptr+=sprintf (cpu->name, "%s", "Pentium III/Pentium III Xeon");
			break;
		}
		break;
	case 0x680:
		switch (cpu->brand) {
		case 2:
			nameptr+= sprintf (cpu->name, "%s", "Pentium III-M (Coppermine)");
			switch (cpu->stepping) {
			case 1:
				nameptr+=sprintf (nameptr, "%s", " [cA2]");
				break;
			case 3:
				nameptr+=sprintf (nameptr, "%s", " [cB0]");
				break;
			case 6:
				nameptr+=sprintf (nameptr, "%s", " [cC0]");
				break;
			case 0xA:
				nameptr+=sprintf (nameptr, "%s", " [cD0]");
				break;
			}
			break;

		case 8:
			// cpu->connector = CONN_BGA2; - Could also be Micro-PGA2
			nameptr+= sprintf (cpu->name, "%s", "Mobile Pentium III");
			break;

		default:
			cpu->connector = CONN_SOCKET_370_FCPGA;
			if (cpu->cachesize_L2 == 128) {
				nameptr+=sprintf (cpu->name, "%s", "Celeron");
			} else {
				nameptr+=sprintf (cpu->name, "%s", "Pentium III");
			}
			nameptr+=sprintf (nameptr, "%s", " (Coppermine)");
			switch (cpu->stepping) {
			case 1:
				nameptr+=sprintf (nameptr, "%s", " [cA2]");
				break;
			case 3:
				nameptr+=sprintf (nameptr, "%s", " [cB0]");
				break;
			case 6:
				nameptr+=sprintf (nameptr, "%s", " [cC0]");
				break;
			case 0xA:
				nameptr+=sprintf (nameptr, "%s", " [cD0]");
				break;
			}
			break;
		}
		break;

	case 0x690:
//		cpu->connector = 
		nameptr += sprintf (cpu->name, "%s", "Pentium 4-M (Centrino)");
		break;

	case 0x6A0:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->brand) {
		case 0:
			nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes)");
			break;
		case 1:
			nameptr+=sprintf (cpu->name, "%s", "Celeron");
			break;
		case 2:
			nameptr+=sprintf (cpu->name, "%s", "Pentium III");
			break;
		case 3:
			nameptr+=sprintf (cpu->name, "%s", "Pentium III Xeon");
			break;
		case 4:
			nameptr+=sprintf (cpu->name, "%s", "Pentium III (Cascades)");
			break;
		default:
			nameptr+=sprintf (cpu->name, "%s", "Unknown CPU");
			break;
		}
		break;
	case 0x6B0:
		switch (cpu->brand) {
			case 1:
			case 3:
				cpu->connector = CONN_SLOT_1;
				nameptr += sprintf (cpu->name, "%s", "Celeron (Tualatin) [tA1/cA2]");
				break;
			case 6:
				cpu->connector = CONN_MICROFCBGA;
				nameptr += sprintf (cpu->name, "%s", "Pentium III-M");
				break;
			default:
				cpu->connector = CONN_SLOT_1;
				switch (cpu->stepping) {
				case 1:
					nameptr+=sprintf (cpu->name, "%s", "Pentium III (Tualatin) [tA1/cA2]");
					break;
				case 4:
					nameptr+=sprintf (cpu->name, "%s", "Pentium III [B-1]");
					break;
				default:
					nameptr+=sprintf (cpu->name, "%s", "Unknown CPU");
					break;
				}
		}
		break;

	case 0x700:		/* Family 7 */
		nameptr += sprintf (cpu->name, "%s", "Itanium");
		break;

	case 0xF00:	/* Family 15 */
		cpu->connector = CONN_SOCKET_423;
		cpu->datasheet_url = strdup (p4_423_datasheet);
		cpu->errata_url = strdup (p4_errata);
		nameptr += sprintf (cpu->name, "%s", "Pentium 4");
		switch (cpu->stepping) {
		case 7:
			nameptr+=sprintf (nameptr, "%s", " [B2]");
			break;
		case 0xA:
			nameptr+=sprintf (nameptr, "%s", " [C1]");
			break;
		}
		break;

	case 0xF10:
		cpu->connector = CONN_SOCKET_423;
		nameptr += sprintf (cpu->name, "%s", "Pentium 4 (Willamette)");
		cpu->datasheet_url = strdup (p4_423_datasheet);
		cpu->errata_url = strdup (p4_errata);
		switch (cpu->stepping) {
		case 1:
			nameptr+=sprintf (nameptr, "%s", " [C0]");
			break;
		case 2:
			nameptr+=sprintf (nameptr, "%s", " [D0]");
			break;
		case 3:
			nameptr+=sprintf (nameptr, "%s", " [E0]");
			break;
		}
		break;
	case 0xF20:
		cpu->connector = CONN_SOCKET_478;
		cpu->datasheet_url = strdup (p4_478_datasheet);
		cpu->errata_url = strdup (p4_errata);
		switch (cpu->brand) {
			case 15:
				nameptr += sprintf (cpu->name, "%s", "Celeron (P4 core)");
				break;
			case 7:
			default:
				nameptr += sprintf (cpu->name, "%s", "Pentium 4 (Northwood)");
				break;
		}
		switch (cpu->stepping) {
		case 4:
			nameptr+=sprintf (nameptr, "%s", " [B0]");
			break;
		case 7:
			nameptr+=sprintf (nameptr, "%s", " [C1]");
			break;
		case 9:
			nameptr+=sprintf (nameptr, "%s", " [D1]");
			break;
		}
		break;
	case 0xF40:
	case 0xF50:
		cpu->connector = CONN_SOCKET_603;
//		cpu->datasheet_url = strdup (p4_478_datasheet);
//		cpu->errata_url = strdup (p4_errata);
		nameptr+=sprintf (cpu->name, "%s", "Pentium 4 Xeon (Foster)");
		break;
	default:
		nameptr+=sprintf (cpu->name, "%s", "Unknown CPU");
		break;
	}

	switch (cpu->type) {
	case 0:
		sprintf (nameptr, "%s", " Original OEM");
		break;
	case 1:
		sprintf (nameptr, "%s", " Overdrive");
		break;
	case 2:
		sprintf (nameptr, "%s", " Dual-capable");
		break;
	case 3:
		sprintf (nameptr, "%s", " Reserved");
		break;
	}
}
