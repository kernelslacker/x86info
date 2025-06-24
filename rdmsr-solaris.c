/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Copyright 2018 Joyent, Inc.
 */

#if defined(__sun)

#include <x86info.h>

/*
 * No userspace MSR access under Solaris or Illumos.
 */
int read_msr(__attribute__((__unused__))int cpu,
    __attribute__((__unused__))unsigned int idx,
    __attribute__((__unused__))unsigned long long *val)
{
	return 0;
}

#endif /* __sun */
