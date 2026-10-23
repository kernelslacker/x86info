/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  CPUID replay backend for tests: answers cpuid() from a dump in the
 *  format printed by 'x86info -r', named by $X86INFO_CPUID_DUMP.
 */

#ifdef CPUID_REPLAY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86info.h>

struct replay_entry {
	unsigned int leaf, sub;
	unsigned int regs[4];
};

static struct replay_entry *entries;
static unsigned int nr_entries;

static void replay_load(void)
{
	const char *path = getenv("X86INFO_CPUID_DUMP");
	char line[256];
	FILE *f;

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
		struct replay_entry e = { 0 };
		struct replay_entry *tmp;

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

void cpuid(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx)
{
	unsigned int leaf = (unsigned int)idx;
	unsigned int sub = (unsigned int)(idx >> 32);
	unsigned int regs[4] = { 0, 0, 0, 0 };
	unsigned int i;

	(void)CPU_number;
	if (!entries)
		replay_load();

	for (i = 0; i < nr_entries; i++) {
		if (entries[i].leaf == leaf && entries[i].sub == sub) {
			memcpy(regs, entries[i].regs, sizeof(regs));
			break;
		}
	}

	if (eax)
		*eax = regs[0];
	if (ebx)
		*ebx = regs[1];
	if (ecx)
		*ecx = regs[2];
	if (edx)
		*edx = regs[3];
}

#endif	/* CPUID_REPLAY */
