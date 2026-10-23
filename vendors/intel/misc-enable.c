/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  IA32_MISC_ENABLE decoding.
 */

#include <stdio.h>
#include <x86info.h>
#include "intel.h"

struct misc_enable_bit {
	unsigned int bit;
	const char *name;
	const char *state;	/* word printed when the bit is set */
	const char *brief;	/* label for the one-line form, if any */
};

static const struct misc_enable_bit misc_enable_bits[] = {
	{ 0,  "Fast string enable", "set", NULL },
	{ 2,  "x87 FPU Fopcode compatibility mode", "set", NULL },
	{ 3,  "Thermal monitor enable", "set", "TCC" },
	{ 4,  "Split lock disable", "set", NULL },
	{ 6,  "L3 cache disable", "set", NULL },
	{ 7,  "Performance monitoring", "available", "PerfMon" },
	{ 8,  "Suppress lock enable", "set", NULL },
	{ 9,  "Prefetch queue disable", "set", NULL },
	{ 10, "FERR# Interrupt reporting enable", "set", "FERR#" },
	{ 11, "Branch trace storage unavailable", "set", "noBTS" },
	{ 12, "Precise Event Based Sampling Unavailable", "set", "noPEBS" },
	{ 16, "Enhanced SpeedStep enable", "set", "EnhancedSpeedStep" },
};

/* One line per bit: "<name> is [un]<state>". */
void decode_misc_enable(unsigned long long val)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(misc_enable_bits); i++) {
		const struct misc_enable_bit *b = &misc_enable_bits[i];

		printf("%s is %s%s\n", b->name,
		       val & (1ULL << b->bit) ? "" : "un", b->state);
	}
}

/* "[Enabled: a b c ]" form listing the set bits that have a short label. */
void decode_misc_enable_brief(unsigned long long val)
{
	unsigned int i;

	printf(" [Enabled: ");
	for (i = 0; i < ARRAY_SIZE(misc_enable_bits); i++) {
		const struct misc_enable_bit *b = &misc_enable_bits[i];

		if (b->brief && (val & (1ULL << b->bit)))
			printf("%s ", b->brief);
	}
	printf("]\n");
}
