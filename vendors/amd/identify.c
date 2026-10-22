/*
 *  (C) 2001 Dave Jones.
 *
 *  Copyright (C) 2007 Advanced Micro Devices, Inc.
 *                     Andreas Herrmann <andreas.herrmann3@amd.com>
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific information
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <x86info.h>
#include <amd.h>
#include <amd-revision.h>

static void set_k8_name(struct k8_rev *r, struct cpudata *c)
{
	unsigned int i, id, cont;

	c->name[0] = 0;
	if (!r) {
		cpu_name_append(c, "Unknown CPU");
		return;
	}

	cont = 0;
	for (i = 0; i < ARRAY_SIZE(k8_names); i++) {
		const char *p;

		p = NULL;
		id = 1<<i;
		if (r->nameid & id)
			p = get_k8_name(id);
		if (p) {
			if (cont)
				cpu_name_append(c, "/");
			else
				cont = 1;
			cpu_name_append(c, p);
		}
	}
	cpu_name_append(c, " (");
	cpu_name_append(c, r->rev);
	cpu_name_append(c, ")");
}

static void set_k8_revinfo(int id, struct cpudata *c)
{
	int i;
	struct k8_rev *r;

	c->connector = CONN_UNKNOWN;
	r = NULL;
	for (i = 0; i < ARRAY_SIZE(k8_revisions); i++) {
		if (k8_revisions[i].eax == id) {
			r = &k8_revisions[i];
			break;
		}
	}

	set_k8_name(r, c);
	c->connector = r ? r->socketid : CONN_UNKNOWN;
}


static void set_fam10h_name(struct fam10h_rev *r, struct cpudata *c)
{
	if (!r) {
		snprintf(c->name, CPU_NAME_LEN, "Unknown CPU");
		return;
	}

	snprintf(c->name, CPU_NAME_LEN,
		 "Phenom/Athlon/Sempron/Turion (II)/Opteron (%s)",
		 r->rev);
}

static void set_connector(struct cpudata *c)
{
	unsigned int eax, ebx, ecx, edx;
	int pkg_id;

	if (family(c) == 0x11) {
		c->connector = CONN_SOCKET_S1G2;
		return;
	}

	cpuid(c->number, 0x80000001, &eax, &ebx, &ecx, &edx);
	pkg_id = (ebx >> 28) & 0xf;


	if ((family(c) == 0x10) ||
	    ((family(c) == 0x15) && (model(c) <= 0xf))) {
		switch (pkg_id) {
		case 0:
			c->connector = CONN_SOCKET_F_R2;
			break;
		case 1:
			c->connector = CONN_SOCKET_AM3;
			break;
		case 2:
			c->connector = CONN_SOCKET_S1G3;
			break;
		case 3:
			c->connector = CONN_SOCKET_G34;
			break;
		case 4:
			c->connector = CONN_SOCKET_ASB2;
			break;
		case 5:
			c->connector = CONN_SOCKET_C32;
			break;
		default:
			c->connector = 0;
		}
	} else if ((family(c) == 0x15) && (model(c) <= 0x1f)) {
		switch (pkg_id) {
		case 0:
			c->connector = CONN_SOCKET_FP2;
			break;
		case 1:
			c->connector = CONN_SOCKET_FS1r2;
			break;
		case 2:
			c->connector = CONN_SOCKET_FM2;
			break;
		default:
			c->connector = 0;
		}
	} else if(family(c) == 0x12) {
		switch (pkg_id) {
		case 0:
			c->connector = CONN_SOCKET_FP1;
			break;
		case 1:
			c->connector = CONN_SOCKET_FS1;
			break;
		case 2:
			c->connector = CONN_SOCKET_FM1;
			break;
		default:
			c->connector = 0;
		}
	} else if (family(c) == 0x14) {
		switch (pkg_id) {
		case 0:
			c->connector = CONN_SOCKET_FT1;
			break;
		default:
			c->connector = 0;
		}
	}
}

static void set_fam10h_revinfo(int id, struct cpudata *c)
{
	int i;
	struct fam10h_rev *r = NULL;

	for (i = 0; i < ARRAY_SIZE(fam10h_revisions); i++) {
		if (fam10h_revisions[i].eax == id) {
			r = &fam10h_revisions[i];
			break;
		}
	}
	set_fam10h_name(r, c);
	set_connector(c);
}

static void set_fam11h_revinfo(int id, struct cpudata *c)
{
	const char *p;

	p = get_fam11h_revision_name(id);
	if (p)
		snprintf(c->name, CPU_NAME_LEN,
			 "AMD Turion X2 Ultra Dual-Core (%s)", p);
	else
		snprintf(c->name, CPU_NAME_LEN, "Unknown CPU");

	set_connector(c);
}

static void set_fam12h_revinfo(int id, struct cpudata *c)
{
	const char *p;

	p = get_fam12h_revision_name(id);
	if(p)
		snprintf(c->name, CPU_NAME_LEN,
			 "AMD A/E2-Series Processor (%s)", p);
	else
		snprintf(c->name, CPU_NAME_LEN, "Unknown CPU");

	set_connector(c);
}

static void set_fam14h_revinfo(int id, struct cpudata *c)
{
	const char *p;

	p = get_fam14h_revision_name(id);
	if (p)
		snprintf(c->name, CPU_NAME_LEN,
			 "AMD C/E/G-Series Processor (%s)", p);
	else
		snprintf(c->name, CPU_NAME_LEN, "Unknown CPU");


	set_connector(c);
}

static void set_fam15h_revinfo(int id, struct cpudata *c)
{
	const char *p;

	p = get_fam15h_revision_name(id);
	if (p)
		snprintf(c->name, CPU_NAME_LEN,
			 "AMD FX Series Processor (%s)", p);
	else
		snprintf(c->name, CPU_NAME_LEN, "Unknown CPU");
	set_connector(c);
}

static void set_fam16h_revinfo(int id, struct cpudata *c)
{
	(void)id;
	if (model(c) <= 0x0f)
		snprintf(c->name, CPU_NAME_LEN, "AMD Jaguar APU (Kabini/Temash)");
	else if (model(c) >= 0x30 && model(c) <= 0x3f)
		snprintf(c->name, CPU_NAME_LEN, "AMD Puma APU (Beema/Mullins)");
	else
		snprintf(c->name, CPU_NAME_LEN, "AMD Family 16h Processor");
}

/* Zen die names, keyed on the full model (stepping ignored). */
struct zen_die {
	unsigned int lo, hi;
	const char *name;
};

