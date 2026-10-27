/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Parsing of kernel CPU lists.
 */

#include <stdlib.h>
#include <x86info.h>

/*
 * Parse a kernel cpu list such as "0-3,8,10-11" into an array of ids.
 * Returns NULL if the list is malformed, empty or names more than MAX_CPUS.
 */
unsigned int *parse_cpu_list(const char *s, unsigned int *count)
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
