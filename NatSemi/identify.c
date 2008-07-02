/*
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  NatSemi specific parts.
 */
#include <stdio.h>
#include "../x86info.h"

static char *NatSemi_nameptr;
#define add_to_cpuname(x)   NatSemi_nameptr += snprintf(NatSemi_nameptr, sizeof(x), "%s", x);

void identify_natsemi(struct cpudata *cpu)
{
	NatSemi_nameptr = cpu->name;

	switch (tuple(cpu) & 0xff0) {
		case 0x540:	add_to_cpuname("Geode GX1");
					break;
		default:	add_to_cpuname("Unknown CPU");
					break;
	}
}
