/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Linux specific routines for retrieving cpuid registers.
 */

#ifdef __linux__

#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define __USE_GNU
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

void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{
	static int nodriver=0;
	char cpuname[20];
	unsigned char buffer[CPUID_CHUNK_SIZE];
	int fh;

	if (nodriver == 1) {
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);
		return;
	}

	memset(cpuname, 0, sizeof(cpuname));
	/* Ok, use the /dev/cpu interface in preference to the _up code. */
	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpu/%u/cpuid", CPU_number);

	fh = open(cpuname, O_RDONLY);
	if (fh != -1) {
		if (lseek64(fh, (off64_t)idx, SEEK_SET) < 0) {
			printf("cpuid seek error: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (read(fh, &buffer[0], CPUID_CHUNK_SIZE) != CPUID_CHUNK_SIZE) {
			perror(cpuname);
			exit(EXIT_FAILURE);
		}
		if (eax != NULL)
			memcpy(eax, buffer, 4);
		if (ebx != NULL)
			memcpy(ebx, buffer + 4, 4);
		if (ecx != NULL)
			memcpy(ecx, buffer + 8, 4);
		if (edx != NULL)
			memcpy(edx, buffer + 12, 4);

		if (close(fh) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}
	} else {
		/* Something went wrong, just do UP and hope for the best. */
		nodriver = 1;
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);
		return;
	}
}

#endif	/* __linux__ */
