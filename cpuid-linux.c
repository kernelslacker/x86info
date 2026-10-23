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

/*
 * sched_* calls weren't stable until 2.3.4
 * AFAIK, there's no macro to check for the .4, so we just
 * check for the next minor version up. (2.4)
 */
#ifdef __GLIBC__
#if __GLIBC__ < 2 || __GLIBC__ == 2 && __GLIBC_MINOR__ < 4
#error Need at least glibc 2.4
#endif
#endif

int bind_cpu(unsigned int cpunr)
{
	cpu_set_t *set;
	size_t size = CPU_ALLOC_SIZE(cpunr + 1);
	int ret;

	set = CPU_ALLOC(cpunr + 1);
	if (!set)
		return -1;

	CPU_ZERO_S(size, set);
	CPU_SET_S(cpunr, size, set);
	ret = sched_setaffinity(getpid(), size, set);
	CPU_FREE(set);
	return ret;
}

static const char *NATIVE_CPUID_FAILED_MSG = "WARNING: Native cpuid failed\n";

/* Kernel CPUID driver's minimum supported read size
 * (see linux/arch/i386/kernel/cpuid.c)
 */
#define CPUID_CHUNK_SIZE (16)

#define MAX_CPUS 8192

/* Per-CPU /dev/cpu/N/cpuid descriptors, opened lazily and stored as fd + 1. */
static int cpuid_fds[MAX_CPUS];

static int cpuid_fd(unsigned int cpunr)
{
	char cpuname[32];
	int fh;

	if (cpunr >= MAX_CPUS)
		return -1;
	if (cpuid_fds[cpunr])
		return cpuid_fds[cpunr] - 1;

	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpu/%u/cpuid", cpunr);
	fh = open(cpuname, O_RDONLY);
	if (fh != -1)
		cpuid_fds[cpunr] = fh + 1;
	else if (errno == ENOENT)
		fprintf(stderr, "%s: %s. Try 'modprobe cpuid'; using native cpuid.\n",
			cpuname, strerror(errno));
	return fh;
}

void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{
	static int nodriver=0;
	unsigned char buffer[CPUID_CHUNK_SIZE];
	int fh = -1;

	/* Use the /dev/cpu interface in preference to the _up code. */
	if (nodriver == 0)
		fh = cpuid_fd(CPU_number);

	/* Something went wrong, just do UP and hope for the best. */
	if (fh == -1) {
		nodriver = 1;
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);
		return;
	}

	memset(buffer, 0, sizeof(buffer));
	if (pread(fh, buffer, CPUID_CHUNK_SIZE, (off_t)idx) != CPUID_CHUNK_SIZE)
		printf("cpuid read error: %s\n", strerror(errno));
	if (eax != NULL)
		memcpy(eax, buffer, 4);
	if (ebx != NULL)
		memcpy(ebx, buffer + 4, 4);
	if (ecx != NULL)
		memcpy(ecx, buffer + 8, 4);
	if (edx != NULL)
		memcpy(edx, buffer + 12, 4);
}

#endif	/* __linux__ */