static const struct zen_die fam17h_dies[] = {
	{0x00, 0x0f, "Zeppelin"},
	{0x10, 0x1f, "Raven Ridge"},
	{0x20, 0x2f, "Dali"},
	{0x30, 0x3f, "Rome"},
	{0x60, 0x6f, "Renoir"},
	{0x70, 0x7f, "Matisse"},
	{0x90, 0x9f, "Van Gogh"},
	{0xa0, 0xaf, "Mendocino"},
};

static const struct zen_die fam19h_dies[] = {
	{0x00, 0x0f, "Milan"},
	{0x10, 0x1f, "Genoa"},
	{0x20, 0x2f, "Vermeer"},
	{0x40, 0x4f, "Rembrandt"},
	{0x50, 0x5f, "Cezanne"},
	{0x60, 0x6f, "Raphael"},
	{0x70, 0x7f, "Phoenix"},
	{0xa0, 0xaf, "Bergamo/Siena"},
};

static const struct zen_die fam1ah_dies[] = {
	{0x00, 0x1f, "Turin"},
	{0x20, 0x2f, "Strix Point/Gorgon Point"},
	{0x40, 0x4f, "Granite Ridge"},
	{0x60, 0x6f, "Krackan Point"},
	{0x70, 0x7f, "Strix Halo"},
};

