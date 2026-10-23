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
