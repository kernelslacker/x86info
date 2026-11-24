/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  MTRR register dumping.
 */

#include <stdio.h>
#include <x86info.h>

#define IA32_MTRRCAP_WC   (1 << 10)
#define IA32_MTRRCAP_FIX  (1 << 8)
#define IA32_MTRRCAP_VCNT 0xFF

#define IA32_MTRR_DEFTYPE_E    0x800
#define IA32_MTRR_DEFTYPE_FE   0x400
#define IA32_MTRR_DEFTYPE_TYPE 0xFF

#define IA32_PHYBASE_TYPE   0xFF
#define IA32_PHYMASK_VALID  (1 << 11)

#define MSR_IA32_MTRRCAP      0xfe
#define MSR_IA32_MTRR_DEFTYPE 0x2ff

/* Architectural minimum when CPUID doesn't report the physical address width. */
#define MTRR_DEFAULT_PHYS_BITS 36

#define MTRR_NUM_TYPES 7

static const char * mtrr_types[MTRR_NUM_TYPES] =
{
	"uncacheable",
	"write-combining",
	"?",
	"?",
	"write-through",
	"write-protect",
	"write-back",
};

static void decode_address(unsigned long long val, unsigned int max_phy_addr)
{
	unsigned int bits = max_phy_addr ? max_phy_addr : MTRR_DEFAULT_PHYS_BITS;

	if (bits <= 12 || bits > 64)
		bits = MTRR_DEFAULT_PHYS_BITS;
	bits -= 12;
	printf("0x%0*llx ", (int)((bits + 3) / 4),
	       (val >> 12) & ((1ULL << bits) - 1));
}

static void dump_mtrr(unsigned int cpu, int msr)
{
	unsigned long long val = 0;

	if (read_msr(cpu, msr, &val))
		printf("0x%016llx\n", val);
}

static void decode_mtrrcap(unsigned long long val)
{
	printf("0x%016llx ", val);
	printf("wc:%d ", !! ((unsigned int) val & IA32_MTRRCAP_WC));
	printf("fix:%d ", !! ((unsigned int) val & IA32_MTRRCAP_FIX));
	printf("vcnt:%u\n", (unsigned int) val & IA32_MTRRCAP_VCNT);
}

static void decode_mtrr_deftype(unsigned int cpu, int msr)
{
	unsigned long long val;
	uint8_t type;

	if (read_msr(cpu, msr, &val)) {
		printf("0x%016llx ", val);

		printf("(fixed-range flag:%d ",
			!! ((unsigned int) val & IA32_MTRR_DEFTYPE_FE));

		printf("enable flag:%d ",
			!! ((unsigned int) val & IA32_MTRR_DEFTYPE_E));

		type = val & IA32_MTRR_DEFTYPE_TYPE;
		printf("default type:0x%02x (%s))\n", type,
			type < MTRR_NUM_TYPES ? mtrr_types[type] : "?");
	}
}

static void decode_mtrr_physbase(unsigned int cpu, int msr, unsigned int max_phy_addr)
{
	unsigned long long val;
	uint8_t type;

	if (read_msr(cpu, msr, &val)) {
		printf("0x%016llx ", val);

		printf("(physbase:");
		decode_address(val, max_phy_addr);

		type = (unsigned int) val & IA32_PHYBASE_TYPE;
		printf("type: 0x%02x (%s))\n", type,
			type < MTRR_NUM_TYPES ? mtrr_types[type] : "?");
	}
}

static void decode_mtrr_physmask(unsigned int cpu, int msr, unsigned int max_phy_addr)
{
	unsigned long long val;

	if (read_msr(cpu, msr, &val)) {
		printf("0x%016llx ", val);

		printf("(physmask:");
		decode_address(val, max_phy_addr);

		printf("valid:%d)\n",
			!! ((int) val & IA32_PHYMASK_VALID));
	}
}


static const struct {
	const char *label;
	unsigned int msr;
} mtrr_fixed[] = {
	{ "MTRRfix64K_00000", 0x250 },
	{ "MTRRfix16K_80000", 0x258 },
	{ "MTRRfix16K_A0000", 0x259 },
	{ "MTRRfix4K_C0000", 0x268 },
	{ "MTRRfix4K_C8000", 0x269 },
	{ "MTRRfix4K_D0000", 0x26a },
	{ "MTRRfix4K_D8000", 0x26b },
	{ "MTRRfix4K_E0000", 0x26c },
	{ "MTRRfix4K_E8000", 0x26d },
	{ "MTRRfix4K_F0000", 0x26e },
	{ "MTRRfix4K_F8000", 0x26f },
};

void dump_mtrrs(struct cpudata *cpu)
{
	unsigned long long val = 0;
	unsigned int i;
	unsigned int vcnt;
	unsigned int max_phy_addr;

	if (!(cpu->flags_edx & (X86_FEATURE_MTRR)))
		return;

	/*
	 * If MTRR registers are not accessible like in some
	 * virtualization systems then return
	 */
	if (!read_msr(cpu->number, MSR_IA32_MTRRCAP, &val))
		return;

	printf("MTRR registers:\n");

	printf("MTRRcap (0x%x): ", MSR_IA32_MTRRCAP);
	decode_mtrrcap(val);

	max_phy_addr = cpu->phyaddr_bits;

	/* vcnt is the number of variable MTRR pairs the CPU actually has. */
	vcnt = (unsigned int)(val & IA32_MTRRCAP_VCNT);

	for (i = 0; i < vcnt * 2; i+=2) {
		printf("MTRRphysBase%u (0x%x): ", i / 2, (unsigned int) 0x200 + i);
		decode_mtrr_physbase(cpu->number, 0x200 + i, max_phy_addr);
		printf("MTRRphysMask%u (0x%x): ", i/2, (unsigned int) 0x201 + i);
		decode_mtrr_physmask(cpu->number, 0x201 + i, max_phy_addr);
	}

	if (val & IA32_MTRRCAP_FIX) {
		for (i = 0; i < ARRAY_SIZE(mtrr_fixed); i++) {
			printf("%s (0x%x): ", mtrr_fixed[i].label, mtrr_fixed[i].msr);
			dump_mtrr(cpu->number, mtrr_fixed[i].msr);
		}
	}

	printf("MTRRdefType (0x%x): ", MSR_IA32_MTRR_DEFTYPE);
	decode_mtrr_deftype(cpu->number, MSR_IA32_MTRR_DEFTYPE);

	printf("\n");
}
