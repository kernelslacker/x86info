/*
 *  $Id: identify.c,v 1.8 2001/12/10 17:52:15 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Intel specific parts
 */

#include <stdio.h>
#include "../x86info.h"
#include "Intel.h"

/* Decode Intel TLB and cache info descriptors */
void decode_Intel_TLB (int x, int family)
{
	switch (x & 0xff) {
	case 0:
		break;
	case 0x1:
		printf ("Instruction TLB: 4KB pages, 4-way set assoc, 32 entries\n");
		break;
	case 0x2:
		printf ("Instruction TLB: 4MB pages, fully assoc, 2 entries\n");
		break;
	case 0x3:
		printf ("Data TLB: 4KB pages, 4-way set assoc, 64 entries\n");
		break;
	case 0x4:
		printf ("Data TLB: 4MB pages, 4-way set assoc, 8 entries\n");
		break;
	case 0x6:
		printf ("Instruction cache: 8KB, 4-way set assoc, 32 byte line size\n");
		break;
	case 0x8:
		printf ("Instruction cache: 16KB, 4-way set assoc, 32 byte line size\n");
		break;
	case 0xa:
		printf ("Data cache: 8KB, 2-way set assoc, 32 byte line size\n");
		break;
	case 0xc:
		printf ("Data cache: 16KB, 2-way or 4-way set assoc, 32 byte line size\n");
		break;
	case 0x40:
		if (family==15)
			printf ("No L3 cache\n");	/* Pentium 4 */
		else
			printf ("No L2 cache\n");
		break;
	case 0x41:
		printf ("L2 unified cache: 128KB, 4-way set assoc, 32 byte line size\n");
		break;
	case 0x42:
		printf ("L2 unified cache: 256KB, 4-way set assoc, 32 byte line size\n");
		break;
	case 0x43:
		printf ("L2 unified cache: 512KB, 4-way set assoc, 32 byte line size\n");
		break;
	case 0x44:
		printf ("L2 unified cache: 1MB, 4-way set assoc, 32 byte line size\n");
		break;
	case 0x45:
		printf ("L2 unified cache: 2MB, 4-way set assoc, 32 byte line size\n");
		break;
	case 0x50:
		printf ("Instruction TLB: 4K, 2MB or 4MB pages, fully assoc, 64 entries\n");
		break;
	case 0x5b:
		printf ("Data TLB: 4K or 4MB pages, fully assoc, 64 entries\n");
		break;
	case 0x66:
		printf ("Data cache: Sectored, 64 byte cache line, 8 way set assoc, 8K\n");
		break;
	case 0x70:
		printf ("Instruction trace cache: 4 way set associative, 12K uOps\n");
		break;
	case 0x7a:
		printf ("L2 unified cache: Sectored, 64 byte cache line, 8 way set associative, 256K\n");
		break;
	case 0x82:
		printf ("L2 unified cache: Sectored, 32 byte cache line, 8 way set associative, 256K\n");
		break;
	case 0x84:
		printf ("L2 unified cache: Sectored, 32 byte cache line, 8 way set associative, 1MB\n");
		break;
	case 0x85:
		printf ("L2 unified cache: Sectored, 32 byte cache line, 8 way set associative, 2MB\n");
		break;

	default:
		printf ("unknown TLB/cache descriptor: 0x%x\n", x);
		break;
	}
}


