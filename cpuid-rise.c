/*
 *  $Id: cpuid-rise.c,v 1.4 2001/04/28 00:58:24 davej Exp $
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

void doRise (int cpunum, unsigned int maxi, struct cpudata *cpu)
{
	unsigned int i;
	unsigned long maxei, eax, ebx, ecx, edx;
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

	cpu->vendor = VENDOR_RISE;

	cpuid (cpunum, 0x80000000, &maxei, NULL, NULL, NULL);

	if (maxi != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x00000000; i <= maxi; i++) {
			cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
		}
		printf ("\n");
	}

	if (maxei != 0 && show_registers) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (cpunum, i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
		}
		printf ("\n");
	}

	/* Do standard stuff */
	if (maxi >= 1) {
		cpuid (cpunum, 1, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;

		printf ("Family: %d Model: %d [iDragon ", cpu->family, cpu->model);
		switch (cpu->family) {
		case 5:
			switch (cpu->model) {
			case 0:
				printf ("(0.25um)");
				break;
			case 2:
				printf ("(0.18um)");
				break;
			case 8:
				printf ("II (0.25um)");
				break;
			case 9:
				printf ("II (0.18um)");
				break;
			default:
				printf ("<unknown type>");
				break;
			}
			break;

		default:
			printf ("<unknown type>");
			break;
		}
		printf ("]\n");

		printf("Stepping %d\n", cpu->stepping);

		/* according to Rise documentation,  COMPXCHG8B is always enable
		even tho the function bit is set to 0 (bit 8) 
		so I am just setting the edx value and let the rest of the co
		*/

	// FIXME: Hmmm, Not so sure about this.
	//	edx |= (1 << 8) ;
 
		if (show_flags) {
			printf ("Feature flags %08lx:\n", edx);
			for (i = 0; i < 32; i++) {
				if (edx  & (1 << i))
					printf ("%s\n", x86_cap_flags[i]);
			}
		}
	}

	/* Check for presence of extended info */
	if (maxei == 0)
		return;

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
		*cp++ = '\n';
		printf ("Processor name string: %s\n", namestring);
	}
}
