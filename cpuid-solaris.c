/*
 *  (C) 2000, 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Routines for retrieving cpuid registers.
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

void bind_cpu(int cpunr)
{
	//FIXME
}

void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax,
	unsigned int *ebx,
	unsigned int *ecx,
	unsigned int *edx)
{
	static int nodriver = 0;
	char cpuname[20];
	unsigned char buffer[CPUID_CHUNK_SIZE];
	unsigned int *ptr = (unsigned int *)buffer;
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
	/* Solaris doesn't (yet) have per-CPU interface */
	(void)snprintf(cpuname, sizeof(cpuname), "/dev/cpu/self/cpuid");

	fh = open(cpuname, O_RDONLY);
	if (fh != -1) {
		lseek64(fh, (off64_t)idx, SEEK_CUR);
		if (read(fh, &buffer[0], CPUID_CHUNK_SIZE) == -1) {
			perror(cpuname);
			exit(EXIT_FAILURE);
		}
		if (eax!=0)	*eax = *ptr;
		if (ebx!=0)	*ebx = *(++ptr);
		if (ecx!=0)	*ecx = *(++ptr);
		if (edx!=0)	*edx = *(++ptr);
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
#endif	/* __sun__ */
