/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  FreeBSD Routines for reading MSRs.
 */

#if defined(__FreeBSD__) && !defined(CPUID_REPLAY)

#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/cpuctl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <x86info.h>

bool read_msr(unsigned int cpu, unsigned int idx, unsigned long long *val)
{
	char cpuname[16];
	int fh;
	static int nodriver=0;
	cpuctl_msr_args_t args;

	if (nodriver==1)
		return false;

	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpuctl%u", cpu);

	fh = open(cpuname, O_RDONLY);
	if (fh==-1) {
		perror(cpuname);
		nodriver=1;
		return false;
	}

	args.msr = idx;
	if (ioctl(fh, CPUCTL_RDMSR, &args) != 0) {
		if (close(fh) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}

		return false;
	}

	*val = args.data;

	if (close(fh)==-1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	return true;
}

#endif /* __FreeBSD__ */
