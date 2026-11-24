/*
 * Check the lsmsr option parser.
 */
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "../lsmsr/lsmsr-opts.h"

static int fail;

static int parse(struct lsmsr_opts *o, const char *a1, const char *a2)
{
	char prog[] = "lsmsr";
	/* Static: o->msr_name points into these. */
	static char b1[32], b2[32];
	char *argv[4] = { prog, NULL, NULL, NULL };
	int argc = 1;

	if (a1) {
		snprintf(b1, sizeof(b1), "%s", a1);
		argv[argc++] = b1;
	}
	if (a2) {
		snprintf(b2, sizeof(b2), "%s", a2);
		argv[argc++] = b2;
	}
	/* 0 also drops getopt's state left by an early return. */
	optind = 0;
	return lsmsr_parse_opts(argc, argv, o);
}

static void expect(int ok, const char *what)
{
	if (!ok) {
		fprintf(stderr, "lsmsr: %s\n", what);
		fail = 1;
	}
}

int main(void)
{
	struct lsmsr_opts o;

	expect(parse(&o, "-h", NULL) == LSMSR_OPT_HELP, "-h is not help");
	expect(parse(&o, "--version", NULL) == LSMSR_OPT_VERSION,
	       "--version is not version");
	expect(parse(&o, "-x", NULL) == LSMSR_OPT_USAGE, "-x accepted");

	expect(parse(&o, "-a", NULL) == LSMSR_OPT_OK && o.show_all,
	       "-a not parsed");
	expect(parse(&o, "-l", NULL) == LSMSR_OPT_OK && o.list,
	       "-l not parsed");
	expect(parse(&o, "-V", "3") == LSMSR_OPT_OK && o.verbosity == 3,
	       "-V 3 not parsed");
	expect(parse(&o, "-c", "5") == LSMSR_OPT_OK && o.cpu == 5,
	       "-c 5 not parsed");
	expect(parse(&o, "-f", "0x10") == LSMSR_OPT_OK && o.family == 16,
	       "-f 0x10 not parsed");

	expect(parse(&o, "-c", "-1") == LSMSR_OPT_BAD, "-c -1 accepted");
	expect(parse(&o, "-c", "3x") == LSMSR_OPT_BAD, "-c 3x accepted");
	expect(parse(&o, "-V", "") == LSMSR_OPT_BAD, "-V empty accepted");
	expect(parse(&o, "-f", "99999999999") == LSMSR_OPT_BAD,
	       "out of range -f accepted");

	/* Hex digits only: a number, even though it looks like a name. */
	expect(parse(&o, "-r", "c0010015") == LSMSR_OPT_OK &&
	       o.reg == 0xc0010015 && !o.msr_name, "-r hex address");
	expect(parse(&o, "-r", "0xc0010015") == LSMSR_OPT_OK &&
	       o.reg == 0xc0010015 && !o.msr_name, "-r 0x address");
	expect(parse(&o, "-r", "abc") == LSMSR_OPT_OK && o.reg == 0xabc &&
	       !o.msr_name, "-r abc is an address, not a name");
	expect(parse(&o, "-r", "HWCR") == LSMSR_OPT_OK &&
	       o.reg == (uint32_t)-1 && o.msr_name &&
	       strcmp(o.msr_name, "HWCR") == 0, "-r name");
	expect(parse(&o, "-r", "1ffffffff") == LSMSR_OPT_OK &&
	       o.msr_name, "oversized -r address is not treated as a name");
	expect(parse(&o, "--register", "HWCR") == LSMSR_OPT_OK &&
	       o.msr_name, "--register not parsed");
	return fail;
}
