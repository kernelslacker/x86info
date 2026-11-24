#ifndef LSMSR_OPTS_H
#define LSMSR_OPTS_H

#include <stdint.h>

struct lsmsr_opts {
	int show_all;
	int list;
	int verbosity;
	int cpu;
	int family;
	const char *msr_name;	/* -r argument that is not a number */
	uint32_t reg;		/* -r argument that is, else (uint32_t)-1 */
};

#define LSMSR_OPT_OK		0
#define LSMSR_OPT_HELP		1
#define LSMSR_OPT_VERSION	2
#define LSMSR_OPT_BAD		3	/* already reported */
#define LSMSR_OPT_USAGE		4

int lsmsr_parse_opts(int argc, char **argv, struct lsmsr_opts *o);

#endif
