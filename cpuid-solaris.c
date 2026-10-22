/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Solaris routines for retrieving cpuid registers.
 *  Originally submitted by John Levon <levon@movementarian.org>
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

int bind_cpu(unsigned int cpunr)
{
	//FIXME
	return 0;
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
	int fh;

	if (eax != NULL) {
		*eax = (unsigned int) idx;
		if (*eax == 4 && ecx != NULL)
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
		if (lseek64(fh, (off64_t)idx, SEEK_SET) < 0) {
			printf("cpuid seek error: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (read(fh, &buffer[0], CPUID_CHUNK_SIZE) == -1) {
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
#endif	/* __sun__ */
