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

void display_Intel_info (struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	printf ("Family: %d Model: %d Stepping: %d Type: %d Brand: %d\n",
		cpu->family, cpu->model, cpu->stepping, cpu->type, cpu->brand);
	printf ("CPU Model: %s\n", cpu->name);

	/* Pentium4 and Banias have cpu name. */
	if (cpu->family == 0xF || (cpu->family == 6 && cpu->model == 9))
		get_model_name (cpu);

	decode_feature_flags (cpu);

	if (show_msr) {
		if (cpu->family==0xf)
			dump_p4_MSRs(cpu);
		if (cpu->family==0x6 && cpu->model==9)
			dump_centrino_MSRs(cpu);
	}

	//show_Intel_caches(cpu);
	decode_Intel_caches(cpu, 1);

	if (cpu->maxi >= 3) {
		/* Pentium III CPU serial number */
		unsigned long signature;
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
	}

	if (show_eblcr) {
		if (cpu->family == 6 && cpu->model >= 3) {
			unsigned long long eblcr;
			read_msr (cpu->number, 0x2A, &eblcr);
			interpret_eblcr(eblcr);
		}
	}

	/* FIXME: Bit test for MCA here!*/
	if (show_bluesmoke)
		decode_Intel_bluesmoke(cpu->number, cpu->family);

	/* Hyper-Threading Technology */
	if (cpu->flags & (1 << 28)) {
		int nr_ht = (cpu->bflags >> 16) & 0xFF;
		int phys_id = (cpu->bflags >> 24) & 0xFF;
		if (!nr_ht)
			nr_ht = 1;
		printf ("The physical package supports "
			"%d logical processors \n\n", nr_ht);
	}

	if (show_microcode)
		decode_microcode(cpu);
}
