/*
 * x86info main.
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <x86info.h>
#include "mptable.h"

#include <intel.h>
#include <amd.h>

unsigned int nrCPUs = 1;

#ifndef CPUID_REPLAY
/* Parse a kernel cpu list such as "0-3,8,10-11" into an array of ids. */
static unsigned int *parse_cpu_list(const char *s, unsigned int *count)
{
	unsigned int *ids = NULL, *tmp;
	unsigned int n = 0;
	char *end;

	while (*s && *s != '\n') {
		unsigned long lo = strtoul(s, &end, 10);
		unsigned long hi = lo;

		if (end == s)
			goto bad;
		if (*end == '-') {
			s = end + 1;
			hi = strtoul(s, &end, 10);
			if (end == s || hi < lo)
				goto bad;
		}
		if (hi >= MAX_CPUS || n + (hi - lo + 1) > MAX_CPUS)
			goto bad;
		tmp = realloc(ids, (n + (hi - lo + 1)) * sizeof(*ids));
		if (!tmp)
			goto bad;
		ids = tmp;
		for (; lo <= hi; lo++)
			ids[n++] = lo;
		s = end;
		if (*s == ',')
			s++;
		else if (*s && *s != '\n')
			goto bad;
	}
	if (n == 0)
		goto bad;
	*count = n;
	return ids;
bad:
	free(ids);
	return NULL;
}
#endif

/* Online CPU ids, which need not be contiguous. */
static unsigned int *get_online_cpus(unsigned int *count)
{
#ifdef CPUID_REPLAY
	return replay_cpu_ids(count);
#else
	char buf[4096];
	unsigned int *ids = NULL;
	FILE *f = fopen("/sys/devices/system/cpu/online", "r");

	if (f) {
		if (fgets(buf, sizeof(buf), f))
			ids = parse_cpu_list(buf, count);
		fclose(f);
	}
	if (!ids) {
		long n = sysconf(_SC_NPROCESSORS_ONLN);
		long i;

		if (n < 1 || n > MAX_CPUS) {
			printf("CPU count is bogus: defaulting to 1 CPU.\n");
			n = 1;
		}
		ids = malloc(n * sizeof(*ids));
		if (!ids)
			return NULL;
		for (i = 0; i < n; i++)
			ids[i] = i;
		*count = n;
	}
	return ids;
#endif
}

static void separator(void)
{
	int j;

	for (j = 0; j < 74; j++)
		printf("-");
	printf("\n");
}


static void display_detailed_info(struct cpudata *cpu, int header)
{
	show_info(cpu, header);

	if (opts.show_cpuid) {
		dump_raw_cpuid(cpu->number, 0, cpu->cpuid_level);
		if (cpu->maxei >=0x80000000)
			dump_raw_cpuid(cpu->number, 0x80000000, cpu->maxei);

		if (cpu->maxei2 >=0xC0000000)
			dump_raw_cpuid(cpu->number, 0xC0000000, cpu->maxei2);
	}

	if (opts.show_cacheinfo)
		show_cache_info(cpu);

	if (opts.show_feature_flags)
		display_feature_flags(cpu);

	if (opts.show_connector)
		decode_connector(cpu->connector);

	/* MTRR and APIC dumps */
	if (opts.show_mtrr)
		dump_mtrrs(cpu);

	if (opts.show_apic)
		dump_apics(cpu);

	if (opts.show_addr_sizes)
		printf("Address sizes : %u bits physical, %u bits virtual\n",
			cpu->phyaddr_bits, cpu->viraddr_bits);

	if (opts.show_MHz)
		display_MHz_line(cpu);
}

/* True if two CPUs are the same model, stepping and core type. */
static int same_cpu(const struct cpudata *a, const struct cpudata *b)
{
	return a->efamily == b->efamily && a->family == b->family &&
	       model(a) == model(b) && a->stepping == b->stepping &&
	       a->core_type == b->core_type;
}

/*
 * Set leader[i] to the index of the first CPU that is the same as cpus[i],
 * count[] to the size of each group, and return the number of groups.
 */
static unsigned int find_groups(const struct cpudata *cpus, unsigned int n,
				unsigned int *leader, unsigned int *count)
{
	unsigned int i, j, groups = 0;

	for (i = 0; i < n; i++) {
		for (j = 0; j < i; j++)
			if (leader[j] == j && same_cpu(&cpus[j], &cpus[i]))
				break;
		leader[i] = j;
		count[i] = 0;
		count[j]++;
		if (j == i)
			groups++;
	}
	return groups;
}