static void set_zen_revinfo(const char *series, const struct zen_die *dies,
			    unsigned int ndies, int id, struct cpudata *c)
{
	unsigned int i;

	for (i = 0; i < ndies; i++) {
		if (model(c) >= dies[i].lo && model(c) <= dies[i].hi) {
			snprintf(c->name, CPU_NAME_LEN,
				 "AMD %s Series Processor (%s)", series,
				 dies[i].name);
			return;
		}
	}
	snprintf(c->name, CPU_NAME_LEN, "Unknown CPU 0x%x", (unsigned)id);
}

static void set_fam17h_revinfo(int id, struct cpudata *c)
{
	set_zen_revinfo("Zen/Zen+/Zen2", fam17h_dies, ARRAY_SIZE(fam17h_dies), id, c);
}

static void set_fam19h_revinfo(int id, struct cpudata *c)
{
	set_zen_revinfo("Zen3/Zen3+/Zen4", fam19h_dies, ARRAY_SIZE(fam19h_dies), id, c);
}

static void set_fam1ah_revinfo(int id, struct cpudata *c)
{
	set_zen_revinfo("Zen5", fam1ah_dies, ARRAY_SIZE(fam1ah_dies), id, c);
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
	case 0x9:
		printf("See CPUID 0x8000001D. ");
		break;
	default:
		break;
	}

	if (a)
		printf("%lu-way associative. ", a);
}

