/*
 *  $Id: info.c,v 1.10 2004/10/06 21:19:05 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Intel specific parts
 *
 * References:
 *	http://developer.intel.com/
 *	http://microcodes.sourceforge.net/CPUID.htm
 *
 */

#include <stdio.h>
#include <string.h>
#include "../x86info.h"
#include "Intel.h"

/* Decode Pentium III CPU serial number */
void decode_serial_number(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;
	unsigned long signature;

	if (cpu->family!=6)
		return;

	switch (cpu->model) {
	case 7:
	case 8:
	case 10:
	case 11:
		if (cpu->maxi < 3)
			return;

		cpuid (cpu->number, 1, &eax, NULL, NULL, NULL);
		signature = eax;

		cpuid (cpu->number, 3, &eax, &ebx, &ecx, &edx);
		printf ("Processor serial: ");
		printf ("%04lX", signature >> 16);
		printf ("-%04lX", signature & 0xffff);
		printf ("-%04lX", edx >> 16);
		printf ("-%04lX", edx & 0xffff);
		printf ("-%04lX", ecx >> 16);
		printf ("-%04lX\n", ecx & 0xffff);
		return;
	default:
		return;
	}
}

void display_Intel_info (struct cpudata *cpu)
{
	unsigned long ebx;

	printf ("Family: %u Model: %u Stepping: %u Type: %u Brand: %u\n",
		cpu->family, cpu->model, cpu->stepping, cpu->type, cpu->brand);
	printf ("CPU Model: %s\n", cpu->name);

	/* Pentium4 and Banias have cpu name. */
	if (cpu->family == 0xF || (cpu->family == 6 && cpu->model == 9))
		get_model_name (cpu);

	decode_feature_flags (cpu);

	if (show_msr) {
		if (cpu->family==0xf)
			dump_p4_MSRs(cpu);
		if (cpu->family==0x6 && (cpu->model == 9 || cpu->model == 13))
			dump_centrino_MSRs(cpu);
	}

	decode_Intel_caches(cpu, 1);

	decode_serial_number(cpu);

	if (show_eblcr) {
		if (cpu->family == 6 && cpu->model >= 3) {
			unsigned long long eblcr;
			read_msr(cpu->number, 0x2A, &eblcr);
			interpret_eblcr(eblcr);
		}
	}

	/* FIXME: Bit test for MCA here!*/
	if (show_bluesmoke)
		decode_Intel_bluesmoke(cpu->number, cpu->family);

	/* Hyper-Threading Technology */
	if (cpu->flags_edx & (1 << 28)) {
		int nr_ht;
		int phys_id;
		cpuid(cpu->number, 1, NULL, &ebx, NULL, NULL);

		nr_ht = (ebx >> 16) & 0xFF;
		phys_id = (ebx >> 24) & 0xFF;

		if (!nr_ht)
			nr_ht = 1;
		printf ("The physical package supports "
			"%d logical processors \n\n", nr_ht);
	}

	if (show_microcode)
		decode_microcode(cpu);
}
