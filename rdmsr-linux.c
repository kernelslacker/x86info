/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Linux specific routines for reading MSRs.
 */

#if defined(__linux__) && !defined(CPUID_REPLAY)

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <x86info.h>

/* Per-CPU /dev/cpu/N/msr descriptors, opened lazily. */
static int msr_fds[MAX_CPUS];

bool read_msr(unsigned int cpu, unsigned int idx, unsigned long long *val)
{
	unsigned long long raw;

	if (cpudev_pread(msr_fds, cpu, "msr", &raw, sizeof(raw), (off_t)idx) !=
	    sizeof(raw))
		return false;

	*val = raw;
	return true;
}

#endif /* __linux__ */
