/*
 *  $Id: identify.c,v 1.54 2004/06/11 12:35:05 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific information
 *
 * http://www.pbase.com/image/17079307/original
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "AMD.h"

static char *amd_nameptr;
#define add_to_cpuname(x)	amd_nameptr += snprintf(amd_nameptr, sizeof(x), "%s", x);

static void do_assoc(unsigned long assoc)
{
	if ((assoc & 0xff) == 255)
		printf("Fully");
	else
		printf("%lu-way", assoc);
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
		printf("%lu entries.\n", ebx & 0xff);

		printf("Data TLB: ");
		do_assoc(ebx >> 24);
		printf("%lu entries.\n", (ebx >> 16) & 0xff);

		printf("L1 Data cache:\n\t");
		printf("Size: %luKb\t", ecx >> 24);
		do_assoc((ecx >> 16) & 0xff);
		printf("\n\t");
		printf("lines per tag=%lu\t", (ecx >> 8) & 0xff);
		printf("line size=%lu bytes.\n", ecx & 0xff);

		printf("L1 Instruction cache:\n\t");
		printf("Size: %luKb\t", edx >> 24);
		do_assoc((edx >> 16) & 0xff);
		printf("\n\t");
		printf("lines per tag=%lu\t", (edx >> 8) & 0xff);
		printf("line size=%lu bytes.\n", edx & 0xff);
	}

	/* check K6-III (and later) on-chip L2 cache size */
	if (cpu->maxei >= 0x80000006) {
		cpuid(cpu->number, 0x80000006, &eax, &ebx, &ecx, &edx);
		printf("L2 (on CPU) cache:\n\t");
		printf("Size: %luKb\t", ecx >> 16);
		do_assoc((ecx >> 12) & 0x0f);
		printf("\n\t");
		printf("lines per tag=%lu\t", (ecx >> 8) & 0x0f);
		printf("line size=%lu bytes.\n", ecx & 0xff);
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
		add_to_cpuname("XP");
	} else {
		add_to_cpuname("MP");
	}
}


