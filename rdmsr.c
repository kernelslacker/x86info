/*
 *  $Id: rdmsr.c,v 1.5 2001/08/18 23:59:05 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Contributions by Arjan van de Ven & Philipp Rumpf.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "x86info.h"

int rdmsr(int cpu, unsigned int index, unsigned long long *val)
{
	char cpuname[16];
	unsigned char buffer[8];
	unsigned long lo, hi;
	int fh;

	sprintf (cpuname, "/dev/cpu/%d/msr", cpu);

	fh = open (cpuname, O_RDONLY);
	if (fh==-1)
		return (0);

	lseek (fh, index, SEEK_CUR);

	if (fh != -1) {

		if (read (fh, &buffer[0], 8) != 8) {
			close (fh);
			return (0);
		}

		lo = (*(unsigned long *)buffer);
		hi = (*(unsigned long *)(buffer+4));
		*val = hi;
		*val = *val<<32 | lo;
	}
	close (fh);
	return (1);
}


void dumpmsr (int cpu, unsigned int msr)
{
	unsigned long long val=0;

	if (rdmsr(cpu, msr, &val) == 1) {
		printf ("MSR: 0x%08x=0x%08llx\t", msr, val);
		long2binstr(val);
		return;
	}
	printf ("Couldn't read MSR 0x%x\n", msr);
}


void dumpmsr_bin (int cpu, unsigned int msr)
{
	unsigned long long val=0;

	if (rdmsr(cpu, msr, &val) == 1) {
		long2binstr(val);
		return;
	}
	printf ("Couldn't read MSR 0x%x\n", msr);
}
