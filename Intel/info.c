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
	printf(")\tBrand: %u (", cpu->brand);
	switch (cpu->brand) {
	case 0:		printf("Unsupported");
			break;
	case 1:
	case 0xA:
	case 0x14:	printf("Intel® Celeron® processor");
			break;
	case 2:
	case 4:		printf("Intel® Pentium® III processor");
			break;
	case 3:		if (tuple(cpu) == 0x6b1)
				printf("Intel® Celeron® processor");
			else
				printf("Intel® Pentium® III Xeon processor");
			break;
	case 6:		printf("Mobile Intel® Pentium® III processor");
			break;
	case 7:
	case 0xF:
	case 0x17:	printf("Mobile Intel® Celeron® processor");
			break;
	case 8:		if (tuple(cpu) >= 0xf13)
				printf("Intel® genuine processor");
			else
			printf("Intel® Pentium® 4 processor");
			break;
	case 9:		printf("Intel® Pentium® 4 processor");
			break;

	case 0xb:	if (tuple(cpu) <0xf13)
				printf("Intel® Xeon processor MP");
			else
				printf("Intel® Xeon processor");
			break;
	case 0xc:	printf("Intel® Xeon processor");
			break;
	case 0xe:	if (tuple(cpu) <0xf13)
				printf("Intel® Xeon processor");
			else
				printf("Mobile Intel® Pentium® 4 processor-M");
			break;
	case 0x11:
	case 0x15:	printf("Mobile Genuine Intel® processor");
			break;
	case 0x12:	printf("Intel® Celeron® M processor");
			break;
	case 0x13:	printf("Mobile Intel® Celeron® processor");
			break;
	case 0x16:	printf("Intel® Pentium® M processor");
			break;
	default:	printf("unknown");
			break;
	}
	printf(")\n");

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

	if (show_microcode)
		decode_microcode(cpu);
}