void Identify_AMD(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	amd_nameptr = cpu->name;
	cpu->vendor = VENDOR_AMD;

	if (cpu->maxi < 1)
		return;

	cpuid(cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;
	cpu->emodel = (eax >> 16) & 0xff;
	cpu->efamily= (eax >> 20) & 0xf;

	switch (cpu->family) {
	case 4:
		cpu->connector = CONN_SOCKET_3;
		break;
	}

	switch (tuple(cpu) & 0xff0) {
	case 0x430:
		add_to_cpuname("Am486DX2-WT");
		break;
	case 0x470:
		add_to_cpuname("Am486DX2-WB");
		break;
	case 0x480:
		add_to_cpuname("Am486DX4-WT / Am5x86-WT");
		break;
	case 0x490:
		add_to_cpuname("Am486DX4-WB / Am5x86-WB");
		break;
	case 0x4a0:
		add_to_cpuname("Elan SC400");
		break;
	case 0x4e0:
		add_to_cpuname("Am5x86-WT");
		break;
	case 0x4f0:
		add_to_cpuname("Am5x86-WB");
		break;

	case 0x500:
		add_to_cpuname("SSA5 (PR75/PR90/PR100)");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x510:
		add_to_cpuname("K5 (PR120/PR133)");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x520:
		add_to_cpuname("K5 (PR166)");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x530:
		add_to_cpuname("K5 (PR200)");
		cpu->connector = CONN_SOCKET_5_7;
		break;
	case 0x560:
		add_to_cpuname("K6 (0.30 um)");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x570:
		add_to_cpuname("K6 (0.25 um)");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x580:
		add_to_cpuname("K6-2");
		cpu->connector = CONN_SUPER_SOCKET_7;
		if (cpu->stepping >= 8)
			add_to_cpuname(" (CXT core)");
		break;
	case 0x590:
		add_to_cpuname("K6-III");
		cpu->connector = CONN_SUPER_SOCKET_7;
		break;
	case 0x5c0:
		add_to_cpuname("K6-2+ (0.18um)");
		cpu->connector = CONN_SUPER_SOCKET_7;
		break;
	case 0x5d0:
		add_to_cpuname("K6-3+ (0.18um)");
		cpu->connector = CONN_SUPER_SOCKET_7;
		break;

	case 0x600:
		cpu->connector = CONN_SLOT_A;
		add_to_cpuname("K7 ES");
		break;

	case 0x610:
		cpu->connector = CONN_SLOT_A;
		add_to_cpuname("Athlon (0.25um)");
		switch (cpu->stepping) {
		case 1:
			add_to_cpuname(" [C1]");
			break;
		case 2:
			add_to_cpuname(" [C2]");
			break;
		}
		break;

	case 0x620:
		cpu->connector = CONN_SLOT_A;
		add_to_cpuname("Athlon (0.18um)");
		switch (cpu->stepping) {
		case 1:
			add_to_cpuname(" [A1]");
			break;
		case 2:
			add_to_cpuname(" [A2]");
			break;
		}
		break;

	case 0x630:
		cpu->connector = CONN_SOCKET_A;
		add_to_cpuname("Duron (spitfire)");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname(" [A0]");
			break;
		case 1:
			add_to_cpuname(" [A2]");
			break;
		}
		break;

	case 0x640:
		cpu->connector = CONN_SOCKET_A;
		add_to_cpuname("Athlon (Thunderbird)");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname(" [A1]");
			break;
		case 1:
			add_to_cpuname(" [A2]");
			break;
		case 2:
			add_to_cpuname(" [A4-A8]");
			break;
		case 3:
			add_to_cpuname(" [A9]");
			break;
		}
		break;

	case 0x660:
		cpu->connector = CONN_SOCKET_A;

		if (is_mobile(cpu)) {
			add_to_cpuname("Mobile Athlon 4");
			goto out_660;
		}
		if (getL2size(cpu->number) < 256) {
			add_to_cpuname("Duron (Morgan)");
		} else {
			add_to_cpuname("Athlon ");
			determine_xp_mp(cpu);
			/* Palomino
			 * 0.18u L2=256KB
			 * 266MHz FSB
			 * 12%-20% faster than Athlon Thunderbird at same GHz
			 * Power requirement reduced by 20%
			 * Athlon XP 1500+ (Oct 2001)
			 * Athlon XP 1600+ (Oct 2001)
			 * Athlon XP 1700+ (Oct 2001)
			 * Athlon XP 1800+ (Oct 2001)
			 * Athlon XP 1900+ (Nov 2001)
			 * Athlon XP 2000+ (Jan 2002)
			 * Athlon XP 2100+ (Mar 2002)
			 */
			add_to_cpuname(" (Palomino)");
		}
out_660:
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname(" [A0-A1]");
			break;
		case 1:
			add_to_cpuname(" [A2]");
			break;
		}
		break;

	case 0x670:
		cpu->connector = CONN_SOCKET_A;
		if (is_mobile(cpu))
			add_to_cpuname("Mobile ");
		add_to_cpuname("Duron (Morgan core)");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname(" [A0]");
			break;
		case 1:
			add_to_cpuname(" [A1]");
			break;
		}
		break;

	case 0x680:
		cpu->connector = CONN_SOCKET_A;
		if (is_mobile(cpu))
			add_to_cpuname("Mobile ");
		if (getL2size(cpu->number) < 256) {
			add_to_cpuname("Duron ");
		} else {
			add_to_cpuname("Athlon ");
			determine_xp_mp(cpu);
		}
		/*
		 * Thoroughbred
		 * 0.13u L2=256KB
		 * Thoroughbred 'A' = 266FSB
		 * Thoroughbred 'B' = 266FSB
		 * Thoroughbred 'B' = 333FSB
		 * Throughbred B has an extra layer of copper interconnects
		 * to reduce interference.
		 * Athlon XP1600+ (A:June 2002 B:Mar 2003)
		 * Athlon XP1700+ (A:June 2002 B:Dec 2002)
		 * Athlon XP1800+ (A:June 2002 B:Dec 2002)
		 * Athlon XP1900+ (A:June 2002 B:Dec 2002)
		 * Athlon XP2000+ (A:June 2002 B:Aug 2002)
		 * Athlon XP2100+ (A:June 2002 B:Dec 2002)
		 * Athlon XP2200+ (A:June 2002 B:Aug 2002)
		 * Athlon XP2400+ (            B:Aug 2002)
		 * Athlon XP2600+ (            B:Aug 2002 B2: Nov 2002)
		 * Athlon XP2700+ (                       B2: Oct 2002)
		 * Athlon XP2800+ (                       B2: Oct 2002)
		 */
		add_to_cpuname(" (Thoroughbred)");

		if (cpu->stepping == 0)
			add_to_cpuname("[A0]");
		if (cpu->stepping == 1)
			add_to_cpuname("[B0]");
		//fab_process = ".13 micron";
		//transistors = 37600000;
		//die_size = "84 sq.mm";
		break;

	case 0x6a0:
		cpu->connector = CONN_SOCKET_A;
		if (is_mobile(cpu))
			add_to_cpuname("Mobile ");
		add_to_cpuname("Athlon ");
		determine_xp_mp(cpu);
		add_to_cpuname(" (Barton)");
		//fab_process = ".13 micron copper";
		//transistors = 54300000;
		//die_size = "101 sq. mm";
		/* Barton
		 * L2=512
		 * 333 FSB & 400 FSB
		 * 10%-15% faster than Athlon XP (old) with same GHz
		 * CPU core size 20% bigger than T-bred.
		 * 333 FSB:
		 * Athlon XP 2500+ (Feb 2003)
		 * Athlon XP 2600+ (June 2003)
		 * Athlon XP 2800+ (Feb 2003)
		 * Athlon XP 3000+ (Feb 2003)
		 * 400 FSB:
		 * Athlon XP 3000+ (Apr 2003)
		 * Athlon XP 3200+ (May 2003)
		 * Athlon XP 3400+ (Q4 2003)
		 * Athlon XP 3600+ (Q1 2004)
		 * Athlon XP 3800+ (Q2 2004)
		 */
		break;
