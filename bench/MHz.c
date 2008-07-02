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
#include "bench.h"

static volatile int nosignal = 0;

static void sighandler(int sig __attribute__((unused)))
{
	nosignal = 1;
}

void estimate_MHz(struct cpudata *cpu)
{
	cpu_set_t set;
	unsigned long long int cycles[2]; /* gotta be 64 bit */
	unsigned int eax, ebx, ecx, edx;
	unsigned long r;

	/* Make sure we have a TSC (and hence RDTSC) */
	cpuid(cpu->number, 1, &eax, &ebx, &ecx, &edx);
	if ((edx & (1<<4)) == 0) {
		printf("No TSC, MHz calculation cannot be performed.\n");
		cpu->MHz = 0;
		return;
	}

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

	alarm(1);
	while (!nosignal)
		r = r * rand();

	nosignal = 0;

	cycles[1] = rdtsc();

	cpu->MHz = (cycles[1] - cycles[0]) / 1000000;

	if ((cpu->MHz % 50) > 15)
		cpu->MHz = ((cpu->MHz / 50) * 50) + 50;
	else
		cpu->MHz = ((cpu->MHz / 50) * 50);
}
