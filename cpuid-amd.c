/*
 *  $Id: cpuid-amd.c,v 1.12 2001/03/17 17:15:49 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific information
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "x86info.h"

extern int show_registers;
extern int show_cacheinfo;
extern int show_msr;

static void do_assoc(unsigned long assoc)
{
	if ((assoc & 0xff) == 255)
		printf ("Fully");
	else
		printf ("%ld-way", assoc);
	printf (" associative. ");
}

static void decode_AMD_cacheinfo(int cpunum, unsigned long maxei)
{
	unsigned long eax, ebx, ecx, edx;

	if (maxei >= 0x80000005) {
		/* TLB and cache info */
		cpuid (cpunum, 0x80000005, &eax, &ebx, &ecx, &edx);

		printf ("Instruction TLB: ");
		do_assoc ((ebx >> 8) & 0xff);
		printf ("%ld entries.\n", ebx & 0xff);

		printf ("Data TLB: ");
		do_assoc (ebx>>24);
		printf ("%ld entries.\n", (ebx >> 16) & 0xff);

		printf ("L1 Data cache:\n\t");
		printf ("Size: %ldKb\t", ecx >> 24);
		do_assoc ((ecx >> 16) & 0xff);
		printf ("\n\t");
		printf ("lines per tag=%ld\t", (ecx >> 8) & 0xff);
		printf ("line size=%ld bytes.\n", ecx & 0xff);

		printf ("L1 Instruction cache:\n\t");
		printf ("Size: %ldKb\t", edx >> 24);
		do_assoc ((edx >> 16) & 0xff);
		printf ("\n\t");
		printf ("lines per tag=%ld\t", (edx >> 8) & 0xff);
		printf ("line size=%ld bytes.\n", edx & 0xff);
	}

	/* check K6-III (and later?) on-chip L2 cache size */
	if (maxei >= 0x80000006) {
		cpuid (cpunum, 0x80000006, &eax, &ebx, &ecx, &edx);
		printf ("L2 (on CPU) cache:\n\t");
		printf ("Size %ldKb\t", ecx >> 16);
		do_assoc ((ecx >> 12) & 0x0f);
		printf ("\n\t");
		printf ("lines per tag=%ld\t", (ecx >> 8) & 0x0f);
		printf ("line size=%ld bytes.\n", ecx & 0xff);
	}
	printf ("\n");
}


static void dump_extended_AMD_regs(int cpunum, unsigned long maxei)
{
	unsigned long eax, ebx, ecx, edx;
	unsigned int i;

	/* Dump extended info in raw hex */
	for (i = 0x80000000; i <= maxei; i++) {
		cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
		printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
	}
	printf ("\n");
}

static void dump_athlon_MSR(int cpunum)
{
	if (getuid()!=0) {
		printf ("Need to be root to access MSRs.\n");
		return;
	}

	printf("\t\t\t\t31       23       15       7 \n");
	dumpmsr(cpunum, 0x2A);
	dumpmsr(cpunum, 0xC0000080);
	dumpmsr(cpunum, 0xC0010010);
	dumpmsr(cpunum, 0xC0010015);
	dumpmsr(cpunum, 0xC001001B);
	printf ("\n");
}


void doamd (int cpunum, unsigned int maxi, struct cpudata *cpu)
{
	unsigned int i;
	unsigned long maxei, eax, ebx, ecx, edx;

	cpu->vendor = VENDOR_AMD;

	cpuid (cpunum, 0x80000000, &maxei, NULL, NULL, NULL);
	if (show_registers && (maxei != 0))
		dump_extended_AMD_regs(cpunum, maxei);

	if (maxi >= 0x00000001) {
		cpuid (cpunum, 0x00000001, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;

		printf ("Family: %d Model: %d [", cpu->family, cpu->model);
		switch (cpu->family) {
		case 4:
			switch (cpu->model) {
			case 3:
				printf ("80486DX2");
				break;
			case 7:
				printf ("80486DX2WB");
				break;
			case 8:
				printf ("80486DX4");
				break;
			case 9:
				printf ("80486DX4WB");
				break;
			case 0xe:
				printf ("5x86");
				break;
			case 0xf:
				printf ("5x86WB");
				break;
			default:
				printf ("Unknown CPU");
				break;
			}
			break;

		case 5:
			switch (cpu->model) {
			case 0:
				printf ("SSA5 (PR75/PR90/PR100)");
				break;
			case 1:
				printf ("K5 (PR120/PR133)");
				break;
			case 2:
				printf ("K5 (PR166)");
				break;
			case 3:
				printf ("K5 (PR200)");
				break;
			case 6:
				printf ("K6 (0.30 um)");
				break;
			case 7:
				printf ("K6 (0.25 um)");
				break;
			case 8:
				printf ("K6-2");
				if (cpu->stepping >= 8)
					printf (" (CXT core)");
				break;
			case 9:
				printf ("K6-III");
				break;
			case 12:
				printf ("K6-2+ (0.18um)");
				break;
			case 13:
				printf ("K6-3+ (0.18um)");
				break;
			default:
				printf ("Unknown CPU");
				break;
			}
			break;

		case 6:
			switch (cpu->model) {
			case 0:
				printf ("K7 ES");
				break;
			case 1:
				printf ("Athlon (0.25um");
				switch (cpu->stepping) {
				case 1:
					printf (" Rev C1");
					break;
				case 2:
					printf (" Rev C2");
					break;
				}
				break;
			case 2:
				printf ("Athlon (0.18um");
				switch (cpu->stepping) {
				case 1:
					printf (" Rev A1");
					break;
				case 2:
					printf (" Rev A2");
					break;
				}
				break;
			case 3:
				printf ("Duron");
				switch (cpu->stepping) {
				case 0:
					printf (" Rev A0");
					break;
				case 1:
					printf (" Rev A2");
					break;
				}
				break;
			case 4:
				printf ("Thunderbird");
				switch (cpu->stepping) {
				case 0:
					printf (" Rev A1");
					break;
				case 1:
					printf (" Rev A2");
					break;
				case 2:
					printf (" Rev A4-A8");
					break;
				case 3:
					printf (" Rev A9");
					break;
				}
				break;
			default:
				printf ("Unknown CPU");
				break;
			}
			break;
		}
		printf ("]\n");
		if (show_msr)
			dump_athlon_MSR(cpunum);
	}

	if (maxei >= 0x80000001) {
		cpuid (cpunum, 0x80000001, &eax, &ebx, &ecx, &edx);
		decode_feature_flags (cpu, edx);
	}

	if (maxei >= 0x80000002) {
		/* Processor identification string */
		char namestring[49], *cp;
		unsigned int j;
		cp = namestring;
		for (j = 0x80000002; j <= 0x80000004; j++) {
			cpuid (cpunum, j, &eax, &ebx, &ecx, &edx);

			for (i = 0; i < 4; i++)
				*cp++ = eax >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = ebx >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = ecx >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = edx >> (8 * i);
		}
		printf ("Processor name string: %s\n\n", namestring);
	}

	if (show_cacheinfo)
		decode_AMD_cacheinfo(cpunum, maxei);
}