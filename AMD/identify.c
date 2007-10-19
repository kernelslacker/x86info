/*
 *  $Id: identify.c,v 1.54 2004/06/11 12:35:05 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Copyright (C) 2007 Advanced Micro Devices, Inc.
 *                     Andreas Herrmann <andreas.herrmann3@amd.com>
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific information
 *
 * http://www.pbase.com/image/17079307/original
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "../x86info.h"
#include "AMD.h"
#include "revision.h"

static char *amd_nameptr;
#define add_to_cpuname(x) \
do { \
	amd_nameptr += snprintf(amd_nameptr, sizeof(x), "%s", x); \
} while(0)

static void set_k8_name(struct k8_rev *r, struct cpudata *c)
{
	int i, id, cont;
	char s[CPU_NAME_LEN];
	const char *p;

	s[0] = 0;
	cont = 0;
	for (i=0; (r!=NULL) && (i<ARRAY_SIZE(k8_names)); i++) {
		p = NULL;
		id = 1<<i;
		if (r->nameid & id)
			p = get_k8_name(id);
		if (p) {
			if (cont)
				strncat(s, "/", CPU_NAME_LEN-1);
			else
				cont = 1;
			strncat(s, p, CPU_NAME_LEN-1);
		}
	}
	if (r)
		snprintf(c->name, CPU_NAME_LEN, "%s (%s)", s, r->rev);
	else
		snprintf(c->name, CPU_NAME_LEN, "Unknown CPU");
}

void set_k8_revinfo(int id, struct cpudata *c)
{
	int i;
	struct k8_rev *r;

	c->connector = 0;
	r = NULL;
	for (i=0; i<ARRAY_SIZE(k8_revisions); i++) {
		if (k8_revisions[i].eax == id) {
			r = &k8_revisions[i];
			break;
		}
	}

	set_k8_name(r, c);
	c->connector = r ? r->socketid : 0;
}

void set_fam10h_revinfo(int id, struct cpudata *c)
{
	unsigned long eax, ebx, ecx, edx;
	int pkg_id;
	const char *p;
	

	p = get_fam10h_revision_name(id);
	if (p)
		snprintf(c->name, CPU_NAME_LEN,
			 "Quad-Core/Dual-Core/Embedded Opteron (%s)", p);
	else
		snprintf(c->name, CPU_NAME_LEN, "Unknown CPU");

	cpuid(c->number, 0x80000001, &eax, &ebx, &ecx, &edx);
	pkg_id = (ebx >> 28) & 0xf;

	switch (pkg_id) {
	case 0:
		c->connector = CONN_SOCKET_F_R2;
		break;
	case 1:
		c->connector = CONN_SOCKET_AM2_R2;
		break;
	default:
		c->connector = 0;
	}
}

static void do_assoc(unsigned long assoc)
{
	switch (assoc & 0xff) {
	case 0x0:
		break;
	case 0x1:
		printf("Direct mapped. ");
		break;
	case 0xff:
		printf("Fully associative. ");
		break;
	default:
		printf("%lu-way associative. ", assoc);
	}
}

static void do_l2assoc(unsigned long assoc)
{
	unsigned long a;

	a = 0;
	switch (assoc) {
	case 0x0:
		printf("Disabled. ");
		break;
	case 0x1:
		printf("Direct mapped. ");
		break;
	case 0xf:
		printf("Fully associative. ");
		break;
	case 0x2:
		a = 2;
		break;
	case 0x4:
		a = 4;
		break;
	case 0x6:
		a = 8;
		break;
	case 0x8:
		a = 16;
		break;
	case 0xa:
		a = 32;
		break;
	case 0xb:
		a = 48;
		break;
	case 0xc:
		a = 64;
		break;
	case 0xd:
		a = 96;
		break;
	case 0xe:
		a = 128;
		break;
	default:
		break;
	}

	if (a)
		printf("%lu-way associative. ", a);
}

static void decode_AMD_cacheinfo(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	if ((cpu->eflags_edx & 1<<26) && cpu->maxei >= 0x80000019) {
		/* 1GB page TLB info */
		cpuid(cpu->number, 0x80000019, &eax, &ebx, &ecx, &edx);

		printf("L1 Data TLB (1G):           ");
		do_l2assoc(eax >> 28);
		printf("%lu entries.\n", (eax >> 16) & 0xfff);
		printf("L1 Instruction TLB (1G):    ");
		do_l2assoc((eax >> 12) & 0xf);
		printf("%lu entries.\n", eax & 0xfff);
	}

	if (cpu->maxei >= 0x80000005) {
		/* TLB and cache info */
		cpuid(cpu->number, 0x80000005, &eax, &ebx, &ecx, &edx);

		printf("L1 Data TLB (2M/4M):        ");
		do_assoc(eax >> 24);
		printf("%lu entries.\n", (eax >> 16) & 0xff);
		printf("L1 Instruction TLB (2M/4M): ");
		do_assoc((eax >> 8) & 0xff);
		printf("%lu entries.\n", eax & 0xff);

		printf("L1 Data TLB (4K):           ");
		do_assoc(ebx >> 24);
		printf("%lu entries.\n", (ebx >> 16) & 0xff);
		printf("L1 Instruction TLB (4K):    ");
		do_assoc((ebx >> 8) & 0xff);
		printf("%lu entries.\n", ebx & 0xff);

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

	if ((cpu->eflags_edx & 1<<26) && cpu->maxei >= 0x80000019) {
		/* 1GB page TLB info */
		cpuid(cpu->number, 0x80000019, &eax, &ebx, &ecx, &edx);

		printf("L2 Data TLB (1G):           ");
		do_l2assoc(ebx >> 28);
		printf("%lu entries.\n", (ebx >> 16) & 0xfff);
		printf("L2 Instruction TLB (1G):    ");
		do_l2assoc((ebx >> 12) & 0xf);
		printf("%lu entries.\n", ebx & 0xfff);
	}

	if (cpu->maxei >= 0x80000006) {
		/* K6-III (and later) on-chip L2 cache size */
		cpuid(cpu->number, 0x80000006, &eax, &ebx, &ecx, &edx);

		printf("L2 Data TLB (2M/4M):        ");
		do_l2assoc(eax >> 28);
		printf("%lu entries.\n", (eax >> 16) & 0xfff);
		printf("L2 Instruction TLB (2M/4M): ");
		do_l2assoc((eax >> 12) & 0xf);
		printf("%lu entries.\n", eax & 0xfff);

		printf("L2 Data TLB (4K):           ");
		do_l2assoc(ebx >> 28);
		printf("%lu entries.\n", (ebx >> 16) & 0xfff);
		printf("L2 Instruction TLB (4K):    ");
		do_l2assoc((ebx >> 12) & 0xf);
		printf("%lu entries.\n", ebx & 0xfff);

		printf("L2 cache:\n\t");
		printf("Size: %luKb\t", ecx >> 16);
		do_l2assoc((ecx >> 12) & 0x0f);
		printf("\n\t");
		printf("lines per tag=%lu\t", (ecx >> 8) & 0x0f);
		printf("line size=%lu bytes.\n", ecx & 0xff);
		if (family(cpu) == 0x10) {
			/* family 0x10 has shared L3  cache */
			printf("L3 (shared) cache:\n\t");
			printf("Size: %luKb\t",
			       (edx >> 18) * 512);
			do_l2assoc((edx >> 12) & 0x0f);
			printf("\n\t");
			printf("lines per tag=%lu\t", (edx >> 8) & 0x0f);
			printf("line size=%lu bytes.\n", edx & 0xff);
		}
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
	if (cpu->family == 0xf) {
		cpu->emodel = (eax >> 16) & 0xf;
		cpu->efamily= (eax >> 20) & 0xff;
	} else {
		cpu->emodel = 0;
		cpu->efamily = 0;
	}

	if (family(cpu) == 0xf) {
		set_k8_revinfo(eax, cpu);
		return;
	} else if (family(cpu) == 0x10) {
		set_fam10h_revinfo(eax, cpu);
		return;
	}

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
		// 166,200,233 MHz
		add_to_cpuname("K6 (0.30 um)");
		cpu->connector = CONN_SOCKET_7;
		break;
	case 0x570:
		// 200,233,266,300 MHz
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
	default:
		add_to_cpuname("Unknown CPU");
		break;
	}
}

