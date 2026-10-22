/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Linux specific routines for reading MSRs.
 */

#ifdef __linux__

#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <x86info.h>

int read_msr(int cpu, unsigned int idx, unsigned long long *val)
{
	char cpuname[16];
	unsigned long long raw;
	int fh;
	static int nodriver=0;

	if (nodriver==1)
		return 0;

	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpu/%d/msr", cpu);

	fh = open(cpuname, O_RDONLY);
	if (fh == -1) {
		perror(cpuname);
		nodriver=1;
		return 0;
	}

	if (lseek64(fh, (off64_t) idx, SEEK_SET) == -1) {
		perror("lseek");
		close(fh);
		return 0;
	}

	if (read(fh, &raw, sizeof(raw)) != sizeof(raw)) {
		close(fh);
		return 0;
	}

	*val = raw;
	if (close(fh) == -1)
		perror("close");
	return 1;
}

#endif /* __linux__ */
