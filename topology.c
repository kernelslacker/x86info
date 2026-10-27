/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <stddef.h>
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
 * Decode an extended topology leaf. Each subleaf gives the shift to the
 * next level of the 32-bit x2APIC ID and how many logical processors sit
 * at or below that level. Returns 0 if the leaf holds no usable levels.
 */
static int decode_x2apic_leaf(struct cpudata *cpu, unsigned int leaf)
{
	unsigned int eax, ebx, ecx, edx, sub, type;
	unsigned int smt_shift = 0, pkg_shift = 0;
	unsigned int smt_count = 1, logical = 0, id = 0, levels = 0;

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
	if (!levels || !smt_count || !logical || pkg_shift < smt_shift)
		return 0;

	cpu->num_siblings = smt_count;
	cpu->x86_max_cores = logical / smt_count;
	cpu->phys_proc_id = id >> pkg_shift;
	cpu->cpu_core_id = (id >> smt_shift) & low_mask(pkg_shift - smt_shift);
	cpu->topology = TOPOLOGY_X2APIC;
	return 1;
}

/* Decode the extended topology leaf: AMD's 0x80000026, else 0x1F, else 0xB. */
static int get_x2apic_topology(struct cpudata *cpu)
{
	if (cpu->vendor == VENDOR_AMD && cpu->maxei >= 0x80000026 &&
	    decode_x2apic_leaf(cpu, 0x80000026))
		return 1;
	if (cpu->cpuid_level >= 0x1f && decode_x2apic_leaf(cpu, 0x1f))
		return 1;
	if (cpu->cpuid_level >= 0xb && decode_x2apic_leaf(cpu, 0xb))
		return 1;
	return 0;
}

/* Raw core type encodings: AMD 0x80000026 EBX[31:28], Intel 0x1A EAX[31:24]. */
#define AMD_CORE_PERFORMANCE	0
#define AMD_CORE_EFFICIENCY	1
#define INTEL_CORE_ATOM		0x20
#define INTEL_CORE_CORE		0x40

/*
 * Intel: CPUID.7.EDX[15] flags a hybrid part; leaf 0x1A then names this
 * CPU's core. AMD: CPUID 0x80000026 EBX[31:28] of the core level.
 */
void get_core_type(struct cpudata *cpu)
{
	unsigned int eax, ebx, edx;

	cpu->core_type = 0;
	if (cpu->vendor == VENDOR_AMD) {
		if (cpu->maxei < 0x80000026)
			return;
		cpuid_count(cpu->number, 0x80000026, 0, NULL, &ebx, NULL, NULL);
		switch (ebx >> 28) {
		case AMD_CORE_PERFORMANCE:
			cpu->core_type = CORE_TYPE_PERFORMANCE;
			break;
		case AMD_CORE_EFFICIENCY:
			cpu->core_type = CORE_TYPE_EFFICIENCY;
			break;
		default:
			cpu->core_type = CORE_TYPE_UNKNOWN;
		}
		return;
	}

	if (cpu->vendor != VENDOR_INTEL || cpu->cpuid_level < 0x1a)
		return;

	cpuid_count(cpu->number, 7, 0, NULL, NULL, NULL, &edx);
	if (!(edx & (1 << 15)))
		return;

	cpuid(cpu->number, 0x1a, &eax, NULL, NULL, NULL);
	switch (eax >> 24) {
	case INTEL_CORE_CORE:
		cpu->core_type = CORE_TYPE_PERFORMANCE;
		break;
	case INTEL_CORE_ATOM:
		cpu->core_type = CORE_TYPE_EFFICIENCY;
		break;
	default:
		cpu->core_type = CORE_TYPE_UNKNOWN;
	}
}

const char *core_type_name(const struct cpudata *cpu)
{
	int amd = cpu->vendor == VENDOR_AMD;

	switch (cpu->core_type) {
	case CORE_TYPE_EFFICIENCY:
		return amd ? "Efficiency (compact)" : "Efficiency (Atom)";
	case CORE_TYPE_PERFORMANCE:
		return amd ? "Performance" : "Performance (Core)";
	default:
		return "unknown";
	}
}

/* A core type is only worth reporting when the CPUs differ. */
void clear_uniform_core_types(struct cpudata *cpus, unsigned int n)
{
	unsigned int i;

	for (i = 1; i < n; i++)
		if (cpus[i].core_type != cpus[0].core_type)
			return;
	for (i = 0; i < n; i++)
		cpus[i].core_type = 0;
}

/*
 * AMD without an extended topology leaf: 0x80000008 gives the thread count
 * and APIC ID width, and with TOPOEXT 0x8000001E the core ID and the
 * threads per core.
 */
static int get_amd_topology(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, nthreads, bits, apicid, threads = 1;

	if (cpu->maxei < 0x80000008)
		return 0;

	cpuid(cpu->number, 0x80000008, NULL, NULL, &ecx, NULL);
	nthreads = (ecx & 0xff) + 1;
	bits = (ecx >> 12) & 0xf;
	if (!bits)
		bits = get_count_order(nthreads);

	cpuid(cpu->number, 1, NULL, &ebx, NULL, NULL);
	apicid = ebx >> 24;
	cpu->cpu_core_id = apicid & low_mask(bits);

	if (cpu->maxei >= 0x8000001e) {
		cpuid(cpu->number, 0x80000001, NULL, NULL, &ecx, NULL);
		if (ecx & (1u << 22)) {
			cpuid(cpu->number, 0x8000001e, &eax, &ebx, NULL, NULL);
			apicid = eax;
			threads = ((ebx >> 8) & 0xff) + 1;
			cpu->cpu_core_id = ebx & 0xff;
		}
	}
	if (threads > nthreads)
		threads = nthreads;

	cpu->num_siblings = threads;
	cpu->x86_max_cores = nthreads / threads;
	cpu->phys_proc_id = apicid >> bits;
	cpu->topology = TOPOLOGY_LEGACY;
	return 1;
}

