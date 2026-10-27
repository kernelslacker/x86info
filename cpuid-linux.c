/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Linux specific routines for retrieving cpuid registers.
 */

#if defined(__linux__) && !defined(CPUID_REPLAY)

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <string.h>
#include <unistd.h>
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
	unsigned int regs[4] = { 0, 0, 0, 0 };
	ssize_t n;

	/* Use the /dev/cpu interface in preference to the _up code. */
	n = cpudev_pread(cpuid_fds, CPU_number, "cpuid", regs, CPUID_CHUNK_SIZE,
			 (off_t)idx);

	/* No device for this CPU, execute CPUID on it directly. */
	if (n == -1 && errno == ENODEV)
		return native_cpuid(CPU_number, idx, eax,ebx,ecx,edx);

	if (n != CPUID_CHUNK_SIZE) {
		memset(regs, 0, sizeof(regs));
		cpuid_store(eax, ebx, ecx, edx, regs);
		return -1;
	}
	cpuid_store(eax, ebx, ecx, edx, regs);
	return 0;
}

#endif	/* __linux__ */
