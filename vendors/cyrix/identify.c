/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Cyrix specific information.
 */

#include <stdio.h>
#include <x86info.h>
#include "cyrix.h"

/* Decode TLB and cache info descriptors */
void decode_cyrix_tlb(int x)
{
	switch (x & 0xff) {
	case 0:
		break;
	case 0x70:
		printf("TLB: 32 entries 4-way associative 4KB pages\n");
		break;
	case 0x80:
		printf("L1 Cache: 16KB 4-way associative 16 bytes/line\n");
		break;
	}
}

/* Cyrix-specific information */
void identify_cyrix(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
	int known = 1;

	switch (tuple(cpu) & 0xff0) {
		case 0x450:	add_to_cpuname("MediaGX");
				break;

		case 0x520:	if (cpu->stepping == 4)
					add_to_cpuname("GXm");
				else
					add_to_cpuname("6x86");
				break;

		case 0x600:	add_to_cpuname("6x86/MX");
				break;
		case 0x620:	add_to_cpuname("MII");
				break;
		default:	add_to_cpuname("Unknown CPU");
				known = 0;
				break;
	}

	/* The extended family only refines an unrecognised tuple */
	if (known || cpu->maxei < 0x80000000)
		return;

	if (cpu->maxei >= 0x80000001) {
		cpuid (cpu->number, 0x80000001, &eax, &ebx, &ecx, &edx);
		switch ((eax >> 8) & 0xf) {
			case 4:	add_to_cpuname("MediaGX");
					break;
			case 5:	add_to_cpuname("6x86/GXm");
					break;
			case 6:	add_to_cpuname("6x86/MX");
					break;
		}
	}
}

void display_cyrix_info(struct cpudata *cpu)
{
	unsigned int i, ntlb;
	unsigned int eax, ebx, ecx, edx;

	if (cpu->cpuid_level >= 2 && show_cacheinfo) {
		printf("TLB & L1 Cache info\n");
		/* TLB and L1 Cache info */
		ntlb = 255;
		for (i = 0; i < ntlb; i++) {
			cpuid (cpu->number, 2, &eax, &ebx, &ecx, &edx);
			ntlb = eax & 0xff;
			decode_cyrix_tlb(eax >> 8);
			decode_cyrix_tlb(eax >> 16);
			decode_cyrix_tlb(eax >> 24);

			/* ebx and ecx are reserved */
			if ((edx & 0x80000000) == 0) {
				decode_cyrix_tlb(edx);
				decode_cyrix_tlb(edx >> 8);
				decode_cyrix_tlb(edx >> 16);
				decode_cyrix_tlb(edx >> 24);
			}
		}
	}

	if (cpu->maxei >= 0x80000005 && show_cacheinfo) {
		printf("TLB & L1 Cache info from extended info\n");
		/* TLB and L1 Cache info */
		cpuid (cpu->number, 0x80000005, &eax, &ebx, &ecx, &edx);
		decode_cyrix_tlb(ebx >> 8);
		decode_cyrix_tlb(ebx >> 16);
		decode_cyrix_tlb(ebx >> 24);

		/* eax and edx are reserved */
		if ((ecx & 0x80000000) == 0) {
			decode_cyrix_tlb(ecx);
			decode_cyrix_tlb(ecx >> 8);
			decode_cyrix_tlb(ecx >> 16);
			decode_cyrix_tlb(ecx >> 24);
		}
	}
}
