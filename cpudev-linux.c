/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Linux specific routines for caching /dev/cpu/N/ descriptors.
 */

#if defined(__linux__) && !defined(CPUID_REPLAY)

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <x86info.h>

/*
 * Open or fetch the cached descriptor. Returns -1 if there is none, which
 * leaves the slot CPUDEV_CLOSED when only descriptors ran out.
 */
static int cpudev_fd(int *fds, unsigned int cpu, const char *node)
{
	static unsigned int reported;
	unsigned int bit = strcmp(node, "msr") == 0 ? 2 : 1;
	char path[48];
	int fh, err;

	if (cpu >= MAX_CPUS)
		return -1;
	if (fds[cpu] > CPUDEV_CLOSED)
		return fds[cpu] - 1;
	if (fds[cpu] == CPUDEV_FAILED)
		return -1;

	(void)snprintf(path, sizeof(path), "/dev/cpu/%u/%s", cpu, node);
	fh = open(path, O_RDONLY);
	if (fh != -1) {
		fds[cpu] = fh + 1;
		return fh;
	}

	err = errno;
	if (err == EMFILE || err == ENFILE)
		return -1;

	fds[cpu] = CPUDEV_FAILED;
	if (err == ENOENT) {
		/* Called from the MHz measuring threads too; test-and-set once. */
		if (!(__atomic_fetch_or(&reported, bit, __ATOMIC_RELAXED) & bit))
			fprintf(stderr, "%s: %s. Try 'modprobe %s'.\n", path,
				strerror(err), node);
	} else if (err != EACCES && err != EPERM) {
		errno = err;
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
