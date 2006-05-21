/*
 *  $Id: get_model_name.c,v 1.4 2003/06/13 11:36:20 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Get CPU name string from cpuid.
 *
 */

#include <stdio.h>
#include <string.h>
#include "x86info.h"

void get_model_name(struct cpudata *cpu)
{
	unsigned int i, j;
	unsigned long eax, ebx, ecx, edx;
	char namestring[49], *cp;

	if (cpu->maxei < 0x80000004)
		return;

	cp = namestring;
	for (j = 0x80000002; j <= 0x80000004; j++) {
		cpuid(cpu->number, j, &eax, &ebx, &ecx, &edx);
		if (eax == 0)
			return;

		for (i = 0; i < 4; i++)
			*cp++ = eax >> (8 * i);
		for (i = 0; i < 4; i++)
			*cp++ = ebx >> (8 * i);
		for (i = 0; i < 4; i++)
			*cp++ = ecx >> (8 * i);
		for (i = 0; i < 4; i++)
			*cp++ = edx >> (8 * i);
	}
	cp = namestring;
	while(*cp == ' ')
		cp++;

	/* Broken BIOS? Try to determine the model name ourselves.  */
	if (strstr(cp, "unknown") != NULL) {
		unsigned long vendor;
		cpuid(cpu->number, 0, NULL, &vendor, NULL, NULL);
		if (vendor == 0x68747541 && cpu->maxi >= 1 && cpu->maxei >= 0x80000001) { /* AMD defined flags */
			unsigned long bid, ebid;
			cpuid(cpu->number, 0x00000001, NULL, &bid, NULL, NULL);
			bid &= 0xff;
			cpuid(cpu->number, 0x80000001, NULL, &ebid, NULL, NULL);

			/* 8BitBrandId == 0, BrandId == non-zero */
			if (bid == 0 && ebid != 0) {
				int BrandTableIndex = (ebid >> 6) & 0x3f;
				int NN = ebid & 0x3f;

				/* processor name string table */
				int model_number = 0;
				const char *name = NULL;
				switch (BrandTableIndex) {
				case 0x00: name = "AMD Engineering Sample"; break;
				case 0x04: name = "AMD Athlon(tm) 64 Processor %d00+"; model_number = 'X'; break;
				case 0x05: name = "AMD Athlon(tm) 64 X2 Dual Core Processor %d00+"; model_number = 'X'; break;
				case 0x08: name = "Mobile AMD Athlon(tm) 64 Processor %d00+"; model_number = 'X'; break;
				case 0x09: name = "Mobile AMD Athlon(tm) 64 Processor %d00+"; model_number = 'X'; break;
				case 0x0A: name = "AMD Turion(tm) 64 Mobile Technology ML-%d"; model_number = 'X'; break;
				case 0x0B: name = "AMD Turion(tm) 64 Mobile Technology MT-%d"; model_number = 'X'; break;
				case 0x0C: name = "AMD Opteron(tm) Processor 1%d"; model_number = 'Y'; break;
				case 0x0D: name = "AMD Opteron(tm) Processor 1%d"; model_number = 'Y'; break;
				case 0x0E: name = "AMD Opteron(tm) Processor 1%d HE"; model_number = 'Y'; break;
				case 0x0F: name = "AMD Opteron(tm) Processor 1%d EE"; model_number = 'Y'; break;
				case 0x10: name = "AMD Opteron(tm) Processor 2%d"; model_number = 'Y'; break;
				case 0x11: name = "AMD Opteron(tm) Processor 2%d"; model_number = 'Y'; break;
				case 0x12: name = "AMD Opteron(tm) Processor 2%d HE"; model_number = 'Y'; break;
				case 0x13: name = "AMD Opteron(tm) Processor 2%d EE"; model_number = 'Y'; break;
				case 0x14: name = "AMD Opteron(tm) Processor 8%d"; model_number = 'Y'; break;
				case 0x15: name = "AMD Opteron(tm) Processor 8%d"; model_number = 'Y'; break;
				case 0x16: name = "AMD Opteron(tm) Processor 8%d HE"; model_number = 'Y'; break;
				case 0x17: name = "AMD Opteron(tm) Processor 8%d EE"; model_number = 'Y'; break;
				case 0x18: name = "AMD Athlon(tm) 64 Processor %d00+"; model_number = 'E'; break;
				case 0x1D: name = "Mobile Athlon(tm) XP-M Processor %d00+"; model_number = 'X'; break;
				case 0x1E: name = "Mobile Athlon(tm) XP-M Processor %d00+"; model_number = 'X'; break;
				case 0x20: name = "AMD Athlon(tm) XP Processor %d00+"; model_number = 'X'; break;
				case 0x21: name = "Mobile AMD Sempron(tm) Processor %d00+"; model_number = 'T'; break;
				case 0x22: name = "AMD Sempron(tm) Processor %d00+"; model_number = 'T'; break;
				case 0x23: name = "Mobile AMD Sempron(tm) Processor %d00+"; model_number = 'T'; break;
				case 0x24: name = "AMD Athlon(tm) 64 FX-%d Processor"; model_number = 'Z'; break;
				case 0x26: name = "AMD Sempron(tm) Processor %d00+"; model_number = 'T'; break;
				case 0x29: name = "Dual Core AMD Opteron(tm) Processor 1%d SE"; model_number = 'R'; break;
				case 0x2A: name = "Dual Core AMD Opteron(tm) Processor 2%d SE"; model_number = 'R'; break;
				case 0x2B: name = "Dual Core AMD Opteron(tm) Processor 8%d SE"; model_number = 'R'; break;
				case 0x2C: name = "Dual Core AMD Opteron(tm) Processor 1%d"; model_number = 'R'; break;
				case 0x2D: name = "Dual Core AMD Opteron(tm) Processor 1%d"; model_number = 'R'; break;
				case 0x2E: name = "Dual Core AMD Opteron(tm) Processor 1%d HE"; model_number = 'R'; break;
				case 0x2F: name = "Dual Core AMD Opteron(tm) Processor 1%d EE"; model_number = 'R'; break;
				case 0x30: name = "Dual Core AMD Opteron(tm) Processor 2%d"; model_number = 'R'; break;
				case 0x31: name = "Dual Core AMD Opteron(tm) Processor 2%d"; model_number = 'R'; break;
				case 0x32: name = "Dual Core AMD Opteron(tm) Processor 2%d HE"; model_number = 'R'; break;
				case 0x33: name = "Dual Core AMD Opteron(tm) Processor 2%d EE"; model_number = 'R'; break;
				case 0x34: name = "Dual Core AMD Opteron(tm) Processor 8%d"; model_number = 'R'; break;
				case 0x35: name = "Dual Core AMD Opteron(tm) Processor 8%d"; model_number = 'R'; break;
				case 0x36: name = "Dual Core AMD Opteron(tm) Processor 8%d HE"; model_number = 'R'; break;
				case 0x37: name = "Dual Core AMD Opteron(tm) Processor 8%d EE"; model_number = 'R'; break;
				case 0x38: name = "Dual Core AMD Opteron(tm) Processor 1%d"; model_number = 'R'; break;
				case 0x39: name = "Dual Core AMD Opteron(tm) Processor 2%d"; model_number = 'R'; break;
				case 0x3A: name = "Dual Core AMD Opteron(tm) Processor 8%d"; model_number = 'R'; break;
				default: name = "AMD Processor model unknown"; break;
				}

				/* model number calculation */
				switch (model_number) {
				case 'X': model_number = 22 + NN; break;
				case 'Y': model_number = 38 + (2 * NN); break;
				case 'Z': model_number = 24 + NN; break;
				case 'T': model_number = 24 + NN; break;
				case 'R': model_number = 45 + (5 * NN); break;
				case 'E': model_number = 9 + NN; break;
				}

				cp = namestring;
				if (model_number)
					(void)snprintf(cp, sizeof(namestring), "%s %d", name, model_number);
				else
					strcpy(cp, name);
			}
		}
	}

	printf("Processor name string: %s\n\n", cp);
}
