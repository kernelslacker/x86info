/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  NetBSD Routines for retrieving cpuid registers.
 */

#if defined(__NetBSD__)
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <x86info.h>

int bind_cpu(unsigned int cpunr)
{
	cpuset_t *cpuset;
	int ret;

	cpuset = cpuset_create();
	if (cpuset == NULL)
		return -1;
	cpuset_zero(cpuset);
	cpuset_set(cpunr, cpuset);
	ret = _sched_setaffinity(0, 0, cpuset_size(cpuset), cpuset);
	cpuset_destroy(cpuset);
	return ret;
}

static const char *NATIVE_CPUID_FAILED_MSG = "WARNING: Native cpuid failed\n";

void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{

	if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
		printf("%s", NATIVE_CPUID_FAILED_MSG);
}

#endif /* __NetBSD__ */
