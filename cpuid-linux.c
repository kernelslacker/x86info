/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Linux specific routines for retrieving cpuid registers.
 */

#if defined(__linux__) && !defined(CPUID_REPLAY)

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <sched.h>

#include <x86info.h>

/* The CPU this thread is bound to, or -1 if bind_cpu() hasn't succeeded. */
static __thread int bound_cpu = -1;

int bind_cpu(unsigned int cpunr)
{
	cpu_set_t *set;
	size_t size = CPU_ALLOC_SIZE(cpunr + 1);
	int ret;

	if (bound_cpu == (int)cpunr)
		return 0;

	set = CPU_ALLOC(cpunr + 1);
	if (!set)
		return -1;

	CPU_ZERO_S(size, set);
	CPU_SET_S(cpunr, size, set);
	ret = sched_setaffinity(0, size, set);
	CPU_FREE(set);
	if (ret == 0)
		bound_cpu = cpunr;
	return ret;
}

/* Kernel CPUID driver's minimum supported read size
 * (see linux/arch/i386/kernel/cpuid.c)
 */
#define CPUID_CHUNK_SIZE (16)

/* Per-CPU /dev/cpu/N/cpuid descriptors, opened lazily. */
static int cpuid_fds[MAX_CPUS] = { [0 ... MAX_CPUS - 1] = CPUDEV_CLOSED };

int cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{
	unsigned char buffer[CPUID_CHUNK_SIZE];
	int fh;

	/* Use the /dev/cpu interface in preference to the _up code. */
	fh = cpudev_fd(cpuid_fds, CPU_number, "cpuid");

	/* No device for this CPU, execute CPUID on it directly. */
	if (fh == -1)
		return native_cpuid(CPU_number, idx, eax,ebx,ecx,edx);

	memset(buffer, 0, sizeof(buffer));
	if (pread(fh, buffer, CPUID_CHUNK_SIZE, (off_t)idx) != CPUID_CHUNK_SIZE) {
		if (eax != NULL)
			*eax = 0;
		if (ebx != NULL)
			*ebx = 0;
		if (ecx != NULL)
			*ecx = 0;
		if (edx != NULL)
			*edx = 0;
		return -1;
	}
	if (eax != NULL)
		memcpy(eax, buffer, 4);
	if (ebx != NULL)
		memcpy(ebx, buffer + 4, 4);
	if (ecx != NULL)
		memcpy(ecx, buffer + 8, 4);
	if (edx != NULL)
		memcpy(edx, buffer + 12, 4);
	return 0;
}

#endif	/* __linux__ */
