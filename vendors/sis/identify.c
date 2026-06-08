/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include <x86info.h>

static char *sis_nameptr;
#define add_to_cpuname(x) do { \
	int _avail = (int)(cpu->name + CPU_NAME_LEN - sis_nameptr); \
	if (_avail > 1) { \
		int _n = snprintf(sis_nameptr, (size_t)_avail, "%s", x); \
		sis_nameptr += (_n > 0 && _n < _avail) ? _n : _avail - 1; \
	} \
} while (0)

void identify_sis(struct cpudata *cpu)
{
	sis_nameptr = cpu->name;

	switch (tuple(cpu)) {
		case 0x505:	add_to_cpuname("SiS55x");
					break;
		default:	add_to_cpuname("Unknown CPU");
					break;
	}
}
