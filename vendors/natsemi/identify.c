/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  NatSemi specific parts.
 */
#include <stdio.h>
#include <x86info.h>

static char *NatSemi_nameptr;
#define add_to_cpuname(x) do { \
	int _avail = (int)(cpu->name + CPU_NAME_LEN - NatSemi_nameptr); \
	if (_avail > 1) { \
		int _n = snprintf(NatSemi_nameptr, (size_t)_avail, "%s", x); \
		NatSemi_nameptr += (_n > 0 && _n < _avail) ? _n : _avail - 1; \
	} \
} while (0)

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
