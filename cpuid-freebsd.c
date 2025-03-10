/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  FreeBSD Routines for retrieving cpuid registers.
 *  Originally submitted by Stanislav Sedov <stas@FreeBSD.org>
 */

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/cpuctl.h>
#include <sys/cpuset.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <x86info.h>

void bind_cpu(unsigned int cpunr)
{
	cpuset_t mask;

	CPU_ZERO(&mask);
	CPU_SET(cpunr, &mask);
	(void) cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_PID, -1,
	    sizeof(mask), &mask);
}

void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{
	static int nodriver=0;
	char cpuname[20];
	int fh;
	cpuctl_cpuid_count_args_t args;

	if (nodriver == 1) {
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);
		return;
	}

	args.level = idx;
	args.level_type = idx >> 32;
	/* Ok, use the /dev/CPU interface in preference to the _up code. */
	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpuctl%u", CPU_number);
	fh = open(cpuname, O_RDONLY);
	if (fh != -1) {
		if (ioctl(fh, CPUCTL_CPUID_COUNT, &args) != 0) {
			perror(cpuname);
			exit(EXIT_FAILURE);
		}
		if (eax!=0)	*eax = args.data[0];
		if (ebx!=0)	*ebx = args.data[1];
		if (ecx!=0)	*ecx = args.data[2];
		if (edx!=0)	*edx = args.data[3];
		if (close(fh) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}
	} else {
		/* Something went wrong, just do UP and hope for the best. */
		nodriver = 1;
		if (nrCPUs != 1)
			perror(cpuname);
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);

		return;
	}
}

#endif /* __FreeBSD__ */
