/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Linux specific routines for caching /dev/cpu/N/ descriptors.
 */

#if defined(__linux__) && !defined(CPUID_REPLAY)

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <x86info.h>

static void cpudev_close_all(int *fds)
{
	unsigned int i;

	for (i = 0; i < MAX_CPUS; i++) {
		if (fds[i] >= 0)
			close(fds[i]);
		if (fds[i] != CPUDEV_FAILED)
			fds[i] = CPUDEV_CLOSED;
	}
}

int cpudev_fd(int *fds, unsigned int cpu, const char *node)
{
	static int reported_module;
	char path[48];
	struct stat st;
	int fh;

	if (cpu >= MAX_CPUS)
		return -1;
	if (fds[cpu] >= 0)
		return fds[cpu];
	if (fds[cpu] == CPUDEV_FAILED)
		return -1;

	(void)snprintf(path, sizeof(path), "/dev/cpu/%u/%s", cpu, node);
	fh = open(path, O_RDONLY);
	if (fh == -1 && (errno == EMFILE || errno == ENFILE)) {
		cpudev_close_all(fds);
		fh = open(path, O_RDONLY);
	}
	if (fh != -1) {
		fds[cpu] = fh;
		return fh;
	}

	if (errno == EMFILE || errno == ENFILE)
		return -1;

	fds[cpu] = CPUDEV_FAILED;
	if (errno == ENOENT && stat("/dev/cpu", &st) != 0) {
		if (!reported_module)
			fprintf(stderr, "%s: %s. Try 'modprobe %s'.\n", path,
				strerror(errno), node);
		reported_module = 1;
	} else if (errno != EACCES && errno != EPERM) {
		perror(path);
	}
	return -1;
}

#endif	/* __linux__ */
