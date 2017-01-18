/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  NetBSD Routines for reading MSRs.
 */

#if defined(__NetBSD__)

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <x86info.h>

int read_msr(__attribute__((__unused__))int cpu,
    __attribute__((__unused__))unsigned int idx,
    __attribute__((__unused__))unsigned long long *val)
{

	/* Currently NetBSD has no API to read/write MSRs. */
	return 0;
}

#endif /* __NetBSD__ */
