/*
 *  $Id: MHz.c,v 1.7 2001/11/19 20:09:21 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 * Estimate CPU MHz routine by Andrea Arcangeli <andrea@suse.de>
 * Small changes by David Sterba <sterd9am@ss1000.ms.mff.cuni.cz>
 *
 */

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include "x86info.h"

__inline__ unsigned long long int rdtsc()
{
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}

void estimate_MHz(int cpunum)
{
	struct timezone tz;
	struct timeval tvstart, tvstop;
	unsigned long long int cycles[2]; /* gotta be 64 bit */
	unsigned int microseconds; /* total time taken */
	unsigned long eax, ebx, ecx, edx;

	/* Make sure we have a TSC (and hence RDTSC) */
	cpuid (cpunum, 1, &eax, &ebx, &ecx, &edx);
	if ((edx & (1<<4))==0) {
		printf ("No TSC, MHz calculation cannot be performed.\n");
		return;
	}

	memset(&tz, 0, sizeof(tz));

	/* get this function in cached memory */
	gettimeofday(&tvstart, &tz);
	cycles[0] = rdtsc();
	gettimeofday(&tvstart, &tz);

	/* we don't trust that this is any specific length of time */
	usleep(10);
	
	cycles[1] = rdtsc();
	gettimeofday(&tvstop, &tz);
	microseconds = ((tvstop.tv_sec-tvstart.tv_sec)*1000000) +
		(tvstop.tv_usec-tvstart.tv_usec);

	printf("%lldMHz processor (estimate).\n\n",
		(cycles[1]-cycles[0])/microseconds);
}
