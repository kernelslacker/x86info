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

#define MAX_CPUS 8192

/* Per-CPU /dev/cpu/N/msr descriptors, opened lazily and stored as fd + 1. */
static int msr_fds[MAX_CPUS];

static int msr_fd(int cpu)
{
	char cpuname[32];
	int fh;

	if (cpu < 0 || cpu >= MAX_CPUS)
		return -1;
	if (msr_fds[cpu])
		return msr_fds[cpu] - 1;

	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpu/%d/msr", cpu);
	fh = open(cpuname, O_RDONLY);
	if (fh == -1) {
		perror(cpuname);
		if (errno == ENOENT)
			fprintf(stderr, "Is the msr module loaded? Try 'modprobe msr'.\n");
	} else
		msr_fds[cpu] = fh + 1;
	return fh;
}

int read_msr(int cpu, unsigned int idx, unsigned long long *val)
{
	unsigned long long raw;
	int fh;
	static int nodriver=0;

	if (nodriver==1)
		return 0;

	fh = msr_fd(cpu);
	if (fh == -1) {
		nodriver=1;
		return 0;
	}

	if (pread(fh, &raw, sizeof(raw), (off_t)idx) != sizeof(raw))
		return 0;

	*val = raw;
	return 1;
}

#endif /* __linux__ */
