/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  CPUID replay backend for tests: answers cpuid() from a dump in the
 *  format printed by 'x86info -r', named by $X86INFO_CPUID_DUMP.
 *
 *  A dump from 'x86info -r --all-cpus' has a "CPU #N:" line before each
 *  CPU's leaves. Leaves before the first such line apply to every CPU.
 */

#ifdef CPUID_REPLAY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86info.h>

#define ALL_CPUS	(-1)

struct replay_entry {
	int cpu;
	unsigned int leaf, sub;
	unsigned int regs[4];
};

static struct replay_entry *entries;
static unsigned int nr_entries;

static unsigned int *cpu_ids;
static unsigned int nr_cpu_ids;

static void replay_add_cpu(unsigned int cpu)
{
	unsigned int *tmp;
	unsigned int i;

	for (i = 0; i < nr_cpu_ids; i++)
		if (cpu_ids[i] == cpu)
			return;

	tmp = realloc(cpu_ids, (nr_cpu_ids + 1) * sizeof(*cpu_ids));
	if (!tmp) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	cpu_ids = tmp;
	cpu_ids[nr_cpu_ids++] = cpu;
}

static void replay_load(void)
{
	static int loaded;
	const char *path = getenv("X86INFO_CPUID_DUMP");
	char line[256];
	int current = ALL_CPUS;
	FILE *f;

	if (loaded)
		return;
	loaded = 1;

	if (!path) {
		fprintf(stderr, "X86INFO_CPUID_DUMP is not set\n");
		exit(EXIT_FAILURE);
	}
	f = fopen(path, "r");
	if (!f) {
		perror(path);
		exit(EXIT_FAILURE);
	}

	while (fgets(line, sizeof(line), f)) {
		struct replay_entry e = { .cpu = current };
		struct replay_entry *tmp;
		unsigned int cpu;
		int len = 0;

		if (sscanf(line, "CPU #%u:%n", &cpu, &len) >= 1 && len) {
			current = cpu;
			replay_add_cpu(cpu);
			continue;
		}

		if (sscanf(line, "eax in: 0x%x, ecx in: 0x%x, eax = %x ebx = %x ecx = %x edx = %x",
			   &e.leaf, &e.sub, &e.regs[0], &e.regs[1], &e.regs[2], &e.regs[3]) != 6 &&
		    sscanf(line, "eax in: 0x%x, eax = %x ebx = %x ecx = %x edx = %x",
			   &e.leaf, &e.regs[0], &e.regs[1], &e.regs[2], &e.regs[3]) != 5)
			continue;

		tmp = realloc(entries, (nr_entries + 1) * sizeof(*entries));
		if (!tmp) {
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		entries = tmp;
		entries[nr_entries++] = e;
	}
	fclose(f);
}

int bind_cpu(unsigned int cpunr)
{
	(void)cpunr;
	return 0;
}

int read_msr(int cpu, unsigned int idx, unsigned long long *val)
{
	(void)cpu;
	(void)idx;
	(void)val;
	return 0;
}

/* The CPUs the dump describes: those it has sections for, else CPU 0. */
unsigned int *replay_cpu_ids(unsigned int *count)
{
	unsigned int n;
	unsigned int *ids;

	replay_load();

	n = nr_cpu_ids ? nr_cpu_ids : 1;
	ids = calloc(n, sizeof(*ids));
	if (!ids)
		return NULL;
	if (nr_cpu_ids)
		memcpy(ids, cpu_ids, n * sizeof(*ids));
	*count = n;
	return ids;
}

int cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx)
{
	unsigned int leaf = (unsigned int)idx;
	unsigned int sub = (unsigned int)(idx >> 32);
	unsigned int regs[4] = { 0, 0, 0, 0 };
	unsigned int i;

	replay_load();

	/* A leaf in the CPU's own section wins over a shared one. */
	for (i = 0; i < nr_entries; i++) {
		if (entries[i].leaf != leaf || entries[i].sub != sub)
			continue;
		if (entries[i].cpu == (int)CPU_number) {
			memcpy(regs, entries[i].regs, sizeof(regs));
			break;
		}
		if (entries[i].cpu == ALL_CPUS)
			memcpy(regs, entries[i].regs, sizeof(regs));
	}

	cpuid_store(eax, ebx, ecx, edx, regs);
	return 0;
}

/*
 * Nothing about the machine running the replay may reach the output: it
 * is never root, and what a dump doesn't record is reported as unavailable.
 */
bool is_root(void)
{
	return false;
}

int host_longnop(void)
{
	return -1;
}

int host_microcode_version(unsigned int cpu, unsigned long long *ver)
{
	(void)cpu;
	(void)ver;
	return 0;
}

int host_can_measure(void)
{
	return 0;
}

#endif	/* CPUID_REPLAY */