/*
 * Applebred
 * 0.13u L2=64KB
 * 266FSB
 * Barton grade processor modules with 64KB cache
 *
 * Duron 1.4 (Aug 2003)
 * Duron 1.6 (Aug 2003)
 * Duron 1.8 (Q4 2003)
 */

/*
 * Socket 940
 * Sledgehammer
 * 0.13u
 * L2=1mb
 * 400FSB
 * SOI (silicon on insulator)
 * Registered DIMM required
 * 25% faster than Athlon XP with same GHz
 * Athlon 64 FX51 (Sep 2003)
 * Athlon 64 FX53 (Nov 2003)
 * Athlon 64 FX55 (Q4 2003)
 */

 /*
  * Socket 754
  * Clawhammer
  * 0.13
  * L2=1mb
  * 400FSB
  * No dual channel memory access
  * Registered DIMM not required
  * Athlon 64 3000+ (Oct 2003)
  * Athlon 64 3200+ (Sep 2003)
  * Athlon 64 3400+ (Oct 2003)
  * Athlon 64 3700+ (Q4 2003)
  */

 /*
  * Socket 939
  * San Diego
  * 90nm l2=1mb
  * 400FSB
  * Dual channel memory access
  * Registered DIMM not required
  * Athlon64 FX-57 (Q1 2004)
  */

 /*
  * Socket 754
  * Victoria
  * 90nm L2=1MB
  * 400FSB
  * No dual channel memory access
  * Registered DIMM not required
  * Athlon64 3x00+ (Q3 2004)
  */

	case 0xF00:
		cpu->connector = CONN_SOCKET_754;
		add_to_cpuname("Athlon 64 ");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname("[SH7-A0]");
			break;
		case 1:
			add_to_cpuname("[SH7-A2]");
			break;
		}
		break;

	case 0xF10:
		add_to_cpuname("Opteron ES ");
		cpu->connector = CONN_SOCKET_940;
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname("[SH7-A0]");
			break;
		case 1:
			add_to_cpuname("[SH7-A2]");
			break;
		}
		break;

	case 0xF40:
		cpu->connector = CONN_SOCKET_754;
		add_to_cpuname("Athlon 64 ");
		switch (cpu->stepping) {
		case 0:
			if (cpu->emodel==0) {
				add_to_cpuname("[SH7-B0]");
			} else {
				add_to_cpuname("[SH8-D0]");
			}
			break;
		case 8:
			// need to check for longmode bit. could be athlon xp 3000+.
			//  (These are 32bit only amd64's)
			//might be mobile
			add_to_cpuname("[SH7-C0]");
			break;
		case 0xa:
			//might be mobile
			add_to_cpuname("[SH7-CG]");
			break;
		}
		break;

	// Gar, these could also be athlon 64fx
	case 0xF50:
		cpu->connector = CONN_SOCKET_940;
		add_to_cpuname("Opteron");
		switch (cpu->stepping) {
		case 0:
			if (cpu->emodel==0) {
				add_to_cpuname("[SH7-B0]");
			} else {
				add_to_cpuname("[SH8-D0]");
			}
			break;
		case 1:
			add_to_cpuname("[SH7-B3]");
			break;
		case 8:
			add_to_cpuname("[SH7-C0]");
			break;
		case 0xA:
			add_to_cpuname("[SH7-CG]");
			break;
		default:
			break;
		}
		break;

	case 0xF70:
		add_to_cpuname("Athlon 64 ");
		cpu->connector = CONN_SOCKET_939;
		switch (cpu->stepping) {
		case 0x0:
			add_to_cpuname("[SH8-D0]");
			break;
		case 0xa:
			add_to_cpuname("[SH7-CG]");
			break;
		}
		break;

	case 0xF80:
		cpu->connector = CONN_SOCKET_754;
		add_to_cpuname("Athlon 64 ");
		switch (cpu->stepping) {
		case 2:
			//might be mobile
			add_to_cpuname("CH7-CG");
			break;
		}
		break;

	case 0xFB0:
		cpu->connector = CONN_SOCKET_939;
		add_to_cpuname("Athlon 64 ");
		switch (cpu->stepping) {
		case 2:
			add_to_cpuname("CH7-CG");
			break;
		}
		break;

	case 0xFC0:
		cpu->connector = CONN_SOCKET_754;
		add_to_cpuname("Athlon 64 ");
		switch (cpu->stepping) {
		case 0:
			//might be mobile
			add_to_cpuname("DH7-CG");
			break;
		}
		break;

	case 0xFE0:
		//might be mobile
		cpu->connector = CONN_SOCKET_754;
		add_to_cpuname("Athlon 64 ");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname("DH7-CG");
			break;
		}
		break;


	case 0xFF0:
		cpu->connector = CONN_SOCKET_939;
		add_to_cpuname("Athlon 64 ");
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname("DH7-CG");
			break;
		}
		break;

	default:
		add_to_cpuname("Unknown CPU");
		break;
	}
}


void display_AMD_info(struct cpudata *cpu)
{
	printf("Family: %u Model: %u Stepping: %u\n",
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

	/* AMD Multicore characterisation */
	if (cpu->maxei >= 0x80000008) {
		int nr_cores;
		unsigned long ecx;

		cpuid (cpu->number, 0x80000008, NULL, NULL, &ecx, NULL);
		nr_cores = 1 + (ecx & 0xff);

		if (nr_cores > 1)
			printf ("The physical package has %d cores\n", nr_cores);
	}
}

