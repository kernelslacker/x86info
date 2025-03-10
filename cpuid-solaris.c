/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Solaris routines for retrieving cpuid registers.
 *  Originally submitted by John Levon <levon@movementarian.org>
 *
 *  Copyright 2018 Joyent, Inc.
 */

#ifdef __sun__

#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <x86info.h>
#include <sys/processor.h>
#include <sys/procset.h>


/*
 * This can easily fail if we don't have permission to bind; drive on.
 */
void bind_cpu(unsigned int cpunr)
{
	if (processor_bind(P_PID, P_MYID, cpunr, NULL) != 0) {
		fprintf(stderr, "warning: failed to bind to CPU%u: %s\n",
			cpunr, strerror(errno));
	}
}

void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{
	if (eax != NULL) {
		*eax = (unsigned int) idx;
		if (*eax == 4)
			*ecx = idx >> 32;
	}

	/*
	 * Solaris/Illumos do have /dev/cpu/self/cpuid, but caps idx at
	 * UINT_MAX, so just always use native.
	 */
	if (native_cpuid(CPU_number, idx, eax, ebx, ecx, edx))
		printf("%s", NATIVE_CPUID_FAILED_MSG);
}
#endif	/* __sun__ */
