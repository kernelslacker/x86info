/* x86info by Dave Jones
 * Based in part on 'cpuid' by Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#include <stdio.h>
#include <unistd.h>
#include "x86info.h"

int show_registers=0;
int show_flags=0;
int show_cacheinfo=0;
int show_all=0;

void usage (char *programname)
{
	printf ("Usage: %s [OPTION]\n\
-a, --all\n\
-c, --cacheinfo \n\
-f, --flags \n\
-r, --registers\n\n", programname);
	exit (0);
}

void parse_command_line (int argc, char **argv)
{
	char **argp, *arg;

	for (argp = argv+1; argp <= argv + argc && (arg = *argp); argp++) {
		if ((!strcmp(arg, "-a") || !strcmp(arg, "--all"))) {
			show_registers = 1;
			show_flags = 1;
			show_cacheinfo = 1;
		}

		if ((!strcmp(arg, "-f") || !strcmp(arg, "--flags")))
			show_flags = 1;

		if ((!strcmp(arg, "-c") || !strcmp(arg, "--cache")))
			show_cacheinfo = 1;

		if ((!strcmp(arg, "-r") || !strcmp(arg, "--registers")))
			show_registers = 1;
		
		if ((!strcmp(arg, "?") || !strcmp(arg, "--help")))
			usage(argv[0]);	
	}
}


int main (int argc, char **argv)
{
	unsigned int i,n, nrCPUs;

	parse_command_line(argc, argv);
	
	/* FIXME: Insert code here to test if CPUID instruction is available */

	nrCPUs = sysconf (_SC_NPROCESSORS_CONF);
	printf ("%d CPUs found\n", nrCPUs);

	for (i = 1; i<=nrCPUs; i++) {
	
		if (nrCPUs == 1)
			n = 0;
		else
			n = i;

		if (show_registers)
			dumpregs(n);
		identify(n);
	}

	return (0);
}
