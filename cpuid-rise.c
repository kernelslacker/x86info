/*
 *  $Id: cpuid-rise.c,v 1.6 2001/08/10 11:34:57 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include "x86info.h"

extern int show_cacheinfo;
extern int show_flags;
extern int show_registers;

void Identify_Rise (unsigned int maxi, unsigned int maxei, struct cpudata *cpu)
{
	unsigned int i;
	unsigned long eax, ebx, ecx, edx;
	cpu->vendor = VENDOR_RISE;

	/* Do standard stuff */
	if (maxi >= 1) {
		cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;

		switch (cpu->family) {
		case 5:
			switch (cpu->model) {
				case 0:		sprintf (cpu->name, "%s", "iDragon (0.25um)");		break;
				case 2:		sprintf (cpu->name, "%s", "iDragon (0.18um)");		break;
				case 8:		sprintf (cpu->name, "%s", "iDragon II (0.25um)");	break;
				case 9:		sprintf (cpu->name, "%s", "iDragon II (0.18um)");	break;
				default:	sprintf (cpu->name, "%s", "Unknown CPU");			break;
			}
			break;

		default:
			printf ("Unknown CPU");
			break;
		}

	}

	/* Check for presence of extended info */
	if (maxei == 0)
		return;

	if (maxei >= 0x80000002) {
		/* Processor identification string */
		char namestring[49], *cp;
		unsigned int j;
		cp = namestring;
		for (j = 0x80000002; j <= 0x80000004; j++) {
			cpuid (cpu->number, j, &eax, &ebx, &ecx, &edx);

			for (i = 0; i < 4; i++)
				*cp++ = eax >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = ebx >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = ecx >> (8 * i);
			for (i = 0; i < 4; i++)
				*cp++ = edx >> (8 * i);
		}
		*cp++ = '\n';
		sprintf (cpu->name, "%s", namestring);
	}
}


void display_Rise_info(unsigned int maxi, unsigned int maxei, struct cpudata *cpu)
{
	unsigned int i;
	unsigned long eax, ebx, ecx, edx;
	static char *x86_cap_flags[] = {
		"FPU    Floating Point Unit",
		"VME    Virtual 8086 Mode Enhancements",
		"DE     Debugging Extensions",
		"PSE    Page Size Extensions",
		"TSC    Time Stamp Counter",
		"MSR    Model Specific Registers",
		"PAE    Physical Address Extension",
		"MCE    Machine Check Exception",
		"CX8    COMPXCHG8B Instruction",
		"APIC   On-chip Advanced Programmable Interrupt Controller present and enabled",
		"10     Reserved",
		"SEP    Fast System Call",
		"MTRR   Memory Type Range Registers",
		"PGE    PTE Global Flag",
		"MCA    Machine Check Architecture",
		"CMOV   Conditional Move and Compare Instructions",
		"FGPAT  Page Attribute Table",
		"PSE-36 36-bit Page Size Extension",
		"PN     Processor Serial Number present and enabled",
		"19     reserved",
		"20     reserved",
		"21     reserved",
		"22     reserved",
		"MMX    MMX instruction set",
		"FXSR   Fast FP/MMX Streaming SIMD Extensions save/restore",
		"XMM    Streaming SIMD Extensions instruction set",
		"26     reserved",
		"27     reserved",
		"28     reserved",
		"29     reserved",
		"30     reserved",
		"31     reserved"
	};

	if (maxi != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x00000000; i <= maxi; i++) {
			cpuid (cpu->number, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
		}
		printf ("\n");
	}

	if (maxei != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (cpu->number, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
		}
		printf ("\n");
	}

	if (show_flags) {
		printf ("Feature flags %08lx:\n", edx);
		for (i = 0; i < 32; i++) {
			if (edx  & (1 << i))
				printf ("%s\n", x86_cap_flags[i]);
		}
	}

	if (maxei >= 0x80000001) {
		cpuid (cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
		decode_feature_flags (cpu, edx);
	}
}
