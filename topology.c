/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86info.h>
#include <intel.h>

static const char * corenum(int num)
{
	switch (num) {
	case 1:	return ("single");
	case 2:	return ("dual");
	case 3:	return ("tri");
	case 4:	return ("quad");
	case 6:	return ("six");
	case 8:	return ("eight");
	default:
		return ("?");
	}
}

void get_topology(struct cpudata *cpu)
{
	if (cpu->vendor == VENDOR_INTEL)
		get_intel_topology(cpu);
}

void display_topology(struct cpudata *cpus)
{
	unsigned int threads_per_socket;
	unsigned int i;
	unsigned int max_proc_id = 0;
	char *sockets;

	int num_sockets = 0;

	/* For now, we only support topology parsing on Intel. */
	if (cpus[0].vendor != VENDOR_INTEL)
		return;

	if (debug == 1) {
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

	sockets = malloc(max_proc_id + 1);
	if (sockets == NULL)
		return;

	for (i = 0; i <= max_proc_id; i++)
		sockets[i]=0;

	for (i = 0; i < nrCPUs; i++) {
		sockets[cpus[i].phys_proc_id]++;
	}

	for (i = 0; i <= max_proc_id; i++) {
		if (debug == 1)
			if (sockets[i] != 0)
				printf("Socket %u: %u threads\n", i, (unsigned int) sockets[i]);
		if (sockets[i] != 0)	/* only count populated sockets */
			num_sockets++;
	}

	/* Print a topology summary */
	printf("Total processor threads: %d\n", sockets[0] * num_sockets);
	printf("This system has %d ", num_sockets);
	threads_per_socket = sockets[0];
	if (cpus[0].flags_edx & X86_FEATURE_HT)
		if (cpus[0].num_siblings > 1)
			threads_per_socket = sockets[0]/2;

	if (nrCPUs == 1) {
		/* Handle the single CPU case */
		printf("processor");
	} else {
		const char *p;
		p = corenum(threads_per_socket);

		if (strncmp("?", p, 1))
			printf("%s-core processor", corenum(threads_per_socket));
		else
			printf("%u-core processor", threads_per_socket);
		if (num_sockets > 1)
			printf("s");
	}

	if (cpus[0].flags_edx & X86_FEATURE_HT && cpus[0].num_siblings > 1)
		printf(" with hyper-threading (%u threads per core)", cpus[0].num_siblings);

	free(sockets);
}
