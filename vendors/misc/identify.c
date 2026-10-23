/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  NatSemi, Rise and SiS specific parts.
 */
#include <stdio.h>
#include <x86info.h>

struct misc_cpu {
	enum vendor vendor;
	unsigned int tuple;
	unsigned int mask;
	const char *name;
};

static const struct misc_cpu misc_cpus[] = {
	{ VENDOR_NATSEMI, 0x540, 0xff0, "Geode GX1" },
	{ VENDOR_RISE, 0x500, 0xff0, "iDragon (0.25um)" },
	{ VENDOR_RISE, 0x520, 0xff0, "iDragon (0.18um)" },
	{ VENDOR_RISE, 0x580, 0xff0, "iDragon II (0.25um)" },
	{ VENDOR_RISE, 0x590, 0xff0, "iDragon II (0.18um)" },
	{ VENDOR_SIS, 0x505, 0xfff, "SiS55x" },
};

void identify_misc(struct cpudata *cpu)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(misc_cpus); i++) {
		const struct misc_cpu *m = &misc_cpus[i];

		if (m->vendor == cpu->vendor && (tuple(cpu) & m->mask) == m->tuple) {
			add_to_cpuname(m->name);
			return;
		}
	}
	add_to_cpuname("Unknown CPU");
}