void get_topology(struct cpudata *cpu)
{
	if (get_x2apic_topology(cpu))
		return;

	if (cpu->vendor == VENDOR_AMD) {
		get_amd_topology(cpu);
		return;
	}

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

/*
 * Number of distinct packages, or with by_core distinct (package, core)
 * pairs, among the CPUs; *min_threads and *max_threads (optional) get the
 * smallest and largest number of CPUs sharing one.
 */
static unsigned int count_groups(const struct cpudata *cpus, unsigned int n,
				 int by_core, unsigned int *min_threads,
				 unsigned int *max_threads)
{
	unsigned long long *keys;
	unsigned int i, run = 0, groups = 0, min = ~0u, max = 0;

	keys = malloc(n * sizeof(*keys));
	if (keys == NULL)
		return 0;

	for (i = 0; i < n; i++)
		keys[i] = by_core ? ((unsigned long long)cpus[i].phys_proc_id << 32) |
				    cpus[i].cpu_core_id : cpus[i].phys_proc_id;
	qsort(keys, n, sizeof(*keys), cmp_u64);

	for (i = 0; i < n; i++) {
		if (i == 0 || keys[i] != keys[i - 1]) {
			groups++;
			run = 0;
		}
		run++;
		if (i + 1 == n || keys[i + 1] != keys[i]) {
			if (run < min)
				min = run;
			if (run > max)
				max = run;
		}
	}

	free(keys);
	if (min_threads)
		*min_threads = min;
	if (max_threads)
		*max_threads = max;
	return groups;
}

/* Print the levels of the extended topology leaf the CPU decodes from. */
static void show_topology_levels(const struct cpudata *cpu)
{
	static const char * const intel_names[] = {
		"Invalid", "SMT", "Core", "Module", "Tile", "Die"
	};
	static const char * const amd_names[] = {
		"Invalid", "Core", "Complex", "Die", "Socket"
	};
	const char * const *names = intel_names;
	unsigned int nr_names = ARRAY_SIZE(intel_names);
	unsigned int eax, ebx, ecx, edx, leaf, subleaf, level_type;

	if (cpu->vendor == VENDOR_AMD && cpu->maxei >= 0x80000026) {
		leaf = 0x80000026;
		names = amd_names;
		nr_names = ARRAY_SIZE(amd_names);
	} else if (cpu->cpuid_level >= 0x1f) {
		leaf = 0x1f;
	} else if (cpu->cpuid_level >= 0xb) {
		leaf = 0xb;
	} else {
		return;
	}

	cpuid_count(cpu->number, leaf, 0, NULL, &ebx, NULL, NULL);
	if (!ebx)
		return;

	printf("Extended topology (leaf 0x%X):\n", leaf);
	for (subleaf = 0; subleaf < 8; subleaf++) {
		cpuid_count(cpu->number, leaf, subleaf, &eax, &ebx, &ecx, &edx);
		level_type = (ecx >> 8) & 0xff;
		if (level_type == 0)
			break;
		printf("  Level %u (%s): %u logical CPUs"
		       ", x2APIC shift=%u, x2APIC ID=%u\n",
		       ecx & 0xff,
		       level_type < nr_names ? names[level_type] : "Unknown",
		       ebx & 0xffff, eax & 0x1f, edx);
	}
}

void display_topology(struct cpudata *cpus, unsigned int n)
{
	unsigned int i;
	unsigned int num_sockets;
	unsigned int total_threads = n;
	unsigned int cores;
	unsigned int min_siblings, max_siblings;
	unsigned int p_threads = 0, e_threads = 0;

	if (cpus[0].topology == TOPOLOGY_NONE)
		return;

	if (opts.debug) {
		static const struct {
			const char *label;
			size_t offset;
		} fields[] = {
			{ "cpu->phys_proc_id: ", offsetof(struct cpudata, phys_proc_id) },
			{ "cpu->x86_max_cores: ", offsetof(struct cpudata, x86_max_cores) },
			{ "cpu->cpu_core_id: ", offsetof(struct cpudata, cpu_core_id) },
		};
		unsigned int f;

		for (f = 0; f < ARRAY_SIZE(fields); f++) {
			printf("%s", fields[f].label);
			for (i = 0; i < n; i++)
				printf("%u, ", *(const unsigned int *)
				       ((const char *)&cpus[i] + fields[f].offset));
			printf("\n");
		}

		for (i = 0; i < n; i++) {
			unsigned int j, threads = 0;

			for (j = 0; j < i; j++)
				if (cpus[j].phys_proc_id == cpus[i].phys_proc_id)
					break;
			if (j < i)
				continue;
			for (j = i; j < n; j++)
				if (cpus[j].phys_proc_id == cpus[i].phys_proc_id)
					threads++;
			printf("Socket %u: %u threads\n", cpus[i].phys_proc_id, threads);
		}
	}

	num_sockets = count_groups(cpus, n, 0, NULL, NULL);

	cores = count_groups(cpus, n, 1, &min_siblings, &max_siblings);

	/* Print a topology summary */
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

	for (i = 0; i < n; i++) {
		if (cpus[i].core_type == CORE_TYPE_PERFORMANCE)
			p_threads++;
		else if (cpus[i].core_type == CORE_TYPE_EFFICIENCY)
			e_threads++;
	}
	if (p_threads || e_threads)
		printf("Hybrid: %u Performance threads, %u Efficiency threads\n",
		       p_threads, e_threads);

	if (opts.debug || opts.verbose)
		show_topology_levels(&cpus[0]);
}
