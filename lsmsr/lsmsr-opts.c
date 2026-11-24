/*
 * Licensed under the terms of the GNU GENERAL PUBLIC LICENSE version 2.
 *
 * lsmsr command line option parsing.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <getopt.h>

#include "lsmsr-opts.h"

/* getopt_long stuff */
static struct option lopts[] = {
	{"all", no_argument, 0, 'a'},		/* show all MSRs */
	{"cpu", required_argument, 0, 'c'},	/* CPU number */
	{"family", required_argument, 0, 'f'},	/* CPU family */
	{"list", no_argument, 0, 'l'},		/* list known MSRs */
	{"register", required_argument, 0, 'r'},  /* register name or address */
	{"help", no_argument, 0, 'h'},		/* help */
	{"version", no_argument, 0, 'v'},	/* version info */
	{"verbosity", required_argument, 0, 'V'}, /* verbosity */
	{0, 0, 0, 0}
};

#define OPTSTRING "ac:f:hlr:vV:"

static int parse_int_opt(const char *name, const char *s, int base, int *out)
{
	char *end;
	long val;

	errno = 0;
	val = strtol(s, &end, base);
	if (end == s || *end != '\0' || errno == ERANGE ||
	    val < INT_MIN || val > INT_MAX) {
		fprintf(stderr, "%s: invalid value: %s\n", name, s);
		return -1;
	}
	*out = (int)val;
	return 0;
}

static int try_parse_uint(const char *s, int base, unsigned int *out)
{
	char *end;
	unsigned long val;

	errno = 0;
	val = strtoul(s, &end, base);
	if (end == s || *end != '\0' || errno == ERANGE || val > UINT_MAX)
		return -1;
	*out = (unsigned int)val;
	return 0;
}

int lsmsr_parse_opts(int argc, char **argv, struct lsmsr_opts *o)
{
	unsigned int reg;
	int c, li;

	memset(o, 0, sizeof(*o));
	o->reg = (uint32_t)-1;

	while ((c = getopt_long(argc, argv, OPTSTRING, lopts, &li)) != -1) {
		switch (c) {
		case 'a':
			o->show_all = 1;
			break;
		case 'h':
			return LSMSR_OPT_HELP;
		case 'v':
			return LSMSR_OPT_VERSION;
		case 'V':
			if (parse_int_opt("-V", optarg, 10, &o->verbosity))
				return LSMSR_OPT_BAD;
			break;
		case 'c':
			if (parse_int_opt("-c", optarg, 0, &o->cpu))
				return LSMSR_OPT_BAD;
			if (o->cpu < 0) {
				fprintf(stderr, "-c: invalid value: %s\n", optarg);
				return LSMSR_OPT_BAD;
			}
			break;
		case 'f':
			if (parse_int_opt("-f", optarg, 0, &o->family))
				return LSMSR_OPT_BAD;
			break;
		case 'l':
			o->list = 1;
			break;
		case 'r':
			/* Hex addresses may start with a-f, so try a number first. */
			if (try_parse_uint(optarg, 16, &reg))
				o->msr_name = optarg;
			else
				o->reg = reg;
			break;
		default:
			return LSMSR_OPT_USAGE;
		}
	}
	return LSMSR_OPT_OK;
}
