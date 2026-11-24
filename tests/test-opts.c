/*
 * Check how command line options map to the work x86info does.
 *
 * commandline.c is included rather than linked so that the option and
 * help tables stay static. The man page check opens x86info.1, so run
 * from the repo root as make check does.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <x86info.h>
#include "../commandline.c"

bool is_root(void)
{
	return false;
}

static int parse(const char *arg)
{
	char prog[] = "x86info";
	char buf[32];
	char *argv[] = { prog, buf, NULL };

	snprintf(buf, sizeof(buf), "%s", arg);

	memset(&opts, 0, sizeof(opts));
	/* optind 0 makes glibc drop its private cursor into the old buf. */
	optind = 0;
	parse_command_line(2, argv);
	return want_powernow();
}

static const struct option *find_val(int val)
{
	for (int i = 0; long_options[i].name; i++)
		if (long_options[i].val == val)
			return &long_options[i];
	return NULL;
}

static const struct option *find_name(const char *name)
{
	for (int i = 0; long_options[i].name; i++)
		if (!strcmp(long_options[i].name, name))
			return &long_options[i];
	return NULL;
}

/* Short forms that exit, such as -h, are left out. */
static const char short_opts[] = "acfmrv";

/* Each short form must set exactly what its long form sets. */
static int check_short_opts(void)
{
	static const struct options none;
	int fail = 0;

	for (const char *s = short_opts; *s; s++) {
		const struct option *lo = find_val(*s);
		char sarg[3] = { '-', *s, '\0' };
		char larg[64];
		struct options by_short, by_long;

		if (!lo) {
			fprintf(stderr, "-%c has no long option\n", *s);
			fail = 1;
			continue;
		}

		snprintf(larg, sizeof(larg), "--%s", lo->name);
		parse(sarg);
		by_short = opts;
		parse(larg);
		by_long = opts;

		if (memcmp(&by_short, &by_long, sizeof(opts))) {
			fprintf(stderr, "-%c differs from %s\n", *s, larg);
			fail = 1;
		}
		if (!memcmp(&by_short, &none, sizeof(opts))) {
			fprintf(stderr, "-%c sets no option\n", *s);
			fail = 1;
		}
	}
	return fail;
}

/* The man page is a few KiB, so a fixed buffer is ample. */
static char man[65536];

static bool read_man(void)
{
	FILE *f = fopen("x86info.1", "r");
	size_t n;
	bool ok;

	if (!f)
		return false;
	n = fread(man, 1, sizeof(man) - 1, f);
	/* A truncated read would silently drop options past the cut. */
	ok = feof(f) && !ferror(f);
	fclose(f);
	man[n] = '\0';
	return ok;
}

/*
 * Man pages escape hyphens, so --all-cpus is written \-\-all\-cpus.
 * Copies the next option name into name and returns the position after
 * it, or NULL when there are no more.
 */
static const char *man_next(const char *p, char *name, size_t len)
{
	size_t n = 0;

	p = strstr(p, "\\-\\-");
	if (!p)
		return NULL;
	p += 4;
	for (;;) {
		char c;

		if (isalnum((unsigned char)*p)) {
			c = *p++;
		} else if (p[0] == '\\' && p[1] == '-') {
			c = '-';
			p += 2;
		} else {
			break;
		}
		if (n + 1 < len)
			name[n++] = c;
	}
	name[n] = '\0';
	return p;
}

static int check_man(void)
{
	char name[64];
	const char *p;
	int fail = 0;

	if (!read_man()) {
		fprintf(stderr, "cannot read x86info.1 in full\n");
		return 1;
	}

	p = man;
	while ((p = man_next(p, name, sizeof(name))) != NULL) {
		if (!find_name(name)) {
			fprintf(stderr, "x86info.1 documents unknown --%s\n", name);
			fail = 1;
		}
	}

	for (int i = 0; long_options[i].name; i++) {
		bool found = false;

		p = man;
		while (!found && (p = man_next(p, name, sizeof(name))) != NULL)
			found = !strcmp(name, long_options[i].name);
		if (!found) {
			fprintf(stderr, "--%s is not in x86info.1\n",
				long_options[i].name);
			fail = 1;
		}
	}
	return fail;
}

static int check_help(void)
{
	size_t nhelp = sizeof(help_text) / sizeof(help_text[0]);
	int fail = 0;

	for (int i = 0; long_options[i].name; i++) {
		int val = long_options[i].val;

		if ((size_t)val >= nhelp || !help_text[val] || !*help_text[val]) {
			fprintf(stderr, "--%s has no help text\n",
				long_options[i].name);
			fail = 1;
		}
	}

	/* Help text without an option means the two tables have drifted. */
	for (size_t v = 0; v < nhelp; v++) {
		if (help_text[v] && !find_val((int)v)) {
			fprintf(stderr, "help text %zu has no long option\n", v);
			fail = 1;
		}
	}
	return fail;
}

int main(void)
{
	int fail = 0;

	/* Without root, --all must not ask for the root-only output. */
	parse("--all");
	if (opts.need_root || opts.show_msr || opts.show_apic ||
	    opts.show_bios || opts.show_mptable || opts.show_mtrr ||
	    opts.show_machine_check) {
		fprintf(stderr, "--all as non-root requests root-only output\n");
		fail = 1;
	}
	if (!opts.show_bugs || !opts.show_cacheinfo || !opts.show_MHz ||
	    !opts.show_pm || !opts.show_cpuid) {
		fprintf(stderr, "--all as non-root drops unprivileged output\n");
		fail = 1;
	}

	if (!parse("--bios")) {
		fprintf(stderr, "--bios alone does not reach the PowerNow path\n");
		fail = 1;
	}
	if (!parse("--pm")) {
		fprintf(stderr, "--pm does not reach the PowerNow path\n");
		fail = 1;
	}
	if (parse("--cache")) {
		fprintf(stderr, "--cache reaches the PowerNow path\n");
		fail = 1;
	}

	fail |= check_short_opts();
	fail |= check_man();
	fail |= check_help();
	return fail;
}
