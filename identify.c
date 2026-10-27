/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include <amd.h>
#include <intel.h>
#include <centaur.h>
#include <cyrix.h>

int get_cpu_info_basics(struct cpudata *cpu)
{
	static int unknown_vendor_reported;

	unsigned int maxi, maxei, vendor, vendor_ecx, vendor_edx, address_bits;
	unsigned int eax;

	if (cpuid(cpu->number, 0, &maxi, &vendor, &vendor_ecx, &vendor_edx))
		return -1;
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */
	cpu->cpuid_level = maxi;

	cpu->vendor = vendor_from_cpuid(vendor, vendor_edx);
	if (cpu->vendor == VENDOR_UNKNOWN && !unknown_vendor_reported) {
		printf("Unknown vendor (%.4s%.4s%.4s)\n",
		       (char *)&vendor, (char *)&vendor_edx, (char *)&vendor_ecx);
		unknown_vendor_reported = 1;
	}

	if (maxi < 1)
		return 0;

	/* Everything that supports cpuid supports these. */
	if (cpuid(cpu->number, 1, &eax, NULL, NULL, NULL))
		return -1;
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;
	if (cpu->family == 0xf)
		cpu->efamily = (eax >> 20) & 0xff;
	/* AMD only has an extended model at family 0xf */
	if (cpu->family == 0xf ||
	    (cpu->family >= 6 && cpu->vendor != VENDOR_AMD))
		cpu->emodel = (eax >> 16) & 0xf;

	cpuid(cpu->number, 0x80000000, &maxei, NULL, NULL, NULL);
	cpu->maxei = sane_max_leaf(maxei, 0x80000000);

	cpuid(cpu->number, 0xC0000000, &maxei, NULL, NULL, NULL);
	cpu->maxei2 = sane_max_leaf(maxei, 0xC0000000);

	if (cpu->maxei >= 0x80000008) {
		cpuid(cpu->number, 0x80000008, &address_bits, NULL, NULL, NULL);
		cpu->phyaddr_bits = address_bits & 0xFF;
		cpu->viraddr_bits = (address_bits >> 8) & 0xFF;
	}
	return 0;
}

void cpu_name_append(struct cpudata *cpu, const char *str)
{
	size_t len = strlen(cpu->name);

	if (len < CPU_NAME_LEN - 1)
		snprintf(cpu->name + len, CPU_NAME_LEN - len, "%s", str);
}

/* What each vendor provides; vendors without an entry have nothing to run. */
struct vendor_ops {
	void (*identify)(struct cpudata *cpu);
	void (*show_info)(struct cpudata *cpu);
	void (*show_cache)(struct cpudata *cpu);
};

static void show_intel_cache(struct cpudata *cpu)
{
	decode_intel_caches(cpu, 1);
}

static const struct vendor_ops vendor_ops[] = {
	[VENDOR_AMD] = {
		.identify = identify_amd,
		.show_info = display_amd_info,
		.show_cache = decode_amd_cacheinfo,
	},
	[VENDOR_CENTAUR] = {
		.identify = identify_centaur,
		.show_info = display_centaur_info,
	},
	[VENDOR_CYRIX] = {
		.identify = identify_cyrix,
		.show_info = display_cyrix_info,
	},
	[VENDOR_INTEL] = {
		.identify = identify_intel,
		.show_info = display_extended_intel_info,
		.show_cache = show_intel_cache,
	},
	[VENDOR_NATSEMI] = { .identify = identify_misc },
	[VENDOR_RISE] = { .identify = identify_misc },
	[VENDOR_SIS] = { .identify = identify_misc },
};

static const struct vendor_ops *ops_for(const struct cpudata *cpu)
{
	if ((unsigned int)cpu->vendor >= ARRAY_SIZE(vendor_ops))
		return NULL;
	return &vendor_ops[cpu->vendor];
}

void identify(struct cpudata *cpu)
{
	const struct vendor_ops *ops = ops_for(cpu);

	if (ops && ops->identify)
		ops->identify(cpu);
}

void show_cache_info(struct cpudata *cpu)
{
	const struct vendor_ops *ops = ops_for(cpu);

	if (ops && ops->show_cache)
		ops->show_cache(cpu);
}

/* What identifies the CPU model, shared by every core type. */
void show_header(struct cpudata *cpu)
{
	size_t len;

	if ((cpu->efamily != 0) || (cpu->emodel != 0))
		printf("Extended Family: %u Extended Model: %u ",
			cpu->efamily, cpu->emodel);
	printf("Family: %u Model: %u Stepping: %u\n",
	       cpu->family, model(cpu), cpu->stepping);
	if (cpu->vendor == VENDOR_INTEL)
		display_basic_intel_info(cpu);
	len = strlen(cpu->name);
	while (len > 0 && cpu->name[len - 1] == ' ')
		len--;
	if (len)
		printf("CPU Model (x86info's best guess): %.*s\n", (int)len, cpu->name);
	display_model_name(cpu);
	if (cpu->vendor == VENDOR_AMD)
		display_amd_package(cpu);
	printf("\n");
}

void show_info(struct cpudata *cpu, int header)
{
	const struct vendor_ops *ops = ops_for(cpu);

	if (header)
		show_header(cpu);
	if (cpu->core_type)
		printf("Core type: %s\n\n", core_type_name(cpu));

	if (ops && ops->show_info)
		ops->show_info(cpu);
}
