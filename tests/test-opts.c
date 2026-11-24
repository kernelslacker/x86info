/*
 * Check how command line options map to the work x86info does.
 */
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <x86info.h>

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
	optind = 1;
	parse_command_line(2, argv);
	return want_powernow();
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
	return fail;
}
