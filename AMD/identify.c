/*
 *  $Id: identify.c,v 1.49 2003/09/12 17:10:41 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific information
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "AMD.h"

static char *amd_nameptr, *namebegin;
#define add_to_cpuname(x)	amd_nameptr += sprintf(amd_nameptr, "%s", x);

static void do_assoc(unsigned long assoc)
{
	if ((assoc & 0xff) == 255)
		printf("Fully");
	else
		printf("%ld-way", assoc);
	printf(" associative. ");
}

static void decode_AMD_cacheinfo(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	if (cpu->maxei >= 0x80000005) {
		/* TLB and cache info */
		cpuid(cpu->number, 0x80000005, &eax, &ebx, &ecx, &edx);

		printf("Instruction TLB: ");
		do_assoc((ebx >> 8) & 0xff);
		printf("%ld entries.\n", ebx & 0xff);

		printf("Data TLB: ");
		do_assoc(ebx >> 24);
		printf("%ld entries.\n", (ebx >> 16) & 0xff);

		printf("L1 Data cache:\n\t");
		printf("Size: %ldKb\t", ecx >> 24);
		do_assoc((ecx >> 16) & 0xff);
		printf("\n\t");
		printf("lines per tag=%ld\t", (ecx >> 8) & 0xff);
		printf("line size=%ld bytes.\n", ecx & 0xff);

		printf("L1 Instruction cache:\n\t");
		printf("Size: %ldKb\t", edx >> 24);
		do_assoc((edx >> 16) & 0xff);
		printf("\n\t");
		printf("lines per tag=%ld\t", (edx >> 8) & 0xff);
		printf("line size=%ld bytes.\n", edx & 0xff);
	}

	/* check K6-III (and later) on-chip L2 cache size */
	if (cpu->maxei >= 0x80000006) {
		cpuid(cpu->number, 0x80000006, &eax, &ebx, &ecx, &edx);
		printf("L2 (on CPU) cache:\n\t");
		printf("Size: %ldKb\t", ecx >> 16);
		do_assoc((ecx >> 12) & 0x0f);
		printf("\n\t");
		printf("lines per tag=%ld\t", (ecx >> 8) & 0x0f);
		printf("line size=%ld bytes.\n", ecx & 0xff);
	}
	printf("\n");
}


/*
 * Returns size of L2 cache for Duron/Athlon descrimination
 * Assumes 0x80000006 is valid.
 */
static int getL2size(int cpunum)
{
	unsigned long eax, ebx, ecx, edx;
	cpuid(cpunum, 0x80000006, &eax, &ebx, &ecx, &edx);
	return (ecx >> 16);
}


static int is_mobile(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;
	if (cpu->maxei >= 0x80000007) {
		cpuid(cpu->number, 0x80000007, &eax, &ebx, &ecx, &edx);
		if ((edx & (1<<1|1<<2)) == 0)
			return 0;
		else
			return 1;
	} else {
		return 0;
	}
}


