/*
 *  $Id: identify.c,v 1.39 2002/11/19 17:06:53 davej Exp $
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
			nameptr+=sprintf (nameptr, "%s", "[B0]");
			break;
		case 2:
			nameptr+=sprintf (nameptr, "%s", "[C0]");
			break;
		case 6:
			nameptr+=sprintf (nameptr, "%s", "[sA0]");
			break;
		case 7:
			nameptr+=sprintf (nameptr, "%s", "[sA1]");
			break;
		case 9:
			nameptr+=sprintf (nameptr, "%s", "[sB1]");
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
			break;
		case 4:
			nameptr+=sprintf (nameptr, "%s", "(Klamath) [C1]");
			break;
		}
		break;
	case 0x640:
		cpu->connector = CONN_SLOT_1;
		nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes?)");
		break;
	case 0x650:
		cpu->connector = CONN_SLOT_1;
		if (cpu->cachesize_L2 == 0) {
			nameptr+=sprintf (cpu->name, "%s", "Celeron (Covington)");
			break;
		}
		if (cpu->cachesize_L2 == 256) {
			nameptr+=sprintf (cpu->name, "%s", "Mobile Pentium II (Dixon)");
			break;
		}
		switch (cpu->stepping) {
		case 0:
			nameptr+=sprintf (cpu->name, "%s", "Pentium II [dA0]");
			break;
		case 1:
			nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes) [dA1]");
			break;
		case 2:
			nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes) [dB0]");
			break;
		case 3:
			nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes) [dB1]");
			break;
		default:
			nameptr+=sprintf (cpu->name, "%s", "Pentium II");
			break;
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
			nameptr+=sprintf (cpu->name, "%s", "Pentium III [kB0]");
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
		cpu->connector = CONN_SLOT_1;
		switch (cpu->stepping) {
		case 1:
			nameptr+=sprintf (cpu->name, "%s", "Celeron / Pentium III (Tualatin) [tA1/cA2]");
			break;
		case 4:
			nameptr+=sprintf (cpu->name, "%s", "Celeron / Pentium III [B-1]");
			break;
		default:
			nameptr+=sprintf (cpu->name, "%s", "Unknown CPU");
			break;
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
		nameptr += sprintf (cpu->name, "%s", "Pentium 4 (Northwood)");
		switch (cpu->stepping) {
		case 4:
			nameptr+=sprintf (nameptr, "%s", " [B0]");
			break;
		case 7:
			nameptr+=sprintf (nameptr, "%s", " [C1]");
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
