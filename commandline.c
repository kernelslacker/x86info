/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
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
unsigned int show_mptable = 0;
unsigned int show_feature_flags = 0;
unsigned int show_MHz = 0;
unsigned int show_addr_sizes = 0;
unsigned int all_cpus = 0;

unsigned int debug = 0;
unsigned int verbose = 0;
unsigned int need_root = 0;

enum {
	OPT_ALL_CPUS = 128,
	OPT_ADDR,
	OPT_APIC,
	OPT_BIOS,
	OPT_BUGS,
	OPT_CONNECTOR,
	OPT_DEBUG,
	OPT_SHOW_MACHINE_CHECK,
	OPT_MHZ,
	OPT_MICROCODE,
	OPT_MPTABLE,
	OPT_MTRR,
	OPT_PM,
};

static struct option long_options[] = {
	{ "all",                no_argument, NULL, 'a' },
	{ "all-cpus",           no_argument, NULL, OPT_ALL_CPUS },
	{ "addr",               no_argument, NULL, OPT_ADDR },
	{ "apic",               no_argument, NULL, OPT_APIC },
	{ "bios",               no_argument, NULL, OPT_BIOS },
	{ "bugs",               no_argument, NULL, OPT_BUGS },
	{ "cache",              no_argument, NULL, 'c' },
	{ "connector",          no_argument, NULL, OPT_CONNECTOR },
	{ "debug",              no_argument, NULL, OPT_DEBUG },
	{ "flags",              no_argument, NULL, 'f' },
	{ "help",               no_argument, NULL, 'h' },
	{ "mhz",                no_argument, NULL, OPT_MHZ },
	{ "microcode",          no_argument, NULL, OPT_MICROCODE },
	{ "mptable",            no_argument, NULL, OPT_MPTABLE },
	{ "msr",                no_argument, NULL, 'm' },
	{ "mtrr",               no_argument, NULL, OPT_MTRR },
	{ "pm",                 no_argument, NULL, OPT_PM },
	{ "raw-cpuid",          no_argument, NULL, 'r' },
	{ "show-machine-check", no_argument, NULL, OPT_SHOW_MACHINE_CHECK },
	{ "verbose",            no_argument, NULL, 'v' },
	{ NULL, 0, NULL, 0 },
};

static void usage(const char *programname, int exit_code)
{
	printf("Usage: %s [<switches>]\n", programname);
	for (int i = 0; long_options[i].name; i++) {
		char short_opt = ' ';
		switch (long_options[i].val) {
			case 'a': short_opt = 'a'; break;
			case 'c': short_opt = 'c'; break;
			case 'f': short_opt = 'f'; break;
			case 'h': short_opt = 'h'; break;
			case 'm': short_opt = 'm'; break;
			case 'r': short_opt = 'r'; break;
			case 'v': short_opt = 'v'; break;
		}
		if (short_opt != ' ')
			printf("-%c, --%s\n", short_opt, long_options[i].name);
		else
			printf("    --%s\n", long_options[i].name);
	}
	exit(exit_code);
}

void parse_command_line(int argc, char **argv)
{
	int opt;

	while ((opt = getopt_long(argc, argv, "acfhmrv", long_options, NULL)) != -1) {
		switch (opt) {
			case 'a':  // --all
				if (user_is_root) {
					show_apic = 1;
					show_bios = 1;
					show_msr = 1;
					show_microcode = 1;
					need_root = 1;
					show_mptable = 1;
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
				break;
			case 'c':  // --cache
				show_cacheinfo = 1;
				break;
			case 'f':  // --flags
				show_feature_flags = 1;
				break;
			case 'h':  // --help
				usage(argv[0], EXIT_SUCCESS);
				break;
			case 'm':  // --msr
				need_root = 1;
				show_msr = 1;
				break;
			case 'r':  // --raw-cpuid
				show_cpuid = 1;
				break;
			case 'v':  // --verbose
				verbose = 1;
				break;
			case OPT_ALL_CPUS:  // --all-cpus
				all_cpus = 1;
				break;
			case OPT_ADDR:  // --addr
				show_addr_sizes = 1;
				break;
			case OPT_APIC:  // --apic
				need_root = 1;
				show_apic = 1;
				break;
			case OPT_BIOS:  // --bios
				need_root = 1;
				show_bios = 1;
				break;
			case OPT_BUGS:  // --bugs
				show_bugs = 1;
				break;
			case OPT_CONNECTOR:  // --connector
				show_connector = 1;
				break;
			case OPT_DEBUG:  // --debug
				debug = 1;
				break;
			case OPT_SHOW_MACHINE_CHECK:  // --show-machine-check
				need_root = 1;
				show_machine_check = 1;
				break;
			case OPT_MHZ:  // --mhz
				show_MHz = 1;
				break;
			case OPT_MICROCODE:  // --microcode
				need_root = 1;
				show_microcode = 1;
				break;
			case OPT_MPTABLE:  // --mptable
				need_root = 1;
				show_mptable = 1;
				break;
			case OPT_MTRR:  // --mtrr
				need_root = 1;
				show_mtrr = 1;
				break;
			case OPT_PM:  // --pm
				show_pm = 1;
				break;
			default:
				usage(argv[0], EXIT_FAILURE);
				break;
		}
	}
}
