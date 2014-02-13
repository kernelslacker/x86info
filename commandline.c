/*
 *  (C) 2011 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x86info.h>

unsigned int show_apic = 0;
unsigned int show_bios = 0;
unsigned int show_machine_check = 0;
unsigned int show_bugs = 0;
unsigned int show_cacheinfo = 0;
unsigned int show_connector = 0;
unsigned int show_msr = 0;
unsigned int show_microcode = 0;
unsigned int show_mtrr = 0;
unsigned int show_pm = 0;
unsigned int show_cpuid = 0;
unsigned int show_urls = 0;
unsigned int show_mptable = 0;
unsigned int show_feature_flags = 0;
unsigned int show_MHz = 0;
unsigned int show_addr_sizes = 0;
unsigned int all_cpus = 0;

unsigned int debug = 0;
unsigned int verbose = 0;
unsigned int need_root = 0;


static void usage(char *programname)
{
	printf("Usage: %s [<switches>]\n\
-a,   --all\n\
      --all-cpus\n\
      --addr\n\
      --apic\n\
      --bios\n\
      --bugs\n\
-c,   --cache\n\
      --connector\n\
      --debug\n\
-f,   --flags\n\
-mce, --show-machine-check\n\
-mhz, --mhz\n\
      --microcode\n\
-mp,  --mptable\n\
-m,   --msr\n\
      --mtrr\n\
      --pm\n\
-r,   --registers\n\
-u,   --urls\n\
-v,   --verbose\n\
\n", programname);
	exit (0);
}

void parse_command_line(int argc, char **argv)
{
	char **argp, *arg;

	for (argp = argv+1; argp <= argv + argc && (arg = *argp); argp++) {
		if ((!strcmp(arg, "-a") || !strcmp(arg, "--all"))) {

			if (user_is_root) {
				show_apic = 1;
				show_bios = 1;
				show_msr = 1;
				show_microcode = 1;
				need_root = 1;
				show_mptable =1;
				show_mtrr = 1;
				show_machine_check = 1;
			}
			show_addr_sizes = 1;
			show_bugs = 1;
			show_cacheinfo = 1;
			show_connector = 1;
			show_feature_flags = 1;
			show_MHz = 1;
			show_pm = 1;
			show_cpuid = 1;
			show_urls = 1;
		}

		if (!strcmp(arg, "--all-cpus"))
			all_cpus = 1;

		if (!strcmp(arg, "--addr"))
			show_addr_sizes = 1;

		if (!strcmp(arg, "--apic")) {
			need_root = 1;
			show_apic = 1;
		}

		if (!strcmp(arg, "--bios")) {
			need_root = 1;
			show_bios = 1;
		}

		if (!strcmp(arg, "--bugs"))
			show_bugs = 1;

		if ((!strcmp(arg, "-c") || !strcmp(arg, "--cache")))
			show_cacheinfo = 1;

		if (!strcmp(arg, "--connector"))
			show_connector = 1;

		if (!strcmp(arg, "--debug"))
			debug = 1;

		if ((!strcmp(arg, "-f") || !strcmp(arg, "--flags")))
			show_feature_flags = 1;

		if ((!strcmp(arg, "-m") || !strcmp(arg, "--msr"))) {
			need_root = 1;
			show_msr = 1;
		}

		if (!strcmp(arg, "--microcode")) {
			need_root = 1;
			show_microcode = 1;
		}

		if ((!strcmp(arg, "-mhz") || !strcmp(arg, "--mhz")))
			show_MHz = 1;

		if ((!strcmp(arg, "-mp") || !strcmp(arg, "--mptable"))) {
			need_root = 1;
			show_mptable = 1;
		}

		if (!strcmp(arg, "--mtrr")) {
			need_root = 1;
			show_mtrr = 1;
		}

		if (!strcmp(arg, "--pm"))
			show_pm = 1;

		if ((!strcmp(arg, "-r") || !strcmp(arg, "--raw-cpuid")))
			show_cpuid = 1;

		if ((!strcmp(arg, "-mce") || !strcmp(arg, "--show-machine-check"))) {
			need_root = 1;
			show_machine_check = 1;
		}

		if ((!strcmp(arg, "-u") || !strcmp(arg, "--urls")))
			show_urls = 1;

		if ((!strcmp(arg, "-v") || !strcmp(arg, "--verbose")))
			verbose = 1;

		if ((!strcmp(arg, "?") || !strcmp(arg, "--help")))
			usage(argv[0]);
	}
}
