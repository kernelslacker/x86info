/*
 *  (C) 2001 Dave Jones.
 *
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
#include "../x86info.h"
#include "Intel.h"

static char *intel_nameptr;
#define add_to_cpuname(x)   intel_nameptr += snprintf(intel_nameptr, sizeof(x), "%s", x);

void Identify_Intel(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
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

	cpu->emodel = (eax >> 16) & 0xf;
	cpu->efamily= (eax >> 20) & 0xff;

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
	case 0x610:
	case 0x630:
	case 0x640:
	case 0x650:
	case 0x660:
	case 0x670:
	case 0x680:
	case 0x690:
	case 0x6A0:
	case 0x6B0:
	case 0x6d0:
	case 0x6e0:
	case 0x6f0:
		Identify_Intel_family6(cpu);
		intel_nameptr += strlen(cpu->name);	// EWW
		break;

	case 0x700:		/* Family 7 */
		add_to_cpuname("Itanium");
		break;

	case 0xF00:
		Identify_Intel_family15(cpu);
		intel_nameptr += strlen(cpu->name);	// EWW
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
