/*
 *  $Id: cpuid.c,v 1.12 2005/04/25 20:54:12 davej Exp $
 *	This file is part of x86info
 *	(C) 2000, 2001 Dave Jones.
 *	Fixes by Arjan van de Ven (arjanv@redhat.com) and
 *	Philipp Rumpf (prumpf@mandrakesoft.com)
 *
 *	Licensed under the terms of the GNU GPL License version 2.
 *
 */

#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "x86info.h"

void cpuid (int CPU_number, unsigned int idx,
	unsigned long *eax,
	unsigned long *ebx,
	unsigned long *ecx,
	unsigned long *edx)
{
	static int nodriver=0;
	char cpuname[20];
	unsigned char buffer[16];
	int fh;

	if (nodriver==1) {
		cpuid_UP(idx, eax, ebx, ecx, edx);
		return;
	}

	/* Ok, use the /dev/CPU interface in preference to the _up code. */
	(void)snprintf(cpuname,18, "/dev/cpu/%d/cpuid", CPU_number);
	fh = open(cpuname, O_RDONLY);
	if (fh != -1) {
#ifndef S_SPLINT_S
		lseek64(fh, (off64_t)idx, SEEK_CUR);
#endif
		if (read(fh, &buffer[0], 16) == -1) {
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
		if (!silent && nrCPUs != 1)
			perror(cpuname);
		used_UP = 1;
		cpuid_UP (idx, eax, ebx, ecx, edx);
		return;
	}
}
