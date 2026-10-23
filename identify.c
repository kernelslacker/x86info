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

void get_cpu_info_basics(struct cpudata *cpu)
{
	static int unknown_vendor_reported;

	unsigned int maxi, maxei, vendor, vendor_edx, address_bits;
	unsigned int eax;

	cpuid(cpu->number, 0, &maxi, &vendor, NULL, &vendor_edx);
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */
	cpu->cpuid_level = maxi;

	cpu->vendor = vendor_from_cpuid(vendor, vendor_edx);
	if (cpu->vendor == VENDOR_UNKNOWN && !unknown_vendor_reported) {
		unsigned int ebx, ecx, edx;

		cpuid(cpu->number, 0, NULL, &ebx, &ecx, &edx);
		printf("Unknown vendor (%.4s%.4s%.4s)\n",
		       (char *)&ebx, (char *)&edx, (char *)&ecx);
		unknown_vendor_reported = 1;
	}

	if (maxi < 1)
		return;

	/* Everything that supports cpuid supports these. */
	cpuid(cpu->number, 1, &eax, NULL, NULL, NULL);
	cpu->stepping = eax & 0xf;
	cpu->model = (eax >> 4) & 0xf;
	cpu->family = (eax >> 8) & 0xf;
	if (cpu->family == 0xf)
		cpu->efamily = (eax >> 20) & 0xff;
	/* AMD only has an extended model at family 0xf */
	if (cpu->family == 0xf ||
	    (cpu->family == 6 && cpu->vendor != VENDOR_AMD))
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
}

void cpu_name_append(struct cpudata *cpu, const char *str)
{
	size_t len = strlen(cpu->name);

	if (len < CPU_NAME_LEN - 1)
		snprintf(cpu->name + len, CPU_NAME_LEN - len, "%s", str);
}

void identify(struct cpudata *cpu)
{
	switch (cpu->vendor) {
	case VENDOR_INTEL:
		identify_intel(cpu);
		break;
	case VENDOR_AMD:
		identify_amd(cpu);
		break;
	case VENDOR_CYRIX:
		identify_cyrix(cpu);
		break;
	case VENDOR_CENTAUR:
		identify_centaur(cpu);
		break;
	case VENDOR_NATSEMI:
	case VENDOR_RISE:
	case VENDOR_SIS:
		identify_misc(cpu);
		break;
	case VENDOR_TRANSMETA:
	case VENDOR_UNKNOWN:
		break;
	}
}

/* What identifies the CPU model, shared by every core type. */
void show_header(struct cpudata *cpu)
{
	if ((cpu->efamily != 0) || (cpu->emodel != 0))
		printf("Extended Family: %u Extended Model: %u ",
			cpu->efamily, cpu->emodel);
	printf("Family: %u Model: %u Stepping: %u\n",
	       cpu->family, model(cpu), cpu->stepping);
	if (cpu->vendor == VENDOR_INTEL)
		display_basic_intel_info(cpu);
	printf("CPU Model (x86info's best guess): %s\n", cpu->name);
	display_model_name(cpu);
	if (cpu->vendor == VENDOR_AMD)
		display_amd_package(cpu);
	printf("\n");
}

void show_info(struct cpudata *cpu, int header)
{
	if (header)
		show_header(cpu);
	if (cpu->core_type)
		printf("Core type: %s\n\n", core_type_name(cpu));

	switch (cpu->vendor) {
	case VENDOR_AMD:
		display_amd_info(cpu);
		break;

	case VENDOR_CYRIX:
		display_cyrix_info(cpu);
		break;

	case VENDOR_CENTAUR:
		display_centaur_info(cpu);
		break;

	case VENDOR_INTEL:
		display_extended_intel_info(cpu);
		break;

	case VENDOR_NATSEMI:
	case VENDOR_RISE:
	case VENDOR_SIS:
	case VENDOR_TRANSMETA:
	case VENDOR_UNKNOWN:
	default:
		break;
	}
}
