/*
 * Check the kernel CPU list parser.
 */
#include <stdio.h>
#include <stdlib.h>
#include <x86info.h>

static int fail;

/* Expect the list to parse to exactly want[0..n-1]; n == 0 means rejected. */
static void check(const char *list, const unsigned int *want, unsigned int n)
{
	unsigned int count = 0, i;
	unsigned int *ids = parse_cpu_list(list, &count);

	if (n == 0) {
		if (ids) {
			fprintf(stderr, "\"%s\" was accepted\n", list);
			fail = 1;
		}
		free(ids);
		return;
	}
	if (!ids || count != n) {
		fprintf(stderr, "\"%s\": got %u ids, want %u\n", list,
			ids ? count : 0, n);
		fail = 1;
		free(ids);
		return;
	}
	for (i = 0; i < n; i++) {
		if (ids[i] != want[i]) {
			fprintf(stderr, "\"%s\": id %u is %u, want %u\n", list,
				i, ids[i], want[i]);
			fail = 1;
			break;
		}
	}
	free(ids);
}

int main(void)
{
	static const unsigned int one[] = { 0 };
	static const unsigned int range[] = { 0, 1, 2, 3 };
	static const unsigned int mixed[] = { 0, 1, 2, 3, 8, 10, 11 };
	static const unsigned int top[] = { MAX_CPUS - 1 };

	check("0", one, 1);
	check("0\n", one, 1);
	check("0-3", range, 4);
	check("0-3,8,10-11\n", mixed, 7);
	check("0-3,8,10-11,", mixed, 7);
	check("8191", top, 1);

	check("", NULL, 0);
	check("\n", NULL, 0);
	check(",", NULL, 0);
	check("a", NULL, 0);
	check("3-1", NULL, 0);
	check("1-", NULL, 0);
	check("1 2", NULL, 0);
	check("0-3x", NULL, 0);
	check("8192", NULL, 0);
	check("0-8192", NULL, 0);
	check("0-8191,0", NULL, 0);

	return fail;
}