/* True if the groups differ only in core type, so one header serves all. */
static int groups_share_header(const struct cpudata *cpus, unsigned int n)
{
	unsigned int i;

	for (i = 1; i < n; i++) {
		const struct cpudata *a = &cpus[0], *b = &cpus[i];

		if (a->vendor != b->vendor || a->efamily != b->efamily ||
		    model(a) != model(b) || a->family != b->family ||
		    a->stepping != b->stepping || strcmp(a->name, b->name))
			return 0;
	}
	return 1;
}

/* Returns zero on success, -1 if the CPU can't be queried. */
static int fill_in_cpu_info(struct cpudata *cpu)
{
	if (bind_cpu(cpu->number))
		return -1;
	if (get_cpu_info_basics(cpu))	/* get vendor,family,model,stepping */
		return -1;
	get_core_type(cpu);
	cpu->MHz = 0;
	get_feature_flags(cpu);
	identify(cpu);
	get_topology(cpu);
	return 0;
}


/* Query every online CPU; sets nrCPUs to the number that could be read. */
static struct cpudata *collect_cpus(void)
{
	struct cpudata *cpus;
	unsigned int *online;
	unsigned int i, n;

	online = get_online_cpus(&nrCPUs);
	if (!online) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	/* Allocate a flat array of CPUs */
	cpus = calloc(nrCPUs, sizeof(struct cpudata));
	if (!cpus) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	for (i = 0, n = 0; i < nrCPUs; i++) {
		cpus[n].number = online[i];
		if (fill_in_cpu_info(&cpus[n])) {
			fprintf(stderr, "CPU #%u: unable to read CPUID, skipping.\n",
				online[i]);
			memset(&cpus[n], 0, sizeof(cpus[n]));
			continue;
		}
		n++;
	}
	free(online);
	if (n == 0) {
		fprintf(stderr, "No usable CPUs found.\n");
		exit(EXIT_FAILURE);
	}
	nrCPUs = n;

	clear_uniform_core_types(cpus, nrCPUs);
	return cpus;
}

static void display_cpus(struct cpudata *cpus, unsigned int n)
{
	unsigned int *leader, *group_size;
	unsigned int i, nr_groups;
	int display_one_cpu;
	int nrSMPCPUs;

	leader = malloc(n * sizeof(*leader));
	group_size = malloc(n * sizeof(*group_size));
	if (!leader || !group_size) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	nr_groups = find_groups(cpus, n, leader, group_size);
	if (opts.show_MHz)
		estimate_MHz_cpus(cpus, n, leader);
	display_one_cpu = nr_groups == 1 && !opts.all_cpus;

	if (opts.show_mptable && is_root())
		display_mptable();

	/* Now we display the info we gathered */
	if (display_one_cpu) {
		if (n >= 2)
			printf("Found %u identical CPUs\n", n);

		/* Check mptable if present. This way we get number of CPUs
		   on SMP systems that have booted UP kernels. */
		if (is_root()) {
			nrSMPCPUs = enumerate_cpus();
			if (nrSMPCPUs > 0 && (unsigned int)nrSMPCPUs > n)
				printf("Found %d CPUs in MPTable!\n", nrSMPCPUs);
		}

		display_detailed_info(&cpus[0], 1);
	} else if (!opts.all_cpus) {
		int common = groups_share_header(cpus, n);

		printf("Found %u CPUs of %u kinds.\n", n, nr_groups);
		if (common) {
			printf("\n");
			show_header(&cpus[0]);
		}
		for (i = 0; i < n; i++) {
			if (leader[i] != i)
				continue;
			printf("CPU #%u (%u in this group):\n", cpus[i].number,
			       group_size[i]);

			display_detailed_info(&cpus[i], !common);
		}
	} else {
		printf("Found %u CPUs.\n", n);
		for (i = 0; i < n; i++) {
			printf("CPU #%u:\n", cpus[i].number);

			display_detailed_info(&cpus[i], 1);

			if (n > 1)
				separator();
		}
	}

	display_topology(&cpus[0], n);

	free(leader);
	free(group_size);
}

int main(int argc, char **argv)
{
	struct cpudata *cpus;

	parse_command_line(argc, argv);

	printf("x86info " __stringify(VERSION) "\n");

	if ((HaveCPUID()) == 0) {
		printf("No CPUID instruction available.\n");
		printf("No further information available for this CPU.\n");
		exit(EXIT_SUCCESS);
	}

	if (opts.need_root && !is_root()) {
		printf("Need to be root to use specified options.\n");
		exit(EXIT_FAILURE);
	}

	cpus = collect_cpus();
	display_cpus(cpus, nrCPUs);
	free(cpus);

	exit(EXIT_SUCCESS);
}
