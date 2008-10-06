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

	intel_nameptr = cpu->name;

	cpuid(cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	cpu->emodel = (eax >> 16) & 0xf;
	cpu->efamily= (eax >> 20) & 0xff;
	cpu->type = (eax >> 12) & 0x3;

	cpu->brand = ebx & 0xf;
	cpu->apicid = ebx >> 24;
	cpu->nr_logical = (ebx >> 16) & 0xff;

	cpu->flags_ecx = ecx; // Used for identification of Core 2
	cpu->flags_edx = edx;

	/* Figure out number of cores on this package. */
	cpu->nr_cores = 1;
	if (cpu->maxi >= 4) {
		cpuid4(cpu->number, 0, &eax, &ebx, &ecx, &edx);
		if (eax & 0x1f)
			cpu->nr_cores = ((eax >> 26) + 1);
	}

	decode_Intel_caches(cpu, 0);

	switch (cpu->family) {
	case 4:	add_to_cpuname("i486 ");
		switch (cpu->model) {
		case 0:
			add_to_cpuname("DX-25/33");
			break;
		case 1:
			add_to_cpuname("DX-50");
			break;
		case 2:
			add_to_cpuname("SX");
			break;
		case 3:
			add_to_cpuname("487/DX2");
			break;
		case 4:
			add_to_cpuname("SL");
			break;
		case 5:
			add_to_cpuname("SX2");
			break;
		case 7:
			add_to_cpuname("write-back enhanced DX2");
			break;
		case 8:
			add_to_cpuname("DX4");
			cpu->connector = CONN_SOCKET_3;
			//transistors = 1600000;
			//fab_process = "0.6 micron CMOS";
			//die_size = "345 sq. mm";
			//introduction_date = "March 1994";
			break;
		case 9:
			add_to_cpuname("write-back enhanced DX4");
			cpu->connector = CONN_SOCKET_3;
			break;
		}
		break;

	case 5: add_to_cpuname("Pentium ");
		switch (cpu->model) {
		case 0:
			add_to_cpuname("A-step");
			cpu->connector = CONN_SOCKET_4;
			break;
		case 1:
			add_to_cpuname("60/66");
			cpu->connector = CONN_SOCKET_4;
			break;
		case 2:
			add_to_cpuname("75-200");
			cpu->connector = CONN_SOCKET_5_7;
			break;
		case 3:
			add_to_cpuname("Overdrive");
			cpu->connector = CONN_SOCKET_4;
			break;
		case 4:
			add_to_cpuname("MMX");
			cpu->connector = CONN_SOCKET_7;
			//transistors = 4500000;
			//fab_process = "0.35 micron CMOS";
			//die_size = "140 sq.mm";
			//introduction_date = "June 1997";
			break;
		case 7:
			add_to_cpuname("Mobile");
			cpu->connector = CONN_SOCKET_7;
			break;
		case 8:
			add_to_cpuname("MMX Mobile");
			cpu->connector = CONN_SOCKET_7;
			break;
		}
		break;

	case 0x6:
		if (cpu->emodel == 0)
			Identify_Intel_family6pentium(cpu);
		else
			Identify_Intel_family6core(cpu);

		intel_nameptr += strlen(cpu->name);	// EWW
		break;

	case 0x7:
		add_to_cpuname("Itanium");
		break;

	case 0xF:
		Identify_Intel_family15(cpu);
		intel_nameptr += strlen(cpu->name);	// EWW
		break;
	}
}
