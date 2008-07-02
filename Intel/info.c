/*
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Intel specific parts
 *
 * References:
 *	http://developer.intel.com/
 *	http://microcodes.sourceforge.net/CPUID.htm
 */

#include <stdio.h>
#include <string.h>
#include "../x86info.h"
#include "Intel.h"

/* Decode Pentium III CPU serial number */
void decode_serial_number(struct cpudata *cpu)
{
	char *p = cpu->serialno;
	unsigned int eax, ebx, ecx, edx;
	unsigned int signature;

	if (cpu->maxi < 3)
		return;

	cpuid(cpu->number, 1, &eax, NULL, NULL, NULL);
	signature = eax;

	cpuid(cpu->number, 3, &eax, &ebx, &ecx, &edx);
	p += sprintf(p, "%04X", signature >> 16);
	p += sprintf(p, "-%04X", signature & 0xffff);
	p += sprintf(p, "-%04X", edx >> 16);
	p += sprintf(p, "-%04X", edx & 0xffff);
	p += sprintf(p, "-%04X", ecx >> 16);
	p += sprintf(p, "-%04X\n", ecx & 0xffff);

	printf("Processor serial: %s\n", cpu->serialno);
}

void display_Intel_info(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;

	printf("Type: %u (", cpu->type);
	switch (cpu->type) {
	case 0:	printf("Original OEM");
		break;
	case 1:	printf("Overdrive");
		break;
	case 2:	printf("Dual-capable");
		break;
	case 3:	printf("Reserved");
		break;
	}
	printf(")\tBrand: %u\n", cpu->brand);

	if (show_msr) {
		if (cpu->family == 0xf)
			dump_p4_MSRs(cpu);
		if (cpu->family == 0x6 && (cpu->model == 9 || cpu->model == 13))
			dump_centrino_MSRs(cpu);
	}

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

	/* Stash the APIC ID for this logical CPU */
	cpuid(cpu->number, 1, NULL, &ebx, NULL, NULL);
	cpu->apicid = ebx >> 24;
	printf("APIC ID: %x\n", cpu->apicid);

	/* Figure out number of cores on this package. */
	cpu->nr_cores = 1;
	if (cpu->maxi >= 4) {
		cpuid4(cpu->number, 0, &eax, &ebx, &ecx, &edx);
		if (eax & 0x1f)
			cpu->nr_cores = ((eax >> 26) + 1);
printf("NUMBER CORES=%d eax=%x\n", cpu->nr_cores, eax);
	}

	/* Hyper-Threading Technology */
	if (cpu->flags_edx & (1 << 28)) {
		int nr_ht;
		int phys_id;
		cpuid(cpu->number, 1, NULL, &ebx, NULL, NULL);

		nr_ht = (ebx >> 16) & 0xFF;
		phys_id = (ebx >> 24) & 0xFF;

		if (!nr_ht)
			nr_ht = 1;
		printf("The physical package supports "
			"%d logical processors \n\n", nr_ht);
	}

	if (show_microcode)
		decode_microcode(cpu);
}
