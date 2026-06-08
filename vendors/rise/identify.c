/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Rise specific parts.
 */
#include <stdio.h>
#include <x86info.h>

static char *rise_nameptr;
#define add_to_cpuname(x) do { \
	int _avail = (int)(cpu->name + CPU_NAME_LEN - rise_nameptr); \
	if (_avail > 1) { \
		int _n = snprintf(rise_nameptr, (size_t)_avail, "%s", x); \
		rise_nameptr += (_n > 0 && _n < _avail) ? _n : _avail - 1; \
	} \
} while (0)

void identify_rise(struct cpudata *cpu)
{
	rise_nameptr = cpu->name;

	switch (tuple(cpu) & 0xff0) {
	case 0x500:	add_to_cpuname("iDragon (0.25um)");
			break;
	case 0x520:	add_to_cpuname("iDragon (0.18um)");
			break;
	case 0x580:	add_to_cpuname("iDragon II (0.25um)");
			break;
	case 0x590:	add_to_cpuname("iDragon II (0.18um)");
			break;
	default:	add_to_cpuname("Unknown CPU");
			break;
	}
}
