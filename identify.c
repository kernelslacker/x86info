/*
 *  $Id: identify.c,v 1.28 2003/11/04 02:02:43 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include "x86info.h"

void identify(struct cpudata *cpu)
{
	unsigned long maxi, maxei, vendor;

	cpuid(cpu->number, 0, &maxi, &vendor, NULL, NULL);
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */
	cpu->maxi = maxi;

	cpuid(cpu->number, 0x80000000, &maxei, NULL, NULL, NULL);
	cpu->maxei = maxei;

	cpuid(cpu->number, 0xC0000000, &maxei, NULL, NULL, NULL);
	cpu->maxei2 = maxei;

	switch (vendor) {
	case 0x756e6547:	/* Intel */
		Identify_Intel(cpu);
		break;
	case 0x68747541:	/* AMD */
		Identify_AMD(cpu);
		break;
	case 0x69727943:	/* Cyrix */
		Identify_Cyrix(cpu);
		break;
	case 0x746e6543:	/* IDT */
		Identify_IDT(cpu);
		break;
	case 0x646f6547:	/* Natsemi Geode */
		Identify_NatSemi(cpu);
		break;
	case 0x52697365:	/* Rise This should be checked. Why 2 ? */
	case 0x65736952:	/* Rise */
		Identify_RiSE(cpu);
		break;
	case 0x20536953:	/* SiS */
		Identify_SiS(cpu);
		break;
	default:
		printf("Unknown vendor (%lx)\n", vendor);
		return;
	}
}

static void dumpregs (int cpunum, unsigned int begin, unsigned int end)
{
	unsigned int i;
	unsigned long eax, ebx, ecx, edx;

	/* Dump all the CPUID results in raw hex */
	for (i=begin; i<=end; i++) {
		cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
		printf ("eax in: 0x%08x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
	}
	printf ("\n");
}

void show_info(struct cpudata *cpu)
{
	if (silent)
		return;

	if (show_registers) {
		dumpregs (cpu->number, 0, cpu->maxi);
		if (cpu->maxei >=0x80000000)
			dumpregs (cpu->number, 0x80000000, cpu->maxei);

		if (cpu->maxei2 >=0xC0000000)
			dumpregs (cpu->number, 0xC0000000, cpu->maxei2);
	}

	switch (cpu->vendor) {
	case VENDOR_AMD:
		display_AMD_info(cpu);
		break;

	case VENDOR_CYRIX:
		display_Cyrix_info(cpu);
		break;

	case VENDOR_CENTAUR:
		display_IDT_info(cpu);
		break;

	case VENDOR_INTEL:
		display_Intel_info(cpu);
		break;

	case VENDOR_NATSEMI:
		display_NatSemi_info(cpu);
		break;

	case VENDOR_RISE:
		display_RiSE_info(cpu);
		break;

	case VENDOR_SIS:
		display_SiS_info(cpu);
		break;

	default:
		break;
	}

	if (show_connector)
		decode_connector (cpu->connector);

	if (show_urls) {
		if (cpu->datasheet_url != NULL)
			printf ("Datasheet: %s\n", cpu->datasheet_url);

		if (cpu->errata_url != NULL)
			printf ("Errata: %s\n", cpu->errata_url);

		printf ("\n");
	}

	if (!user_is_root)
		return;

	if (show_mtrr)
		dump_mtrrs(cpu);
}

