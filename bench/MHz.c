/*
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#define _GNU_SOURCE
#define __USE_GNU
#include <sched.h>
#include <string.h>
#include <signal.h>

#include "../x86info.h"

static unsigned long long int rdtsc(void)
{
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}

static volatile int nosignal = 0;

static void sighandler(int sig __attribute__((unused)))
{
	nosignal = 1;
}

void estimate_MHz(struct cpudata *cpu)
{
	cpu_set_t set;
	struct timezone tz;
	struct timeval tvstart, tvstop;
	unsigned long long int cycles[2]; /* gotta be 64 bit */
	unsigned long microseconds; /* total time taken */
	unsigned int eax, ebx, ecx, edx;
	unsigned long freq = 1;
	unsigned long i;

	/* Make sure we have a TSC (and hence RDTSC) */
	cpuid(cpu->number, 1, &eax, &ebx, &ecx, &edx);
	if ((edx & (1<<4)) == 0) {
		printf("No TSC, MHz calculation cannot be performed.\n");
		cpu->MHz = 0;
		return;
	}

	memset(&tz, 0, sizeof(tz));

	if (sched_getaffinity(getpid(), sizeof(set), &set) == 0) {
		CPU_ZERO(&set);
		CPU_SET(cpu->number, &set);
		sched_setaffinity(getpid(), sizeof(set), &set);
	}

	if (signal(SIGALRM, sighandler) == SIG_ERR) {
		printf("Some kind of signal failure.\n");
		return;
	}

	cycles[0] = rdtsc();
	gettimeofday(&tvstart, &tz);

	alarm(3);
	while (!nosignal) {
		i = i * rand();
	}

	cycles[1] = rdtsc();
	gettimeofday(&tvstop, &tz);

	microseconds = ((tvstop.tv_sec-tvstart.tv_sec)*1000000) +
		(tvstop.tv_usec-tvstart.tv_usec);

	cpu->MHz = (int) (cycles[1]-cycles[0]) / (microseconds/freq);

	if ((cpu->MHz % 50) > 15)
		cpu->MHz = ((cpu->MHz / 50) * 50) + 50;
	else
		cpu->MHz = ((cpu->MHz / 50) * 50);

	nosignal = 0;
}
