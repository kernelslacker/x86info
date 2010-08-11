/*
 *  (C) 2000, 2001 Dave Jones.
 *  Fixes by Arjan van de Ven (arjanv@redhat.com) and
 *  Philipp Rumpf (prumpf@mandrakesoft.com)
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Routines for retrieving cpuid registers.
 */

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

#if defined(__FreeBSD__)
# include <sys/ioctl.h>
# include <cpu.h>
#endif

#include "x86info.h"

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

/* returns zero on success */
static int native_cpuid(unsigned int cpunr, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx)
{
	cpu_set_t set, tmp_set;
	unsigned int a = 0, b = 0, c = 0, d = 0;
	int ret;

	if (eax != NULL)
		a = *eax;
	if (ebx != NULL)
		b = *ebx;
	if (ecx != NULL)
		c = *ecx;
	if (edx != NULL)
		d = *edx;

	ret = sched_getaffinity(getpid(), sizeof(set), &set);
	if (ret)
		return ret;

	/* man CPU_SET(3): To duplicate a CPU set, use memcpy(3) */
	memcpy(&tmp_set, &set, sizeof(cpu_set_t));
	CPU_ZERO(&set);
	CPU_SET(cpunr, &set);
	ret = sched_setaffinity(getpid(), sizeof(set), &set);
	if (ret)
		return ret;

	asm("cpuid"
		: "=a" (a),
		  "=b" (b),
		  "+c" (c),
		  "=d" (d)
		: "0" ((unsigned int)idx));

	if (eax!=NULL)
		*eax = a;
	if (ebx!=NULL)
		*ebx = b;
	if (ecx!=NULL)
		*ecx = c;
	if (edx!=NULL)
		*edx = d;

	/* Restore initial sched affinity */
	ret = sched_setaffinity(getpid(), sizeof(tmp_set), &tmp_set);
	if (ret)
		return ret;
	return 0;
}

static const char *NATIVE_CPUID_FAILED_MSG = "WARNING: Native cpuid failed\n";

#if defined(__FreeBSD__)
void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{
	static int nodriver=0;
	char cpuname[20];
	unsigned char buffer[16];
	int fh;
	cpu_cpuid_args_t args;

	if (nodriver == 1) {
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);
		return;
	}

	args.level = idx;
	/* Ok, use the /dev/CPU interface in preference to the _up code. */
	(void)snprintf(cpuname,18, "/dev/cpu%d", CPU_number);
	fh = open(cpuname, O_RDONLY);
	if (fh != -1) {
		if (ioctl(fh, CPU_CPUID, &args) != 0) {
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
		if (!silent && nrCPUs != 1)
			perror(cpuname);
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx)) {
			printf("%s", NATIVE_CPUID_FAILED_MSG);
			used_UP = 1;
		}
			used_UP = 1;

		return;
	}
}

#else /* !__FreeBSD__ */

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

	if (eax != NULL) {
		*eax = (unsigned int) idx;
		if (*eax == 4)
			*ecx = idx >> 32;
	}

	if (nodriver == 1) {
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx))
			printf("%s", NATIVE_CPUID_FAILED_MSG);
		return;
	}

	memset(cpuname, 0, sizeof(cpuname));
#ifdef __sun__
	/* Solaris doesn't (yet) have per-CPU interface */
	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpu/self/cpuid");
#else
	/* Ok, use the /dev/cpu interface in preference to the _up code. */
	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpu/%d/cpuid", CPU_number);
#endif
	fh = open(cpuname, O_RDONLY);
	if (fh != -1) {
#ifndef S_SPLINT_S
		lseek64(fh, (off64_t)idx, SEEK_CUR);
#endif
		if (read(fh, &buffer[0], CPUID_CHUNK_SIZE) == -1) {
			perror(cpuname);
			exit(EXIT_FAILURE);
		}
		if (eax!=0)	*eax = (*(unsigned *)(buffer   ));
		if (ebx!=0)	*ebx = (*(unsigned *)(buffer+ 4));
		if (ecx!=0)	*ecx = (*(unsigned *)(buffer+ 8));
		if (edx!=0)	*edx = (*(unsigned *)(buffer+12));
		if (close(fh) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}
	} else {
		/* Something went wrong, just do UP and hope for the best. */
		nodriver = 1;
		if (native_cpuid(CPU_number, idx, eax,ebx,ecx,edx)) {
			printf("%s", NATIVE_CPUID_FAILED_MSG);
			used_UP = 1;
		}
		return;
	}
}

#endif /* __FreeBSD__ */

void cpuid4(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
	cpuid(CPU_number, 4 | (idx << 32), eax, ebx, ecx, edx);
}

/* Some CPUID calls want 'count' to be placed in ecx */
void cpuid_count(unsigned int CPU_number, unsigned int op, int count,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx)
{
	*ecx = count;
	cpuid(CPU_number, op, eax, ebx, ecx, edx);
}

unsigned int cpuid_ebx(unsigned int CPU_number, unsigned int op)
{
	unsigned int eax, ebx, ecx, edx;

	cpuid(CPU_number, op, &eax, &ebx, &ecx, &edx);

	return ebx;
}
