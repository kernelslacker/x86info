/*
 *  (C) 2011 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "x86info.h"

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

void display_topology(struct cpudata *head)
{
	struct cpudata *cpu;
	unsigned int threads_per_socket;
	unsigned int i;
	char *sockets;

	int num_sockets = 0;

	/* For now, we only support topology parsing on Intel. */
	if (head->vendor != VENDOR_INTEL)
		return;

	if (debug == 1) {
		cpu = head;
		printf("cpu->phys_proc_id: ");
		for (i = 0; i < nrCPUs; i++) {
			printf("%d, ", cpu->phys_proc_id);
			cpu = cpu->next;
		}
		printf("\n");

		cpu = head;
		printf("cpu->x86_max_cores: ");
		for (i = 0; i < nrCPUs; i++) {
			printf("%d, ", cpu->x86_max_cores);
			cpu = cpu->next;
		}
		printf("\n");

		cpu = head;
		printf("cpu->cpu_core_id: ");
		for (i = 0; i < nrCPUs; i++) {
			printf("%d, ", cpu->cpu_core_id);
			cpu = cpu->next;
		}
		printf("\n");
	}

	sockets = malloc(nrCPUs);
	if (sockets == NULL)
		return;

	for (i = 0; i < nrCPUs; i++)
		sockets[i]=0;

	cpu = head;
	for (i = 0; i < nrCPUs; i++) {
		sockets[cpu->phys_proc_id]++;
		cpu = cpu->next;
	}

	for (i = 0; i < nrCPUs; i++) {
		if (debug == 1)
			printf("Socket %d: %d threads\n", i, sockets[i]);
		if (sockets[i] != 0)	/* only count populated sockets */
			num_sockets++;
	}

	/* Print a topology summary */
	cpu = head;
	printf("Total processor threads: %d\n", sockets[0] * num_sockets);
	printf("This system has %d ", num_sockets);
	threads_per_socket = sockets[0];
	if (cpu->flags_edx & X86_FEATURE_HT)
		if (cpu->num_siblings > 1)
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
			printf("%d-core processor", threads_per_socket);
		if (num_sockets > 1)
			printf("s");
	}

	if (cpu->flags_edx & X86_FEATURE_HT && cpu->num_siblings > 1)
		printf(" with hyper-threading (%d threads per core)", cpu->num_siblings);

	free(sockets);
}
