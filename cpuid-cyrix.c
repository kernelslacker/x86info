/* x86info by Dave Jones.
 * Based on 'cpuid' by Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#include <stdio.h>
#include "x86info.h"

/* Decode Intel TLB and cache info descriptors */
void decode_cyrix_tlb (int x)
{
	switch (x & 0xff) {
	case 0:
		break;
	case 0x70:
		printf ("TLB: 32 entries 4-way associative 4KB pages\n");
		break;
	case 0x80:
		printf ("L1 Cache: 16KB 4-way associative 16 bytes/line\n");
		break;
	}
}

/* Cyrix-specific information */
void docyrix (int maxi, struct cpudata *cpu)
{
	unsigned int i, ntlb;
	unsigned long maxei, eax, ebx, ecx, edx;
	int stepping, model, family, reserved;

	printf ("Cyrix-specific functions\n");
	cpuid (0x80000000, &maxei, NULL, NULL, NULL);
	if (maxei >= 0x80000000) {
		/* Dump extended info in raw hex */
		for (i = 0x80000000; i <= maxei; i++) {
			cpuid (i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx,
				edx);
		}
	}

	/* Do standard stuff */
	if (maxi >= 1) {
		cpuid (1, &eax, &ebx, &ecx, &edx);
		stepping = eax & 0xf;
		model = (eax >> 4) & 0xf;
		family = (eax >> 8) & 0xf;
		reserved = eax >> 12;

		printf ("Family: %d Model: %d [", family, model);
		switch (family) {
		case 4:
			switch (model) {
			case 4:
				printf ("MediaGX");
				break;
			}
			break;
		case 5:
			switch (model) {
			case 2:
				printf ("6x86");
				break;
			case 4:
				printf ("BXm");
				break;
			}
			break;
		case 6:
			switch (model) {
			case 0:
				printf ("6x86/MX");
				break;
			}
			break;
		}
		printf ("]\n\n");
		printf ("Standard feature flags:\n");
		if (edx & (1 << 0))
			printf ("FPU   Floating Point Unit\n");
		if (edx & (1 << 1))
			printf ("V86   Virtual Mode Extensions\n");
		if (edx & (1 << 2))
			printf ("Debug Extension\n");
		if (edx & (1 << 3))
			printf ("4MB Page Size\n");
		if (edx & (1 << 4))
			printf ("Time Stamp Counter\n");
		if (edx & (1 << 5))
			printf ("RDMSR/WRMSR (Model Specific Registers)\n");
		if (edx & (1 << 6))
			printf ("PAE\n");
		if (edx & (1 << 7))
			printf ("Machine Check Exception\n");
		if (edx & (1 << 8))
			printf ("COMPXCHG8B Instruction\n");
		if (edx & (1 << 9)) {
			printf ("APIC - On-chip Advanced Programmable Interrupt Controller present and enabled\n");
		}
		if (edx & (1 << 10))
			printf ("10 - Reserved\n");
		if (edx & (1 << 11))
			printf ("11 - Reserved\n");
		if (edx & (1 << 12))
			printf ("MTRR  Memory Type Range Registers\n");
		if (edx & (1 << 13)) {
			if (family == 5 && model == 0)
				printf ("13 - reserved\n");
			else
				printf ("Global Paging Extension\n");
		}
		if (edx & (1 << 14))
			printf ("Machine Check\n");
		if (edx & (1 << 15))
			printf ("CMOV  Conditional Move Instruction\n");
		if (edx & (1 << 16))
			printf ("16 - reserved\n");
		if (edx & (1 << 17))
			printf ("17 - reserved\n");
		if (edx & (1 << 18))
			printf ("18 - reserved\n");
		if (edx & (1 << 19))
			printf ("19 - reserved\n");
		if (edx & (1 << 20))
			printf ("20 - reserved\n");
		if (edx & (1 << 21))
			printf ("21 - reserved\n");
		if (edx & (1 << 22))
			printf ("22 - reserved\n");
		if (edx & (1 << 23))
			printf ("MMX instructions\n");
		if (edx & (1 << 24))
			printf ("24 - reserved\n");
		if (edx & (1 << 25))
			printf ("25 - reserved\n");
		if (edx & (1 << 26))
			printf ("26 - reserved\n");
		if (edx & (1 << 27))
			printf ("27 - reserved\n");
		if (edx & (1 << 28))
			printf ("28 - reserved\n");
		if (edx & (1 << 29))
			printf ("29 - reserved\n");
		if (edx & (1 << 30))
			printf ("30 - reserved\n");
		if (edx & (1 << 31))
			printf ("31 - reserved\n");
	}
	if (maxi >= 2) {
		/* TLB and L1 Cache info */
		ntlb = 255;
		for (i = 0; i < ntlb; i++) {
			cpuid (2, &eax, &ebx, &ecx, &edx);
			ntlb = eax & 0xff;
			decode_cyrix_tlb (eax >> 8);
			decode_cyrix_tlb (eax >> 16);
			decode_cyrix_tlb (eax >> 24);

			/* ebx and ecx are reserved */

			if ((edx & 0x80000000) == 0) {
				decode_cyrix_tlb (edx);
				decode_cyrix_tlb (edx >> 8);
				decode_cyrix_tlb (edx >> 16);
				decode_cyrix_tlb (edx >> 24);
			}
		}
	}

	/* Check for presence of extended info */
	if (maxei < 0x80000000)
		return;

	printf ("\nExtended feature flags:\n");
	if (maxei >= 0x80000001) {
		cpuid (0x80000001, &eax, &ebx, &ecx, &edx);
		stepping = eax & 0xf;
		model = (eax >> 4) & 0xf;
		family = (eax >> 8) & 0xf;
		reserved = eax >> 12;
		printf ("Family: %d Model: %d [", family, model);
		switch (family) {
		case 4:
			printf ("MediaGX");
			break;
		case 5:
			printf ("6x86/GXm");
			break;
		case 6:
			printf ("6x86/MX");
		}
		printf ("]\n\n");

		printf ("Extended feature flags:\n");
		if (edx & (1 << 0))
			printf ("FPU   Floating Point Unit\n");
		if (edx & (1 << 1))
			printf ("V86   Virtual Mode Extensions\n");
		if (edx & (1 << 2))
			printf ("Debug Extension\n");
		if (edx & (1 << 3))
			printf ("Page Size Extensions\n");
		if (edx & (1 << 4))
			printf ("Time Stamp Counter\n");
		if (edx & (1 << 5))
			printf ("Cyrix MSR\n");
		if (edx & (1 << 6))
			printf ("PAE\n");
		if (edx & (1 << 7))
			printf ("MC Exception\n");
		if (edx & (1 << 8))
			printf ("COMPXCHG8B\n");
		if (edx & (1 << 9))
			printf ("APIC on chip\n");
		if (edx & (1 << 10))
			printf ("SYSCALL/SYSRET\n");
		if (edx & (1 << 11))
			printf ("11 - reserved\n");
		if (edx & (1 << 12))
			printf ("MTRR\n");
		if (edx & (1 << 13))
			printf ("Global bit\n");
		if (edx & (1 << 14))
			printf ("Machine Check\n");
		if (edx & (1 << 15))
			printf ("CMOV\n");
		if (edx & (1 << 16))
			printf ("FPU CMOV\n");
		if (edx & (1 << 17))
			printf ("17 - reserved\n");
		if (edx & (1 << 18))
			printf ("18 - reserved\n");
		if (edx & (1 << 19))
			printf ("19 - reserved\n");
		if (edx & (1 << 20))
			printf ("20 - reserved\n");
		if (edx & (1 << 21))
			printf ("21 - reserved\n");
		if (edx & (1 << 22))
			printf ("22 - reserved\n");
		if (edx & (1 << 23))
			printf ("MMX\n");
		if (edx & (1 << 24))
			printf ("Extended MMX\n");
		if (edx & (1 << 25))
			printf ("25 - reserved\n");
		if (edx & (1 << 26))
			printf ("26 - reserved\n");
		if (edx & (1 << 27))
			printf ("27 - reserved\n");
		if (edx & (1 << 28))
			printf ("28 - reserved\n");
		if (edx & (1 << 29))
			printf ("29 - reserved\n");
		if (edx & (1 << 30))
			printf ("30 - reserved\n");
		if (edx & (1 << 31))
			printf ("3DNow instructions\n");
	}
	printf ("\n");
	if (maxei >= 0x80000002) {
		/* Processor identification string */
		char namestring[49], *cp;
		unsigned int j;
		cp = namestring;
		for (j = 0x80000002; j <= 0x80000004; j++) {
			cpuid (j, &eax, &ebx, &ecx, &edx);

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
	if (maxei >= 0x80000005) {
		/* TLB and L1 Cache info */
		ntlb = 255;
		for (i = 0; i < ntlb; i++) {
			cpuid (0x80000005, &eax, &ebx, &ecx, &edx);
			ntlb = eax & 0xff;
			decode_cyrix_tlb (ebx >> 8);
			decode_cyrix_tlb (ebx >> 16);
			decode_cyrix_tlb (ebx >> 24);

			/* eax and edx are reserved */

			if ((ecx & 0x80000000) == 0) {
				decode_cyrix_tlb (ecx);
				decode_cyrix_tlb (ecx >> 8);
				decode_cyrix_tlb (ecx >> 16);
				decode_cyrix_tlb (ecx >> 24);
			}
		}
	}
}