void display_AMD_info(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	printf("Family: %u Model: %u Stepping: %u\n",
	       family(cpu), model(cpu), cpu->stepping);
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

	if (cpu->maxi >= 0x05) {
		cpuid (cpu->number, 0x05, &eax, &ebx, &ecx, NULL);
		printf("Monitor/Mwait: min/max line size %d/%d%s%s\n",
		       (int) (eax & 0xffff), (int) (ebx & 0xffff),
		       (ecx & 0x2) ? ", ecx bit 0 support" : "",
		       (ecx & 0x1) ? ", enumeration extension" : "");
	}

	if (cpu->maxei >= 0x8000000a) {
		cpuid (cpu->number, 0x8000000a, &eax, &ebx, NULL, &edx);
		printf("SVM: revision %d, %d ASIDs",
		       (int) (eax & 0xff), (int) ebx);
		if (edx & 1)
			printf(", np");
		if (edx & 2)
			printf(", lbrVirt");
		if (edx & 4)
			printf(", SVMLock");
		if (edx & 8)
			printf(", NRIPSave");
		printf("\n");
	}

	/* AMD Multicore characterisation */
	if (cpu->maxei >= 0x80000008) {
		int n, p;
		cpuid (cpu->number, 0x80000008, &eax, NULL, &ecx, NULL);
		printf("Address Size: %d bits virtual, %d bits physical\n",
		       (int) (eax >> 8) & 0xff, (int) eax & 0xff);

		p = (ecx >> 12) & 0xf;
		n = (ecx & 0xff) + 1;
		if (p)
			p = 1 << p;
		else
			p = n;

		if (p > 1)
			printf("The physical package has %d of %d "
			       "possible cores implemented.\n", n, p);
	}


}

