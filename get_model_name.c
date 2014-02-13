/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Get CPU name string from cpuid.
 */

#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include <amd.h>

void get_model_name(struct cpudata *cpu)
{
	unsigned int i, j;
	unsigned int eax, ebx, ecx, edx;
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
		unsigned int vendor;

		cpuid(cpu->number, 0, NULL, &vendor, NULL, NULL);
		if (vendor == 0x68747541 && cpu->cpuid_level >= 1 && cpu->maxei >= 0x80000001) {	/* AMD defined flags */
			cp = namestring;
			fix_amd_model_name(cpu, cp, sizeof(namestring));
		}
	}

	printf("Processor name string (BIOS programmed): %s\n", cp);
}
