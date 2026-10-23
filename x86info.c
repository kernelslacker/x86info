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

unsigned int user_is_root = 0;

unsigned int nrCPUs = 1;


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
		if (hi >= 65536 || n + (hi - lo + 1) > 65536)
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

/* Online CPU ids, which need not be contiguous. */
static unsigned int *get_online_cpus(unsigned int *count)
{
#ifdef CPUID_REPLAY
	/* A replayed dump describes exactly one CPU */
	return parse_cpu_list("0", count);
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

		if (n < 1 || n > 65535) {
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


static void display_detailed_info(struct cpudata *cpu)
{
	show_info(cpu);

	if (opts.show_cpuid) {
		dump_raw_cpuid(cpu->number, 0, cpu->cpuid_level);
		if (cpu->maxei >=0x80000000)
			dump_raw_cpuid(cpu->number, 0x80000000, cpu->maxei);

		if (cpu->maxei2 >=0xC0000000)
			dump_raw_cpuid(cpu->number, 0xC0000000, cpu->maxei2);
	}

	if (opts.show_cacheinfo) {
		switch (cpu->vendor) {
		case VENDOR_INTEL:
			decode_intel_caches(cpu, 1);
			break;
		case VENDOR_AMD:
			decode_amd_cacheinfo(cpu);
			break;

		case VENDOR_CENTAUR:
		case VENDOR_CYRIX:
		case VENDOR_NATSEMI:
		case VENDOR_RISE:
		case VENDOR_SIS:
		case VENDOR_TRANSMETA:
		case VENDOR_UNKNOWN:
		default:
			break;
		}
	}

	if (opts.show_feature_flags)
		display_feature_flags(cpu);

	if (opts.show_connector)
		decode_connector(cpu->connector);

	/* Info that requires root access (eg, reading MSRs etc) */
	if (user_is_root) {
		if (opts.show_mtrr)
			dump_mtrrs(cpu);

		if (opts.show_apic)
			dump_apics(cpu);
	}

	if (opts.show_addr_sizes)
		printf("Address sizes : %u bits physical, %u bits virtual\n",
			cpu->phyaddr_bits, cpu->viraddr_bits);

	if (opts.show_MHz)
		display_MHz_line(cpu);
}

/*
 * check to see if all CPUs are the same.
 * returns 1 if all cpus are the same, 0 if something is different
 */
static int check_cpu_similarity(struct cpudata *cpus)
{
	struct cpudata *first = &cpus[0];
	unsigned int i;

	/* force to display all cpus if the user requested it. */
	if (opts.all_cpus)
		return 0;

	if (nrCPUs == 1)
		return 1;

	for (i = 1; i < nrCPUs; i++) {
		struct cpudata *c = &cpus[i];

		if (c->efamily != first->efamily)
			return 0;
		if (c->family != first->family)
			return 0;
		if (model(c) != model(first))
			return 0;
		if (c->stepping != first->stepping)
			return 0;
		if (c->core_type != first->core_type)
			return 0;
	}

	return 1;
}

static void fill_in_cpu_info(struct cpudata *cpu)
{
	bind_cpu(cpu->number);
	get_cpu_info_basics(cpu);	/* get vendor,family,model,stepping */
	get_core_type(cpu);
	cpu->MHz = 0;
	get_feature_flags(cpu);
	identify(cpu);
	get_topology(cpu);
}


int main(int argc, char **argv)
{
	struct cpudata *cpus;
	unsigned int *online;
	unsigned int i;
	unsigned int display_one_cpu = 1;
	int nrSMPCPUs;

	if (getuid() == 0)
		user_is_root = 1;

	parse_command_line(argc, argv);

	printf("x86info " __stringify(VERSION) "\n");

	if ((HaveCPUID()) == 0) {
		printf("No CPUID instruction available.\n");
		printf("No further information available for this CPU.\n");
		exit(EXIT_SUCCESS);
	}

	if (opts.need_root && !user_is_root) {
		printf("Need to be root to use specified options.\n");
		exit(EXIT_FAILURE);
	}

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

	for (i = 0; i < nrCPUs; i++) {
		cpus[i].number = online[i];
		fill_in_cpu_info(&cpus[i]);
	}
	free(online);

	clear_uniform_core_types(cpus, nrCPUs);
	if (opts.show_MHz)
		estimate_MHz_cpus(cpus, nrCPUs);
	display_one_cpu = check_cpu_similarity(cpus);

	if (opts.show_mptable && user_is_root)
		display_mptable();

	/* Now we display the info we gathered */
	if (display_one_cpu) {
		if (nrCPUs >= 2)
			printf("Found %u identical CPUs", nrCPUs);

		/* Check mptable if present. This way we get number of CPUs
		   on SMP systems that have booted UP kernels. */
		if (user_is_root) {
			nrSMPCPUs = enumerate_cpus();
			if (nrSMPCPUs > 0)
				if ((unsigned int)nrSMPCPUs > nrCPUs)
					printf(" (but found %d CPUs in MPTable!)", nrSMPCPUs);
		}
		if (nrCPUs >= 2)
			printf("\n");

		display_detailed_info(&cpus[0]);
	} else {
		printf("Found %u CPUs.\n", nrCPUs);
		for (i = 0; i < nrCPUs; i++) {
			printf("CPU #%u:\n", i+1);

			display_detailed_info(&cpus[i]);

			if (nrCPUs > 1)
				separator();
		}
	}

	display_topology(&cpus[0]);

	free(cpus);

	exit(EXIT_SUCCESS);
}
