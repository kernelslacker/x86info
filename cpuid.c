/*
 *  $Id: cpuid.c,v 1.9 2001/11/19 12:31:27 davej Exp $
 *	This file is part of x86info
 *	(C) 2000, 2001 Dave Jones.
 *	Fixes by Arjan van de Ven (arjanv@redhat.com) and
 *	Philipp Rumpf (prumpf@mandrakesoft.com)
 *
 * 	Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "x86info.h"

void cpuid (int CPU_number, int index,
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
		cpuid_UP (index, eax, ebx, ecx, edx);
		return;
	}

	/* Ok, use the /dev/CPU interface in preference to the _up code. */
	snprintf (cpuname,18, "/dev/cpu/%d/cpuid", CPU_number);
	fh = open (cpuname, O_RDONLY);
	if (fh != -1) {
		lseek (fh, index, SEEK_CUR);
		read (fh, &buffer[0], 16);
		if (eax!=0)	*eax = (*(unsigned long *)buffer);
		if (ebx!=0)	*ebx = (*(unsigned long *)(buffer+4));
		if (ecx!=0)	*ecx = (*(unsigned long *)(buffer+8));
		if (edx!=0)	*edx = (*(unsigned long *)(buffer+12));
		close (fh);
	} else {
		/* Something went wrong, just do UP and hope for the best. */
		nodriver = 1;
		if (!silent) {
			perror(cpuname);
		}
		used_UP = 1;
		cpuid_UP (index, eax, ebx, ecx, edx);
		return;
	}
}
