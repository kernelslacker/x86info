/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Dump machine check registers.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <x86info.h>
#include <amd.h>

#define MCG_CAP 0x0179
#define MCG_STATUS 0x17a
#define MCG_CTL 0x17b

#define MC_CTL 0x0400
#define MC_STATUS 0x0401
#define MC_ADDR 0x402
#define MC_MISC 0x403

struct mc_bit {
	unsigned int bit;
	const char *label;
};

struct k7_unit {
	const char *name;
	const struct mc_bit *bits;
	unsigned int nr_bits;
};

static const struct mc_bit k7_dc_bits[] = {
	{ 0, "ECC 1 bit error reporting" },
	{ 1, "ECC multi bit error reporting" },
	{ 2, "Data cache data parity" },
	{ 3, "Data cache main tag parity" },
	{ 4, "Data cache snoop tag parity" },
	{ 5, "L1 TLB parity" },
	{ 6, "L2 TLB parity" },
};

static const struct mc_bit k7_ic_bits[] = {
	{ 0, "ECC 1 bit error reporting" },
	{ 1, "ECC multi bit error reporting" },
	{ 2, "Instruction cache data parity" },
	{ 3, "IC main tag parity" },
	{ 4, "IC snoop tag parity" },
	{ 5, "L1 TLB parity" },
	{ 6, "L2 TLB parity" },
	{ 7, "Predecode array parity" },
	{ 8, "Target selector parity" },
	{ 9, "Read data error" },
};

static const struct mc_bit k7_bu_bits[] = {
	{ 0, "External L2 tag parity error" },
	{ 1, "L2 partial tag parity error" },
	{ 2, "System ECC TLB reload error" },
	{ 3, "L2 ECC TLB reload error" },
	{ 4, "L2 ECC K7 deallocate" },
	{ 5, "L2 ECC probe deallocate" },
	{ 6, "System datareaderror reporting" },
};

static const struct mc_bit k7_ls_bits[] = {
	{ 0, "Read data error enable (loads)" },
	{ 1, "Read data error enable (stores)" },
};

#define K7_UNIT(n, b) { n, b, ARRAY_SIZE(b) }

/* Indexed by MCG_CTL bit, which is also the MC bank number. */
static const struct k7_unit k7_units[] = {
	K7_UNIT("Data cache", k7_dc_bits),
	K7_UNIT("Instruction cache", k7_ic_bits),
	K7_UNIT("Bus unit", k7_bu_bits),
	K7_UNIT("Load/Store unit", k7_ls_bits),
};

static void print_ctl_bits(const struct mc_bit *bits, unsigned int nr,
			   unsigned long long ctl)
{
	unsigned int i;

	for (i = 0; i < nr; i++)
		printf("  %s %sabled\n", bits[i].label,
		       ctl & (1ULL << bits[i].bit) ? "en" : "dis");
}

void decode_athlon_machine_check(int cpunum, unsigned int family)
{
	unsigned long long val, val2;
	unsigned int banks, i, ctlp = 0;

	if (!user_is_root)
		return;

	if (read_msr(cpunum, MCG_CAP, &val) != 1)
		return;

	ctlp = val & (1<<8);
	if (ctlp == 0)
		printf("Erk, MCG_CTL not present! :%016llx:\n", val);

	banks = val & 0xff;

	printf("Number of reporting banks : %u\n\n", banks);

	if (read_msr(cpunum, MCG_STATUS, &val) == 1) {
		if (val != 0) {
			printf("MCG_STATUS: ");
			dumpmsr_bin (cpunum, MCG_STATUS, 64);
		}
	}

	/* The per-unit MCG_CTL bits below are the K7/K8 layout. */
	if (ctlp && family >= 0x10 && read_msr(cpunum, MCG_CTL, &val) == 1) {
		printf("MCG_CTL: 0x%016llx\n", val);
	} else if (ctlp && read_msr(cpunum, MCG_CTL, &val) == 1) {
		printf("MCG_CTL:\n");

		for (i = 0; i < ARRAY_SIZE(k7_units); i++) {
			const struct k7_unit *u = &k7_units[i];

			printf(" %s check %sabled\n", u->name, val & (1<<i) ? "en" : "dis");
			if (!(val & (1<<i)) || banks <= i)
				continue;
			if (read_msr(cpunum, MC_CTL + 4 * i, &val2) == 1)
				print_ctl_bits(u->bits, u->nr_bits, val2);
		}
	}
	printf("\n");


	for (i=0; i<banks; i++) {
		printf("Bank: %u (0x%x)\n", i, (unsigned int)MC_CTL+i*4);
		printf("MC%uCTL:    ", i);
		dumpmsr_bin (cpunum, MC_CTL+i*4, 64);
		printf("MC%uSTATUS: ", i);
		dumpmsr_bin (cpunum, MC_STATUS+i*4, 64);
		printf("MC%uADDR:   ", i);
		dumpmsr_bin (cpunum, MC_ADDR+i*4, 64);
		printf("MC%uMISC:   ", i);
		dumpmsr_bin (cpunum, MC_MISC+i*4, 64);
		printf("\n");
	}
}