void decode_amd_cacheinfo(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	if ((cpu->eflags_edx & 1<<26) && cpu->maxei >= 0x80000019) {
		/* 1GB page TLB info */
		cpuid(cpu->number, 0x80000019, &eax, &ebx, &ecx, &edx);

		printf("L1 Data TLB (1G):           ");
		do_l2assoc(eax >> 28);
		printf("%u entries.\n", (eax >> 16) & 0xfff);
		printf("L1 Instruction TLB (1G):    ");
		do_l2assoc((eax >> 12) & 0xf);
		printf("%u entries.\n", eax & 0xfff);
	}

	if (cpu->maxei >= 0x80000005) {
		/* TLB and cache info */
		cpuid(cpu->number, 0x80000005, &eax, &ebx, &ecx, &edx);

		printf("L1 Data TLB (2M/4M):        ");
		do_assoc(eax >> 24);
		printf("%u entries.\n", (eax >> 16) & 0xff);
		printf("L1 Instruction TLB (2M/4M): ");
		do_assoc((eax >> 8) & 0xff);
		printf("%u entries.\n", eax & 0xff);

		printf("L1 Data TLB (4K):           ");
		do_assoc(ebx >> 24);
		printf("%u entries.\n", (ebx >> 16) & 0xff);
		printf("L1 Instruction TLB (4K):    ");
		do_assoc((ebx >> 8) & 0xff);
		printf("%u entries.\n", ebx & 0xff);

		printf("L1 Data cache:\n\t");
		printf("Size: %uKb\t", ecx >> 24);
		do_assoc((ecx >> 16) & 0xff);
		printf("\n\t");
		printf("lines per tag=%u\t", (ecx >> 8) & 0xff);
		printf("line size=%u bytes.\n", ecx & 0xff);

		printf("L1 Instruction cache:\n\t");
		printf("Size: %uKb\t", edx >> 24);
		do_assoc((edx >> 16) & 0xff);
		printf("\n\t");
		printf("lines per tag=%u\t", (edx >> 8) & 0xff);
		printf("line size=%u bytes.\n", edx & 0xff);
	}

	if ((cpu->eflags_edx & 1<<26) && cpu->maxei >= 0x80000019) {
		/* 1GB page TLB info */
		cpuid(cpu->number, 0x80000019, &eax, &ebx, &ecx, &edx);

		printf("L2 Data TLB (1G):           ");
		do_l2assoc(ebx >> 28);
		printf("%u entries.\n", (ebx >> 16) & 0xfff);
		printf("L2 Instruction TLB (1G):    ");
		do_l2assoc((ebx >> 12) & 0xf);
		printf("%u entries.\n", ebx & 0xfff);
	}

	if (cpu->maxei >= 0x80000006) {
		/* K6-III (and later) on-chip L2 cache size */
		cpuid(cpu->number, 0x80000006, &eax, &ebx, &ecx, &edx);

		printf("L2 Data TLB (2M/4M):        ");
		do_l2assoc(eax >> 28);
		printf("%u entries.\n", (eax >> 16) & 0xfff);
		printf("L2 Instruction TLB (2M/4M): ");
		do_l2assoc((eax >> 12) & 0xf);
		printf("%u entries.\n", eax & 0xfff);

		printf("L2 Data TLB (4K):           ");
		do_l2assoc(ebx >> 28);
		printf("%u entries.\n", (ebx >> 16) & 0xfff);
		printf("L2 Instruction TLB (4K):    ");
		do_l2assoc((ebx >> 12) & 0xf);
		printf("%u entries.\n", ebx & 0xfff);

		printf("L2 cache:\n\t");
		printf("Size: %uKb\t", ecx >> 16);
		do_l2assoc((ecx >> 12) & 0x0f);
		printf("\n\t");
		printf("lines per tag=%u\t", (ecx >> 8) & 0x0f);
		printf("line size=%u bytes.\n", ecx & 0xff);
		if (family(cpu) >= 0x10) {
			printf("L3 (shared) cache:\n\t");
			if (!(edx >> 18))
				printf("none/disabled\n");
			else {
				/* shared L3  cache */
				printf("Size: %uKb\t",
				       (edx >> 18) * 512);
				do_l2assoc((edx >> 12) & 0x0f);
				printf("\n\t");
				printf("lines per tag=%u\t", (edx >> 8) & 0x0f);
				printf("line size=%u bytes.\n", edx & 0xff);
			}
		}
	}

	if (family(cpu) >= 0x17 && cpu->maxei >= 0x8000001d) {
		static const char * const ctype[] = {
			"", "data", "instruction", "unified"
		};
		unsigned int sub;

		printf("Caches (CPUID 0x8000001D):\n");
		for (sub = 0; sub < 8; sub++) {
			unsigned int type, ways, parts, line, sets;

			cpuid_count(cpu->number, 0x8000001d, sub, &eax, &ebx, &ecx, &edx);
			type = eax & 0x1f;
			if (type == 0 || type >= ARRAY_SIZE(ctype))
				break;
			ways = (ebx >> 22) + 1;
			parts = ((ebx >> 12) & 0x3ff) + 1;
			line = (ebx & 0xfff) + 1;
			sets = ecx + 1;
			printf("\tL%u %s: %uKb, %u-way, line size=%u bytes, shared by %u threads\n",
			       (eax >> 5) & 7, ctype[type],
			       ways * parts * line * sets / 1024, ways, line,
			       ((eax >> 14) & 0xfff) + 1);
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
	unsigned int eax, ebx, ecx, edx;
	cpuid(cpunum, 0x80000006, &eax, &ebx, &ecx, &edx);
	return (ecx >> 16);
}


static int is_mobile(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
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
	unsigned int eax, ebx, ecx, edx;

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


void identify_amd(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	if (cpu->cpuid_level < 1)
		return;

	cpuid(cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	if (cpu->family == 0xf) {
		cpu->emodel = (eax >> 16) & 0xf;
		cpu->efamily= (eax >> 20) & 0xff;
	} else {
		cpu->emodel = 0;
		cpu->efamily = 0;
	}

	if (family(cpu) >= 0xf) {
		switch (family(cpu)) {
		case 0xf:
			set_k8_revinfo(eax, cpu);
			break;
		case 0x10:
			set_fam10h_revinfo(eax, cpu);
			break;
		case 0x11:
			set_fam11h_revinfo(eax, cpu);
			break;
		case 0x12:
			set_fam12h_revinfo(eax, cpu);
			break;
		case 0x14:
			set_fam14h_revinfo(eax, cpu);
			break;
		case 0x15:
			set_fam15h_revinfo(eax, cpu);
			break;
		case 0x16:
			set_fam16h_revinfo(eax, cpu);
			break;
		case 0x17:
			set_fam17h_revinfo(eax, cpu);
			break;
		case 0x19:
			set_fam19h_revinfo(eax, cpu);
			break;
		case 0x1a:
			set_fam1ah_revinfo(eax, cpu);
			break;
		default:
			snprintf(cpu->name, CPU_NAME_LEN,
				 "Unknown CPU family: 0x%x", family(cpu));
			break;
		}

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

static void show_patch_level(struct cpudata *cpu)
{
	unsigned long long val = 0;
	if (!user_is_root)
		return;

	if (read_msr(cpu->number, 0x8b, &val) == 1) {
		if (val>0)
			printf("Microcode patch level: 0x%llx\n", val);
		printf("\n");
	}
}

/* CPUID 0x8000000A EDX */
static const char *svm_features[32] = {
	"np", "lbrVirt", "SVMLock", "NRIPSave",
	"TscRateMsr", "VmcbClean", "FlushByAsid", "DecodeAssists",
	NULL, NULL, "PauseFilter", NULL,
	"PauseFilterThreshold", "AVIC", NULL, "V_VMSAVE_VMLOAD",
	"vGIF", "GMET", "x2AVIC", NULL,
	"SpecCtrl", NULL, NULL, NULL,
	NULL, "VNMI", NULL, NULL,
	"SvmeAddrChk", NULL, NULL, NULL,
};

void display_amd_info(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx, i;

	if (show_msr) {
		if (cpu->family == 5)
			dump_k6_MSR(cpu);
		if (cpu->family == 6)
			dump_athlon_MSR(cpu);
		if (cpu->family >= 0xf)
			printf("MSR dump not supported for this family.\n");
	}

	if (show_machine_check)
		decode_athlon_machine_check(cpu->number, family(cpu));

	if (show_microcode && family(cpu) >= 0xf)
		show_patch_level(cpu);

	if (show_pm)
		decode_powernow(cpu);

	if (show_bugs)
		show_amd_bugs(cpu);

	if (show_feature_flags && cpu->cpuid_level >= 0x05) {
		cpuid (cpu->number, 0x05, &eax, &ebx, &ecx, NULL);
		printf("Monitor/Mwait: min/max line size %d/%d%s%s\n",
		       (int) (eax & 0xffff), (int) (ebx & 0xffff),
		       (ecx & 0x2) ? ", interrupt break event" : "",
		       (ecx & 0x1) ? ", enumeration extension" : "");
	}

	if (show_feature_flags && cpu->maxei >= 0x8000000a) {
		cpuid (cpu->number, 0x8000000a, &eax, &ebx, NULL, &edx);
		printf("SVM: revision %d, %d ASIDs",
		       (int) (eax & 0xff), (int) ebx);
		for (i = 0; i < 32; i++)
			if ((edx & (1u << i)) && svm_features[i])
				printf(", %s", svm_features[i]);
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

		/* NC counts logical processors from Zen onwards */
		if (p > 1 && family(cpu) >= 0x17) {
			printf("The physical package has %d of %d "
			       "possible threads implemented", n, p);
			if (cpu->maxei >= 0x8000001e) {
				unsigned int tpc;

				cpuid(cpu->number, 0x8000001e, NULL, &ebx, NULL, NULL);
				tpc = ((ebx >> 8) & 0xff) + 1;
				printf(" (%d cores)", n / tpc);
			}
			printf(".\n");
		} else if (p > 1) {
			printf("The physical package has %d of %d "
			       "possible cores implemented.\n", n, p);
		}
	}
}

