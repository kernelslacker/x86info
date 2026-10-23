/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  MTRR register dumping.
 *  TODO : decode the registers too.
 */

#include <stdio.h>
#include <x86info.h>

#define IA32_MTRRCAP_WC   (1 << 10)
#define IA32_MTRRCAP_FIX  (1 << 8)
#define IA32_MTRRCAP_VCNT 0xFF

#define IA32_MTRR_DEFTYPE_E    0x800
#define IA32_MTRR_DEFTYPE_FE   0x400
#define IA32_MTRR_DEFTYPE_TYPE 0xFF

#define IA32_PHYBASE_TYPE   0XFF
#define IA32_PHYMASK_VALID  (1 << 11)

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
	unsigned int bits = max_phy_addr ? max_phy_addr : 36;

	if (bits <= 12 || bits > 64)
		bits = 36;
	bits -= 12;
	printf("0x%0*llx ", (int)((bits + 3) / 4),
	       (val >> 12) & ((1ULL << bits) - 1));
}

static unsigned int get_max_phy_addr(struct cpudata *cpu)
{
	unsigned int value;

	if (cpu->maxei < 0x80000008)
		return 0;
	cpuid(cpu->number, 0x80000008, &value, NULL, NULL, NULL);
	return value & 0xFF;
}

static void dump_mtrr(int cpu, int msr)
{
	unsigned long long val = 0;

	if (read_msr(cpu, msr, &val) == 1)
		printf("0x%016llx\n", val);
}

static void decode_mtrrcap(unsigned long long val)
{
	printf("0x%016llx ", val);
	printf("wc:%d ", !! ((unsigned int) val & IA32_MTRRCAP_WC));
	printf("fix:%d ", !! ((unsigned int) val & IA32_MTRRCAP_FIX));
	printf("vcnt:%u\n", (unsigned int) val & IA32_MTRRCAP_VCNT);
}

static void decode_mtrr_deftype(int cpu, int msr)
{
	unsigned long long val;
	u8 type;

	if (read_msr(cpu, msr, &val) == 1) {
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

static void decode_mtrr_physbase(int cpu, int msr, unsigned int max_phy_addr)
{
	unsigned long long val;
	u8 type;

	if (read_msr(cpu, msr, &val) == 1) {
		printf("0x%016llx ", val);

		printf("(physbase:");
		decode_address(val, max_phy_addr);

		type = (unsigned int) val & IA32_PHYBASE_TYPE;
		printf("type: 0x%02x (%s))\n", type,
			type < MTRR_NUM_TYPES ? mtrr_types[type] : "?");
	}
}

static void decode_mtrr_physmask(int cpu, int msr, unsigned int max_phy_addr)
{
	unsigned long long val;

	if (read_msr(cpu, msr, &val) == 1) {
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
	if (!read_msr(cpu->number, 0xfe, &val))
		return;

	printf("MTRR registers:\n");

	printf("MTRRcap (0xfe): ");
	decode_mtrrcap(val);

	max_phy_addr = get_max_phy_addr(cpu);

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

	printf("MTRRdefType (0x2ff): ");
	decode_mtrr_deftype(cpu->number, 0x2ff);

	printf("\n");
}
