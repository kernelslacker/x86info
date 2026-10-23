/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Probes of the machine x86info is running on, as opposed to the CPUs it
 *  is describing. Replay builds supply their own versions.
 */

#ifndef CPUID_REPLAY

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <x86info.h>

bool is_root(void)
{
	return getuid() == 0;
}

static sigjmp_buf out;

static void sigill(__attribute__((__unused__))int sig)
{
	siglongjmp(out, 1);
}

int host_longnop(void)
{
	volatile int died;
	void (*old)(int);

	old = signal(SIGILL, sigill);

	died = sigsetjmp(out, 1);

	if (!died)
		asm volatile(".byte 0x0f,0x1f,0x00 /* nopl 0(%eax) */");

	signal(SIGILL, old);

	return !died;
}

int host_microcode_version(unsigned int cpu, unsigned long long *ver)
{
	char path[80];
	FILE *f;
	int ret;

	snprintf(path, sizeof(path),
		 "/sys/devices/system/cpu/cpu%u/microcode/version", cpu);
	f = fopen(path, "r");
	if (!f)
		return 0;
	ret = fscanf(f, "%llx", ver) == 1;
	fclose(f);
	return ret;
}

int host_can_measure(void)
{
	return 1;
}

#endif	/* !CPUID_REPLAY */