/* Intel-specific information */
void Identify_Intel (unsigned int maxi, struct cpudata *cpu)
{
	char *nameptr;
	unsigned long eax, ebx, ecx, edx;
	int reserved;

	cpu->vendor = VENDOR_INTEL;
	nameptr = cpu->name;

	if (maxi >= 1) {
		/* Family/model/type etc */
		cpuid (cpu->number, 1, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;
		cpu->type = (eax >> 12) & 0x3;
		cpu->brand = (ebx & 0xf);
		reserved = eax >> 14;

		switch (cpu->family) {
		case 4:		/* Family 4 */
			nameptr += sprintf (cpu->name, "%s", "i486 ");
			switch (cpu->model) {
			case 0:
				nameptr+=sprintf (nameptr, "%s", "DX-25/33");
				break;
			case 1:
				nameptr+=sprintf (nameptr, "%s", "DX-50");
				break;
			case 2:
				nameptr+=sprintf (nameptr, "%s", "SX");
				break;
			case 3:
				nameptr+=sprintf (nameptr, "%s", "487/DX2");
				break;
			case 4:
				nameptr+=sprintf (nameptr, "%s", "SL");
				break;
			case 5:
				nameptr+=sprintf (nameptr, "%s", "SX2");
				break;
			case 7:
				nameptr+=sprintf (nameptr, "%s", "write-back enhanced DX2");
				break;
			case 8:
				nameptr+=sprintf (nameptr, "%s", "DX4");
				break;
			case 9:
				nameptr+=sprintf (nameptr, "%s", "write-back enhanced DX4");
				break;
			default:
				nameptr+=sprintf (nameptr, "%s", "Unknown CPU");
				break;
			}
			break;

		case 5:		/* Family 5 */
			nameptr += sprintf (cpu->name, "%s", "Pentium ");
			switch (cpu->model) {
			case 0:
				nameptr+=sprintf (nameptr, "%s", "A-step");
				break;
			case 1:
				nameptr+=sprintf (nameptr, "%s", "60/66");
				break;
			case 2:
				nameptr+=sprintf (nameptr, "%s", "75-200");
				break;
			case 3:
				nameptr+=sprintf (nameptr, "%s", "Overdrive");
				break;
			case 4:
				nameptr+=sprintf (nameptr, "%s", "MMX");
				break;
			case 7:
				nameptr+=sprintf (nameptr, "%s", "Mobile");
				break;
			case 8:
				nameptr+=sprintf (nameptr, "%s", "MMX Mobile");
				break;
			}
			break;

		case 6:		/* Family 6 */
			switch (cpu->model) {
			case 0:
				nameptr+=sprintf (cpu->name, "%s", "Pentium Pro A-Step");
				break;
			case 1:
				nameptr+=sprintf (cpu->name, "%s", "Pentium Pro");
				break;
			case 3:
				nameptr+=sprintf (cpu->name, "%s", "Pentium II ");
				switch (cpu->stepping) {
				case 2:
					nameptr+=sprintf (nameptr, "%s", "Overdrive");
					break;
				case 3:
					nameptr+=sprintf (nameptr, "%s", "(Klamath) [C0]");
					break;
				case 4:
					nameptr+=sprintf (nameptr, "%s", "(Klamath) [C1]");
					break;
				}
				break;
			case 4:
				nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes?)");
				break;
			case 5:
				/*FIXME: Cache size determine needed here */
				switch (cpu->stepping) {
				case 0:
					nameptr+=sprintf (cpu->name, "%s", "Celeron / Pentium II [dA0]");
					break;
				case 1:
					nameptr+=sprintf (cpu->name, "%s", "Celeron / Pentium II (Deschutes) [dA1]");
					break;
				case 2:
					nameptr+=sprintf (cpu->name, "%s", "Celeron / Pentium II (Deschutes) [dB0]");
					break;
				case 3:
					nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes) [dB1]");
					break;
				default:
					nameptr+=sprintf (cpu->name, "%s", "Pentium II");
					break;
				}
				break;
			case 6:
				switch (cpu->stepping) {
				case 0:
					nameptr+=sprintf (cpu->name, "%s", "Celeron-A [mA0]");
					break;
				case 5:
					nameptr+=sprintf (cpu->name, "%s", "Celeron-A [mB0]");
					break;
				case 0xA:
					nameptr+=sprintf (cpu->name, "%s", "Mobile Pentium II [mdA0]");
					break;
				default:
					nameptr+=sprintf (cpu->name, "%s", "Celeron / Mobile Pentium II");
					break;
				}
				break;
			case 7:
				switch (cpu->stepping) {
				case 2:
					nameptr+=sprintf (cpu->name, "%s", "Pentium III [kB0]");
					break;
				case 3:
					nameptr+=sprintf (cpu->name, "%s", "Pentium III [kC0]");
					break;
				default:
					nameptr+=sprintf (cpu->name, "%s", "Pentium III/Pentium III Xeon");
					break;
				}
				break;
			case 8:
				nameptr+=sprintf (cpu->name, "%s", "Celeron / Pentium III");
				switch (cpu->stepping) {
				case 1:
					nameptr+=sprintf (nameptr, "%s", " [cA2]");
					break;
				case 3:
					nameptr+=sprintf (nameptr, "%s", " [cB0]");
					break;
				case 6:
					nameptr+=sprintf (nameptr, "%s", " [cC0]");
					break;
				case 0xA:
					nameptr+=sprintf (nameptr, "%s", " [cD0]");
					break;
				}
				break;
			case 0xA:
				switch (cpu->brand) {
				case 0:
					nameptr+=sprintf (cpu->name, "%s", "Pentium II (Deschutes)");
					break;
				case 1:
					nameptr+=sprintf (cpu->name, "%s", "Celeron");
					break;
				case 2:
					nameptr+=sprintf (cpu->name, "%s", "Pentium III");
					break;
				case 3:
					nameptr+=sprintf (cpu->name, "%s", "Pentium III Xeon");
					break;
				default:
					nameptr+=sprintf (cpu->name, "%s", "Unknown CPU");
					break;
				}
				break;
			case 0xB:
				switch (cpu->stepping) {
				case 1:
					nameptr+=sprintf (cpu->name, "%s", "Celeron / Pentium III (Tualatin) [tA1]");
					break;
				default:
					nameptr+=sprintf (cpu->name, "%s", "Unknown CPU");
					break;
				}
				break;
			}
			break;

		case 7:		/* Family 7 */
			nameptr += sprintf (cpu->name, "%s", "Itanium");
			break;

		case 0xF:	/* Family 15 */
			switch (cpu->model) {
			case 0:
				nameptr += sprintf (cpu->name, "%s", "Pentium 4");
				switch (cpu->stepping) {
				case 7:
					nameptr+=sprintf (nameptr, "%s", " [B2]");
					break;
				case 0xA:
					nameptr+=sprintf (nameptr, "%s", " [C1]");
					break;
				}
				break;
			case 1:
				nameptr += sprintf (cpu->name, "%s", "Pentium 4");
				switch (cpu->stepping) {
				case 2:
					nameptr+=sprintf (nameptr, "%s", " [D0]");
					break;
				}
				break;
			case 4:
			case 5:
				nameptr+=sprintf (cpu->name, "%s", "P4 Xeon (Foster)");
				break;
			default:
				nameptr+=sprintf (cpu->name, "%s", "Unknown CPU");
				break;
			}
			break;
		}

		switch (cpu->type) {
		case 0:
			sprintf (nameptr, "%s", " Original OEM");
			break;
		case 1:
			sprintf (nameptr, "%s", " Overdrive");
			break;
		case 2:
			sprintf (nameptr, "%s", " Dual-capable");
			break;
		case 3:
			sprintf (nameptr, "%s", " Reserved");
			break;
		}
	}
}


