/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86info.h>
#include <intel.h>

#define LEVEL_TYPE_SMT	1

/* Mask of the low 'bits' bits, safe for bits >= 32. */
static unsigned int low_mask(unsigned int bits)
{
	return bits >= 32 ? ~0u : (1u << bits) - 1;
}

/*
 * Decode the extended topology leaf (0x1F, else 0xB). Each subleaf gives
 * the shift to the next level of the 32-bit x2APIC ID and how many logical
 * processors sit at or below that level.
 */
static int get_x2apic_topology(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx, sub, type;
	unsigned int leaf, smt_shift = 0, pkg_shift = 0;
	unsigned int smt_count = 1, logical = 0, id = 0, levels = 0;

	if (cpu->cpuid_level >= 0x1f)
		leaf = 0x1f;
	else if (cpu->cpuid_level >= 0xb)
		leaf = 0xb;
	else
		return 0;

	for (sub = 0; sub < 8; sub++) {
		cpuid_count(cpu->number, leaf, sub, &eax, &ebx, &ecx, &edx);
		type = (ecx >> 8) & 0xff;
		if (type == 0)
			break;
		if (type == LEVEL_TYPE_SMT) {
			smt_shift = eax & 0x1f;
			smt_count = ebx & 0xffff;
		}
		pkg_shift = eax & 0x1f;
		logical = ebx & 0xffff;
		id = edx;
		levels++;
	}
	if (!levels || !smt_count)
		return 0;

	cpu->num_siblings = smt_count;
	cpu->x86_max_cores = logical / smt_count;
	cpu->phys_proc_id = pkg_shift >= 32 ? 0 : id >> pkg_shift;
	cpu->cpu_core_id = (id >> smt_shift) & low_mask(pkg_shift - smt_shift);
	cpu->topology = TOPOLOGY_X2APIC;
	return 1;
}

void get_topology(struct cpudata *cpu)
{
	if (get_x2apic_topology(cpu))
		return;

	if (cpu->vendor == VENDOR_INTEL) {
		get_intel_topology(cpu);
		cpu->topology = TOPOLOGY_LEGACY;
	}
}

static int cmp_u64(const void *a, const void *b)
{
	unsigned long long x = *(const unsigned long long *)a;
	unsigned long long y = *(const unsigned long long *)b;

	return (x > y) - (x < y);
}

/* Number of distinct (package, core) pairs among the CPUs. */
static unsigned int count_cores(struct cpudata *cpus)
{
	unsigned long long *keys;
	unsigned int i, cores = 0;

	keys = malloc(nrCPUs * sizeof(*keys));
	if (keys == NULL)
		return 0;

	for (i = 0; i < nrCPUs; i++)
		keys[i] = ((unsigned long long)cpus[i].phys_proc_id << 32) |
			  cpus[i].cpu_core_id;
	qsort(keys, nrCPUs, sizeof(*keys), cmp_u64);

	for (i = 0; i < nrCPUs; i++)
		if (i == 0 || keys[i] != keys[i - 1])
			cores++;

	free(keys);
	return cores;
}

void display_topology(struct cpudata *cpus)
{
	unsigned int i;
	unsigned int max_proc_id = 0;
	unsigned int *sockets;
	unsigned int num_sockets = 0;
	unsigned int total_threads = 0;
	unsigned int cores;
	unsigned int min_siblings = ~0u, max_siblings = 0;

	if (cpus[0].topology == TOPOLOGY_NONE)
		return;

	if (debug) {
		printf("cpu->phys_proc_id: ");
		for (i = 0; i < nrCPUs; i++) {
			printf("%u, ", cpus[i].phys_proc_id);
		}
		printf("\n");

		printf("cpu->x86_max_cores: ");
		for (i = 0; i < nrCPUs; i++) {
			printf("%u, ", cpus[i].x86_max_cores);
		}
		printf("\n");

		printf("cpu->cpu_core_id: ");
		for (i = 0; i < nrCPUs; i++) {
			printf("%u, ", cpus[i].cpu_core_id);
		}
		printf("\n");
	}

	/* Find the maximum phys_proc_id so the sockets array is large enough
	 * for sparse or non-zero-based socket APIC ID assignments. */
	for (i = 0; i < nrCPUs; i++) {
		if (cpus[i].phys_proc_id > max_proc_id)
			max_proc_id = cpus[i].phys_proc_id;
	}

	sockets = calloc(max_proc_id + 1, sizeof(*sockets));
	if (sockets == NULL)
		return;

	for (i = 0; i < nrCPUs; i++) {
		sockets[cpus[i].phys_proc_id]++;
		if (cpus[i].num_siblings < min_siblings)
			min_siblings = cpus[i].num_siblings;
		if (cpus[i].num_siblings > max_siblings)
			max_siblings = cpus[i].num_siblings;
	}

	for (i = 0; i <= max_proc_id; i++) {
		if (sockets[i] == 0)	/* only count populated sockets */
			continue;
		if (debug)
			printf("Socket %u: %u threads\n", i, sockets[i]);
		total_threads += sockets[i];
		num_sockets++;
	}
	free(sockets);

	if (cpus[0].topology == TOPOLOGY_X2APIC) {
		cores = count_cores(cpus);
	} else {
		/* No per-core ID: assume every core has the same thread count. */
		cores = max_siblings > 1 ? total_threads / max_siblings : total_threads;
	}

	/* Print a topology summary */
	printf("Total processor threads: %u\n", total_threads);
	printf("This system has %u socket%s, %u core%s, %u thread%s",
	       num_sockets, num_sockets == 1 ? "" : "s",
	       cores, cores == 1 ? "" : "s",
	       total_threads, total_threads == 1 ? "" : "s");

	if (max_siblings > 1) {
		if (min_siblings == max_siblings)
			printf(" (%u threads per core)", max_siblings);
		else
			printf(" (hybrid: %u-%u threads per core)",
			       min_siblings, max_siblings);
	}
	printf("\n");
}
