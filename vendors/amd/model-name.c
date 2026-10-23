/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Get CPU name string from cpuid.
 */

#include <stdio.h>
#include <string.h>
#include <amd.h>
#include <x86info.h>

/* Brand table index to name; the letter selects the model number formula. */
static const struct {
	unsigned int index;
	char letter;
	const char *fmt;
} brand_names[] = {
	{ 0x00, 0, "AMD Engineering Sample" },
	{ 0x04, 'X', "AMD Athlon(tm) 64 Processor %d00+" },
	{ 0x05, 'X', "AMD Athlon(tm) 64 X2 Dual Core Processor %d00+" },
	{ 0x08, 'X', "Mobile AMD Athlon(tm) 64 Processor %d00+" },
	{ 0x09, 'X', "Mobile AMD Athlon(tm) 64 Processor %d00+" },
	{ 0x0A, 'X', "AMD Turion(tm) 64 Mobile Technology ML-%d" },
	{ 0x0B, 'X', "AMD Turion(tm) 64 Mobile Technology MT-%d" },
	{ 0x0C, 'Y', "AMD Opteron(tm) Processor 1%d" },
	{ 0x0D, 'Y', "AMD Opteron(tm) Processor 1%d" },
	{ 0x0E, 'Y', "AMD Opteron(tm) Processor 1%d HE" },
	{ 0x0F, 'Y', "AMD Opteron(tm) Processor 1%d EE" },
	{ 0x10, 'Y', "AMD Opteron(tm) Processor 2%d" },
	{ 0x11, 'Y', "AMD Opteron(tm) Processor 2%d" },
	{ 0x12, 'Y', "AMD Opteron(tm) Processor 2%d HE" },
	{ 0x13, 'Y', "AMD Opteron(tm) Processor 2%d EE" },
	{ 0x14, 'Y', "AMD Opteron(tm) Processor 8%d" },
	{ 0x15, 'Y', "AMD Opteron(tm) Processor 8%d" },
	{ 0x16, 'Y', "AMD Opteron(tm) Processor 8%d HE" },
	{ 0x17, 'Y', "AMD Opteron(tm) Processor 8%d EE" },
	{ 0x18, 'E', "AMD Athlon(tm) 64 Processor %d00+" },
	{ 0x1D, 'X', "Mobile Athlon(tm) XP-M Processor %d00+" },
	{ 0x1E, 'X', "Mobile Athlon(tm) XP-M Processor %d00+" },
	{ 0x20, 'X', "AMD Athlon(tm) XP Processor %d00+" },
	{ 0x21, 'T', "Mobile AMD Sempron(tm) Processor %d00+" },
	{ 0x22, 'T', "AMD Sempron(tm) Processor %d00+" },
	{ 0x23, 'T', "Mobile AMD Sempron(tm) Processor %d00+" },
	{ 0x24, 'Z', "AMD Athlon(tm) 64 FX-%d Processor" },
	{ 0x26, 'T', "AMD Sempron(tm) Processor %d00+" },
	{ 0x29, 'R', "Dual Core AMD Opteron(tm) Processor 1%d SE" },
	{ 0x2A, 'R', "Dual Core AMD Opteron(tm) Processor 2%d SE" },
	{ 0x2B, 'R', "Dual Core AMD Opteron(tm) Processor 8%d SE" },
	{ 0x2C, 'R', "Dual Core AMD Opteron(tm) Processor 1%d" },
	{ 0x2D, 'R', "Dual Core AMD Opteron(tm) Processor 1%d" },
	{ 0x2E, 'R', "Dual Core AMD Opteron(tm) Processor 1%d HE" },
	{ 0x2F, 'R', "Dual Core AMD Opteron(tm) Processor 1%d EE" },
	{ 0x30, 'R', "Dual Core AMD Opteron(tm) Processor 2%d" },
	{ 0x31, 'R', "Dual Core AMD Opteron(tm) Processor 2%d" },
	{ 0x32, 'R', "Dual Core AMD Opteron(tm) Processor 2%d HE" },
	{ 0x33, 'R', "Dual Core AMD Opteron(tm) Processor 2%d EE" },
	{ 0x34, 'R', "Dual Core AMD Opteron(tm) Processor 8%d" },
	{ 0x35, 'R', "Dual Core AMD Opteron(tm) Processor 8%d" },
	{ 0x36, 'R', "Dual Core AMD Opteron(tm) Processor 8%d HE" },
	{ 0x37, 'R', "Dual Core AMD Opteron(tm) Processor 8%d EE" },
	{ 0x38, 'R', "Dual Core AMD Opteron(tm) Processor 1%d" },
	{ 0x39, 'R', "Dual Core AMD Opteron(tm) Processor 2%d" },
	{ 0x3A, 'R', "Dual Core AMD Opteron(tm) Processor 8%d" },
};

static int model_number(char letter, int nn)
{
	switch (letter) {
	case 'X': return 22 + nn;
	case 'Y': return 38 + (2 * nn);
	case 'Z': return 24 + nn;
	case 'T': return 24 + nn;
	case 'R': return 45 + (5 * nn);
	case 'E': return 9 + nn;
	default: return 0;
	}
}

/* Broken BIOS? Try to determine the model name ourselves.  */
void fix_amd_model_name(struct cpudata *cpu, char *cp, size_t len)
{
	unsigned int bid, ebid;
	cpuid(cpu->number, 0x00000001, NULL, &bid, NULL, NULL);
	bid &= 0xff;
	cpuid(cpu->number, 0x80000001, NULL, &ebid, NULL, NULL);

	/* 8BitBrandId == 0, BrandId == non-zero */
	if (bid == 0 && ebid != 0) {
		unsigned int index = (ebid >> 6) & 0x3f;
		int nn = ebid & 0x3f;
		const char *name = "AMD Processor model unknown";
		int number = 0;
		const char *sub;
		unsigned int i;

		for (i = 0; i < ARRAY_SIZE(brand_names); i++) {
			if (brand_names[i].index == index) {
				name = brand_names[i].fmt;
				number = model_number(brand_names[i].letter, nn);
				break;
			}
		}

		/* substitute the model number at the "%d" in the table entry */
		sub = number ? strstr(name, "%d") : NULL;

		if (sub)
			(void)snprintf(cp, len, "%.*s%d%s",
				       (int)(sub - name), name, number, sub + 2);
		else
			(void)snprintf(cp, len, "%s", name);
	}
}