void display_Intel_info (unsigned int maxi, struct cpudata *cpu)
{
	int ntlb, i;
	unsigned long eax, ebx, ecx, edx;

	cpuid (cpu->number, 0x00000001, &eax, &ebx, &ecx, &edx);
	decode_feature_flags (cpu, edx);

	if (maxi >= 2 && show_cacheinfo) {
		/* Decode TLB and cache info */
		ntlb = 255;
		for (i = 0; i < ntlb; i++) {
			cpuid (cpu->number, 2, &eax, &ebx, &ecx, &edx);
			ntlb = eax & 0xff;
			decode_Intel_TLB (eax >> 8, cpu->family);
			decode_Intel_TLB (eax >> 16, cpu->family);
			decode_Intel_TLB (eax >> 24, cpu->family);

			if ((ebx & 0x80000000) == 0) {
				decode_Intel_TLB (ebx, cpu->family);
				decode_Intel_TLB (ebx >> 8, cpu->family);
				decode_Intel_TLB (ebx >> 16, cpu->family);
				decode_Intel_TLB (ebx >> 24, cpu->family);
			}
			if ((ecx & 0x80000000) == 0) {
				decode_Intel_TLB (ecx, cpu->family);
				decode_Intel_TLB (ecx >> 8, cpu->family);
				decode_Intel_TLB (ecx >> 16, cpu->family);
				decode_Intel_TLB (ecx >> 24, cpu->family);
			}
			if ((edx & 0x80000000) == 0) {
				decode_Intel_TLB (edx, cpu->family);
				decode_Intel_TLB (edx >> 8, cpu->family);
				decode_Intel_TLB (edx >> 16, cpu->family);
				decode_Intel_TLB (edx >> 24, cpu->family);
			}
		}
	}

	if (maxi >= 3) {
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
		decode_Intel_bluesmoke(cpu->number);
}
