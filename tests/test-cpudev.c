/*
 * Check the /dev/cpu descriptor cache and its fallbacks, using a fake
 * device directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <x86info.h>

static int fail;
static char root[256];

static void expect(int ok, const char *what)
{
	if (!ok) {
		fprintf(stderr, "cpudev: %s\n", what);
		fail = 1;
	}
}

static void node_path(char *buf, size_t len, unsigned int cpu,
		      const char *node)
{
	snprintf(buf, len, "%s/%u/%s", root, cpu, node);
}

static int make_node(unsigned int cpu, const char *node, const char *data)
{
	char path[300];
	int fd;

	snprintf(path, sizeof(path), "%s/%u", root, cpu);
	if (mkdir(path, 0700) && errno != EEXIST)
		return -1;
	node_path(path, sizeof(path), cpu, node);
	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
		return -1;
	if (write(fd, data, strlen(data)) < 0) {
		close(fd);
		return -1;
	}
	return close(fd);
}

static void cleanup(void)
{
	static const char *const nodes[] = { "cpuid", "msr" };
	char path[300];
	unsigned int cpu, i;

	for (cpu = 0; cpu < 4; cpu++) {
		for (i = 0; i < 2; i++) {
			node_path(path, sizeof(path), cpu, nodes[i]);
			(void)unlink(path);
		}
		snprintf(path, sizeof(path), "%s/%u", root, cpu);
		(void)rmdir(path);
	}
	(void)rmdir(root);
}

static void test_cache(int *fds)
{
	char buf[8];
	int slot;

	memset(buf, 0, sizeof(buf));
	expect(cpudev_pread(fds, 0, "cpuid", buf, 4, 0) == 4 &&
	       memcmp(buf, "abcd", 4) == 0, "first read");
	slot = fds[0];
	expect(slot > CPUDEV_CLOSED, "descriptor not cached");

	memset(buf, 0, sizeof(buf));
	expect(cpudev_pread(fds, 0, "cpuid", buf, 4, 2) == 4 &&
	       memcmp(buf, "cdef", 4) == 0, "offset read");
	expect(fds[0] == slot, "cached descriptor was replaced");
	close(slot - 1);
}

static void test_enodev(int *fds)
{
	char buf[4];
	int saved = dup(STDERR_FILENO);

	/* A missing node is reported once on stderr; keep the log quiet. */
	if (saved >= 0 && freopen("/dev/null", "w", stderr) == NULL)
		fail = 1;

	errno = 0;
	expect(cpudev_pread(fds, 2, "cpuid", buf, sizeof(buf), 0) == -1 &&
	       errno == ENODEV, "missing node is not ENODEV");
	expect(fds[2] == CPUDEV_FAILED, "failure not remembered");

	errno = 0;
	expect(cpudev_pread(fds, 2, "cpuid", buf, sizeof(buf), 0) == -1 &&
	       errno == ENODEV, "remembered failure is not ENODEV");

	errno = 0;
	expect(cpudev_pread(fds, MAX_CPUS, "cpuid", buf, sizeof(buf), 0) == -1 &&
	       errno == ENODEV, "out of range CPU is not ENODEV");

	if (saved >= 0) {
		fflush(stderr);
		dup2(saved, STDERR_FILENO);
		close(saved);
		clearerr(stderr);
	}
}

static void test_emfile(int *fds)
{
	struct rlimit old, lim;
	char buf[8];
	int next;

	if (getrlimit(RLIMIT_NOFILE, &old))
		return;

	/* The next open() must fail: allow only descriptors already in use. */
	next = dup(STDIN_FILENO);
	if (next < 0)
		return;
	close(next);
	lim = old;
	lim.rlim_cur = next;
	if (setrlimit(RLIMIT_NOFILE, &lim))
		return;

	errno = 0;
	expect(cpudev_pread(fds, 1, "msr", buf, 4, 0) == -1 &&
	       errno == EMFILE, "descriptor exhaustion is not EMFILE");
	expect(fds[1] == CPUDEV_CLOSED, "EMFILE marked the slot");

	if (setrlimit(RLIMIT_NOFILE, &old)) {
		fprintf(stderr, "cpudev: can't restore RLIMIT_NOFILE\n");
		fail = 1;
		return;
	}
	memset(buf, 0, sizeof(buf));
	expect(cpudev_pread(fds, 1, "msr", buf, 4, 0) == 4 &&
	       memcmp(buf, "wxyz", 4) == 0, "no recovery after EMFILE");
	expect(fds[1] > CPUDEV_CLOSED, "recovered read not cached");
	if (fds[1] > CPUDEV_CLOSED)
		close(fds[1] - 1);
}

int main(void)
{
	static int fds[MAX_CPUS];
	const char *tmp = getenv("TMPDIR");

	if (!tmp || !*tmp)
		tmp = "/tmp";
	snprintf(root, sizeof(root), "%s/x86info-cpudev-XXXXXX", tmp);
	if (!mkdtemp(root)) {
		perror("mkdtemp");
		return 1;
	}
	cpudev_dir = root;

	if (make_node(0, "cpuid", "abcdefgh") ||
	    make_node(1, "msr", "wxyz1234")) {
		perror("cpudev: fixture");
		cleanup();
		return 1;
	}

	test_cache(fds);
	test_enodev(fds);
	test_emfile(fds);

	cleanup();
	return fail;
}
