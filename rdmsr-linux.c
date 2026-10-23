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
static int msr_fds[MAX_CPUS] = { [0 ... MAX_CPUS - 1] = CPUDEV_CLOSED };

int read_msr(int cpu, unsigned int idx, unsigned long long *val)
{
	unsigned long long raw;
	int fh;

	if (cpu < 0)
		return 0;
	fh = cpudev_fd(msr_fds, cpu, "msr");
	if (fh == -1)
		return 0;

	if (pread(fh, &raw, sizeof(raw), (off_t)idx) != sizeof(raw))
		return 0;

	*val = raw;
	return 1;
}

#endif /* __linux__ */
