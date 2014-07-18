/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  FreeBSD Routines for reading MSRs.
 */

#if defined(__FreeBSD__)

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

int read_msr(int cpu, unsigned int idx, unsigned long long *val)
{
	char cpuname[16];
	int fh;
	static int nodriver=0;
	cpuctl_msr_args_t args;

	if (nodriver==1)
		return 0;

	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpuctl%d", cpu);

	fh = open(cpuname, O_RDONLY);
	if (fh==-1) {
		perror(cpuname);
		nodriver=1;
		return 0;
	}

	args.msr = idx;
	if (ioctl(fh, CPUCTL_RDMSR, &args) != 0) {
		if (close(fh) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}

		return 0;
	}

	*val = args.data;

	if (close(fh)==-1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	return 1;
}

#endif /* __FreeBSD__ */
