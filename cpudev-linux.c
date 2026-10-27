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

/*
 * Open or fetch the cached descriptor. Returns -1 if there is none, which
 * leaves the slot CPUDEV_CLOSED when only descriptors ran out.
 */
static int cpudev_fd(int *fds, unsigned int cpu, const char *node)
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

ssize_t cpudev_pread(int *fds, unsigned int cpu, const char *node,
		     void *buf, size_t len, off_t off)
{
	char path[48];
	ssize_t ret;
	int fh;

	fh = cpudev_fd(fds, cpu, node);
	if (fh >= 0)
		return pread(fh, buf, len, off);

	if (cpu >= MAX_CPUS || fds[cpu] == CPUDEV_FAILED) {
		errno = ENODEV;
		return -1;
	}

	/* Out of descriptors: use a transient one rather than evict another. */
	(void)snprintf(path, sizeof(path), "/dev/cpu/%u/%s", cpu, node);
	fh = open(path, O_RDONLY);
	if (fh == -1)
		return -1;
	ret = pread(fh, buf, len, off);
	close(fh);
	return ret;
}

#endif	/* __linux__ */