static void determine_xp_mp(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	/* There are no mobile MPs. */
	if (is_mobile(cpu)) {
		add_to_cpuname("XP");
		return;
	}

	cpuid(cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
	if ((edx & (1 << 19)) == 0) {
		add_to_cpuname ("XP");
	} else {
		add_to_cpuname ("MP");
	}
}


void Identify_AMD(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	namebegin = amd_nameptr = cpu->name;
	cpu->vendor = VENDOR_AMD;

	if (cpu->maxi < 1)
		return;

	cpuid(cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;
	cpuid(cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
	cpu->efamily = (eax >>8) & 0xf;

	switch (cpu->family) {
	case 4:
		cpu->connector = CONN_SOCKET_3;
		break;
	}

	switch (tuple(cpu) & 0xff0) {
	case 0x430:
		sprintf(cpu->name, "%s", "Am486DX2-WT");
		break;
	case 0x470:
		sprintf(cpu->name, "%s", "Am486DX2-WB");
		break;
	case 0x480:
		sprintf(cpu->name, "%s", "Am486DX4-WT / Am5x86-WT");
		break;
	case 0x490:
		sprintf(cpu->name, "%s", "Am486DX4-WB / Am5x86-WB");
		break;
	case 0x4a0:
		sprintf(cpu->name, "%s", "Elan SC400");
		break;
	case 0x4e0:
		sprintf(cpu->name, "%s", "Am5x86-WT");
		break;
	case 0x4f0:
		sprintf(cpu->name, "%s", "Am5x86-WB");
		break;

	case 0x500:
		sprintf(cpu->name, "%s", "SSA5 (PR75/PR90/PR100)");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x510:
		sprintf(cpu->name, "%s", "K5 (PR120/PR133)");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x520:
		sprintf(cpu->name, "%s", "K5 (PR166)");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x530:
		sprintf(cpu->name, "%s", "K5 (PR200)");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x560:
		sprintf(cpu->name, "%s", "K6 (0.30 um)");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x570:
		sprintf(cpu->name, "%s", "K6 (0.25 um)");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x580:
		add_to_cpuname("K6-2");
		cpu->connector = CONN_SUPER_SOCKET_7;
		if (cpu->stepping >= 8)
			add_to_cpuname (" (CXT core)");
		break;
	case 0x590:
		sprintf(cpu->name, "%s", "K6-III");
		cpu->connector = CONN_SUPER_SOCKET_7;
		break;
	case 0x5c0:
		sprintf(cpu->name, "%s", "K6-2+ (0.18um)");
		cpu->connector = CONN_SUPER_SOCKET_7;
		break;
	case 0x5d0:
		sprintf(cpu->name, "%s", "K6-3+ (0.18um)");
		cpu->connector = CONN_SUPER_SOCKET_7;
		break;

	case 0x600:
		cpu->connector = CONN_SLOT_A;
		sprintf(cpu->name, "%s", "K7 ES");
		break;

	case 0x610:
		cpu->connector = CONN_SLOT_A;
		add_to_cpuname ("Athlon (0.25um)");
		switch (cpu->stepping) {
		case 1:
			add_to_cpuname (" [C1]");
			break;
		case 2:
			add_to_cpuname (" [C2]");
			break;
		}
		break;

	case 0x620:
		cpu->connector = CONN_SLOT_A;
		add_to_cpuname ("Athlon (0.18um)");
		switch (cpu->stepping) {
		case 1:
			add_to_cpuname (" [A1]");
			break;
		case 2:
			add_to_cpuname (" [A2]");
			break;
		}
		break;

	case 0x630:
		cpu->connector = CONN_SOCKET_A;
		add_to_cpuname ("Duron (spitfire)");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname (" [A0]");
			break;
		case 1:
			add_to_cpuname (" [A2]");
			break;
		}
		break;

	case 0x640:
		cpu->connector = CONN_SOCKET_A;
		add_to_cpuname ("Athlon (Thunderbird)");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname (" [A1]");
			break;
		case 1:
			add_to_cpuname (" [A2]");
			break;
		case 2:
			add_to_cpuname (" [A4-A8]");
			break;
		case 3:
			add_to_cpuname (" [A9]");
			break;
		}
		break;

	case 0x660:
		cpu->connector = CONN_SOCKET_A;

		if (is_mobile(cpu)) {
			add_to_cpuname ("Mobile Athlon 4");
			goto out_660;
		}
		if (getL2size(cpu->number) < 256) {
			add_to_cpuname ("Duron (Morgan)");
		} else {
			add_to_cpuname ("Athlon ");
			determine_xp_mp(cpu);
			add_to_cpuname (" (palomino)");
		}
out_660:
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname (" [A0-A1]");
			break;
		case 1:
			add_to_cpuname (" [A2]");
			break;
		case 2:
			//add_to_cpuname (" []");
			break;
		}
		break;

	case 0x670:
		cpu->connector = CONN_SOCKET_A;
		if (is_mobile(cpu))
			add_to_cpuname ("Mobile ");
		add_to_cpuname ("Duron (Morgan core)");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname (" [A0]");
			break;
		case 1:
			add_to_cpuname (" [A1]");
			break;
		}
		break;

	case 0x680:
		cpu->connector = CONN_SOCKET_A;
		if (is_mobile(cpu))
			add_to_cpuname ("Mobile ");
		if (getL2size(cpu->number) < 256) {
			add_to_cpuname ("Duron ");
		} else {
			add_to_cpuname ("Athlon ");
			determine_xp_mp(cpu);
		}
		add_to_cpuname (" (Thoroughbred)");

		if (cpu->stepping == 0)
			add_to_cpuname ("[A0]");
		if (cpu->stepping == 1)
			add_to_cpuname ("[B0]");
		//fab_process = ".13 micron";
		//transistors = 37600000;
		//die_size = "84 sq.mm";
		break;

	case 0x6a0:
		cpu->connector = CONN_SOCKET_A;
		if (is_mobile(cpu))
			add_to_cpuname("Mobile ");
		add_to_cpuname ("Athlon ");
		determine_xp_mp(cpu);
		add_to_cpuname (" (Barton)");
		//fab_process = ".13 micron copper";
		//transistors = 54300000;
		//die_size = "101 sq. mm";
		break;

	case 0xF00:
		cpu->connector = CONN_SOCKET_754;
		add_to_cpuname ("Athlon 64 ES");
		break;
	case 0xF10:
		cpu->connector = CONN_SOCKET_940;
		add_to_cpuname ("Opteron ES");
		break;

	case 0xF40:
	case 0xF80:
		cpu->connector = CONN_SOCKET_754;
		add_to_cpuname ("Athlon 64");
		break;

	case 0xF50:
		cpu->connector = CONN_SOCKET_940;
		add_to_cpuname ("Opteron");
		switch (cpu->stepping) {
			case 0:
				add_to_cpuname("[B0]");
				break;
			case 1:
				add_to_cpuname ("[SH7-B3]");
				break;
			default:
				break;
		}
		break;

	default:
		add_to_cpuname ("Unknown CPU");
		break;
	}
}


void display_AMD_info(struct cpudata *cpu)
{
	printf("Family: %d Model: %d Stepping: %d\n",
	       cpu->family, cpu->model, cpu->stepping);
	printf ("CPU Model : %s\n", cpu->name);
	get_model_name(cpu);

	decode_feature_flags(cpu);

	if (show_msr) {
		if (cpu->family == 5)
			dump_k6_MSR(cpu);
		if (cpu->family == 6)
			dump_athlon_MSR(cpu);
	}

	if (show_bluesmoke)
		decode_athlon_bluesmoke(cpu->number);

	if (show_cacheinfo)
		decode_AMD_cacheinfo(cpu);

	if (show_pm)
		decode_powernow(cpu);

	if (show_bugs)
		show_amd_bugs(cpu);
}

