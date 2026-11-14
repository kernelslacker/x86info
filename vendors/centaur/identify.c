/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Centaur specific parts.
 */
#include <stdio.h>
#include <x86info.h>
#include "centaur.h"

/* Zhaoxin parts have an extended model, or are family 7. */
static void identify_zhaoxin(struct cpudata *cpu)
{
	switch ((family(cpu) << 8) | model(cpu)) {
	case 0x619:
		add_to_cpuname("Zhaoxin ZXC");
		break;
	case 0x71b:
		add_to_cpuname("Zhaoxin ZXD");
		break;
	case 0x73b:
		add_to_cpuname("Zhaoxin ZXE");
		break;
	default:
		add_to_cpuname("Unknown Zhaoxin CPU");
		break;
	}
}

void identify_centaur(struct cpudata *cpu)
{
	if (cpu->family == 7 || cpu->emodel) {
		identify_zhaoxin(cpu);
		return;
	}

	switch (tuple(cpu) & 0xff0) {
		case 0x540:
			add_to_cpuname("Winchip C6");
			break;
		case 0x580:
			switch (cpu->stepping) {
			case 0 ... 6:
				add_to_cpuname("Winchip 2");
				break;
			case 7 ... 9:
				add_to_cpuname("Winchip 2A");
				break;
			case 0xA ... 0xF:
				add_to_cpuname("Winchip 2B");
				break;
			}
			break;
		case 0x590:
			add_to_cpuname("Winchip 3");
			break;

		/* Family 6 is when VIA bought out Cyrix & Centaur
		 * This is the CyrixIII family. */
		case 0x660:
			add_to_cpuname("VIA Cyrix 3 (Samuel) [C5A]");
				break;
		case 0x670:
				switch (cpu->stepping) {
				case 0 ... 7:
					add_to_cpuname("VIA C3 (Samuel 2) [C5B]");
					break;
				case 8 ... 0xf:
					add_to_cpuname("VIA C3 (Ezra) [C5C]");
					break;
				}
				break;
		/* Ezra-T is much like Ezra but reworked to run in Pentium III Tualatin sockets. */
		case 0x680:	add_to_cpuname("VIA C3 (Ezra-T) [C5M/C5N]");
					break;
		case 0x690:	add_to_cpuname("VIA C3 (Nehemiah) [C5XL]");
					break;
		case 0x6A0:	switch (cpu->stepping) {
					case 0:
					case 8 ... 0xF:
						add_to_cpuname("VIA C3 (Esther) [C7-M]");
						break;
					case 1 ... 7:
						add_to_cpuname("VIA C3 (Ruth) [C7-M]");
						break;
					}
					break;

	// C5P introduced the HW AES
	// C5YL
	// C5X
	// CZA

		default:
			add_to_cpuname("Unknown VIA CPU");
			break;
	}
}


void show_centaur_cache(struct cpudata *cpu)
{
	unsigned int ebx, ecx, edx, l2_ecx;

	if (cpu->maxei >= 0x80000005) {
		/* TLB and cache info */
		cpuid(cpu->number, 0x80000005, NULL, &ebx, &ecx, &edx);
		printf("Cache info\n");
		printf(" L1 Instruction cache: %uKB, %u-way associative, %u lines per tag, line size=%u bytes.\n",
			edx >> 24, (edx >> 16) & 0xff, (edx >> 8) & 0xff, edx & 0xff);
		printf(" L1 Data cache: %uKB %u-way associative, %u lines per tag, line size=%u bytes.\n",
			ecx >> 24, (ecx >> 16) & 0xff, (ecx >> 8) & 0xff, ecx & 0xff);
		if (cpu->maxei >= 0x80000006) {
			cpuid(cpu->number, 0x80000006, NULL, NULL, &l2_ecx, NULL);
			if ((cpu->family==6) && (cpu->model==7 || cpu->model==8))
				/* Work around errata. */
				printf(" L2 (on CPU) cache: %uKB %u-way associative, %u lines per tag, line size=%u bytes.\n",
					l2_ecx >> 24, (l2_ecx >> 16) & 0x0f, (l2_ecx >> 8) & 0x0f, l2_ecx & 0xff);
			else
				printf(" L2 (on CPU) cache: %uKB %u-way associative, %u lines per tag, line size=%u bytes.\n",
					l2_ecx >> 16, (l2_ecx >> 12) & 0x0f, (l2_ecx >> 8) & 0x0f, l2_ecx & 0xff);
		}
		printf("TLB info\n");
		printf(" Instruction TLB: %u-way associative. %u entries.\n", (ebx >> 8) & 0xff, ebx & 0xff);
		printf(" Data TLB: %u-way associative. %u entries.\n", ebx >> 24, (ebx >> 16) & 0xff);
	}
}


void display_centaur_info(struct cpudata *cpu)
{
	if (cpu->maxei == 0)
		return;

	if (cpu->family == 6 && opts.show_msr)
		dump_C3_MSR(cpu);
}
