/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  CPUID replay backend for tests: answers cpuid() from a dump in the
 *  format printed by 'x86info -r', named by $X86INFO_CPUID_DUMP.
 *
 *  A dump from 'x86info -r --all-cpus' has a "CPU #N:" line before each
 *  CPU's leaves. Leaves before the first such line apply to every CPU.
 *  An optional "MHz: N" line gives the TSC rate a live run would measure.
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
	unsigned int seq;
	unsigned int regs[4];
};

static struct replay_entry *entries;
static unsigned int nr_entries;

struct replay_mhz {
	int cpu;
	unsigned int mhz;
};

static struct replay_mhz *rates;
static unsigned int nr_rates;

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

static int entry_cmp(const void *a, const void *b)
{
	const struct replay_entry *x = a, *y = b;

	if (x->leaf != y->leaf)
		return x->leaf < y->leaf ? -1 : 1;
	if (x->sub != y->sub)
		return x->sub < y->sub ? -1 : 1;
	if (x->cpu != y->cpu)
		return x->cpu < y->cpu ? -1 : 1;
	/* qsort isn't stable; file order decides between duplicates. */
	if (x->seq != y->seq)
		return x->seq < y->seq ? -1 : 1;
	return 0;
}

/* First entry not ordered before (leaf, sub, cpu). */
static unsigned int entry_lower_bound(unsigned int leaf, unsigned int sub, int cpu)
{
	struct replay_entry key = { .cpu = cpu, .leaf = leaf, .sub = sub };
	unsigned int lo = 0, hi = nr_entries;

	while (lo < hi) {
		unsigned int mid = lo + (hi - lo) / 2;

		if (entry_cmp(&entries[mid], &key) < 0)
			lo = mid + 1;
		else
			hi = mid;
	}
	return lo;
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
		unsigned int cpu, mhz;
		int len = 0;

		if (sscanf(line, "CPU #%u:%n", &cpu, &len) >= 1 && len) {
			current = cpu;
			replay_add_cpu(cpu);
			continue;
		}

		if (sscanf(line, "MHz: %u", &mhz) == 1) {
			struct replay_mhz *r;

			r = realloc(rates, (nr_rates + 1) * sizeof(*rates));
			if (!r) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
			rates = r;
			rates[nr_rates].cpu = current;
			rates[nr_rates++].mhz = mhz;
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
		e.seq = nr_entries;
		entries[nr_entries++] = e;
	}
	fclose(f);

	if (nr_entries)
		qsort(entries, nr_entries, sizeof(*entries), entry_cmp);
}

int bind_cpu(unsigned int cpunr)
{
	(void)cpunr;
	return 0;
}

bool read_msr(unsigned int cpu, unsigned int idx, unsigned long long *val)
{
	(void)cpu;
	(void)idx;
	(void)val;
	return false;
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

	/*
	 * A leaf in the CPU's own section wins (first duplicate), else the
	 * last shared one.
	 */
	i = entry_lower_bound(leaf, sub, (int)CPU_number);
	if (i < nr_entries && entries[i].leaf == leaf && entries[i].sub == sub &&
	    entries[i].cpu == (int)CPU_number) {
		memcpy(regs, entries[i].regs, sizeof(regs));
	} else {
		/* Shared entries (cpu -1) sort just before CPU 0's. */
		i = entry_lower_bound(leaf, sub, 0);
		if (i > 0 && entries[i - 1].leaf == leaf &&
		    entries[i - 1].sub == sub && entries[i - 1].cpu == ALL_CPUS)
			memcpy(regs, entries[i - 1].regs, sizeof(regs));
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

unsigned int host_recorded_MHz(unsigned int cpu)
{
	unsigned int i, mhz = 0;

	replay_load();
	/* A rate in the CPU's own section wins over a shared one. */
	for (i = 0; i < nr_rates; i++) {
		if (rates[i].cpu == (int)cpu)
			return rates[i].mhz;
		if (rates[i].cpu == ALL_CPUS)
			mhz = rates[i].mhz;
	}
	return mhz;
}

#endif	/* CPUID_REPLAY */
