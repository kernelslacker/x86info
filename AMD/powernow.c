/*
 *  $Id: powernow.c,v 1.1 2002/11/19 19:16:01 davej Exp $
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
#include "../x86info.h"
#include "AMD.h"

void decode_powernow(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;

	if (cpu->maxei < 0x80000007)
		return;

	cpuid(cpu->number, 0x80000007, &eax, &ebx, &ecx, &edx);
	printf("PowerNOW! Technology information\n");
	printf("Available features:");
	if (edx & 1 << 0)
		printf("\n\tTemperature sensing diode present.");
	if (edx & 1 << 1)
		printf("\n\tBus divisor control");
	if (edx & 1 << 2)
		printf("\n\tVoltage ID control\n");
	if (!(edx & (1 << 0 | 1 << 1 | 1 << 2)))
		printf(" None");
	printf("\n\n");

}

