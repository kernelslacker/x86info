/*
 *  $Id: x86info.c,v 1.26 2001/08/10 09:00:03 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "x86info.h"

int show_msr=0;
int show_registers=0;
int show_flags=0;
int show_cacheinfo=0;
int show_all=0;
int show_MHz=0;
int show_bluesmoke=0;
int check_bluesmoke=0;
int parse_bluesmoke=0;

int silent = 0;
int used_UP = 0;

void usage (char *programname)
{
	printf ("Usage: %s [<switches>]\n\
-a,   --all\n\
-c,   --cacheinfo \n\
-f,   --flags\n\
-mhz, --mhz\n\
-m,   --msr\n\
-r,   --registers\n\
-s,   --show-bluesmoke\n\
      --check-bluesmoke\n\
      --parse-bluesmoke\n\
\n", programname);
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
			show_msr =1;
			show_MHz =1;
			show_bluesmoke =1;
		}

		if ((!strcmp(arg, "-c") || !strcmp(arg, "--cache")))
			show_cacheinfo = 1;

		if ((!strcmp(arg, "-f") || !strcmp(arg, "--flags")))
			show_flags = 1;

		if ((!strcmp(arg, "-m") || !strcmp(arg, "--msr")))
			show_msr= 1;

		if ((!strcmp(arg, "-mhz") || !strcmp(arg, "--mhz")))
			show_MHz= 1;

		if ((!strcmp(arg, "-r") || !strcmp(arg, "--registers")))
			show_registers = 1;

		if ((!strcmp(arg, "-s") || !strcmp(arg, "--show-bluesmoke")))
			show_bluesmoke = 1;

		if (!strcmp(arg, "--check-bluesmoke")) {
			check_bluesmoke = 1;
			silent = 1;
		}

		if (!strcmp(arg, "--parse-bluesmoke"))
			parse_bluesmoke = 1;

		if ((!strcmp(arg, "?") || !strcmp(arg, "--help")))
			usage(argv[0]);	
	}
}


int main (int argc, char **argv)
{
	unsigned int i, nrCPUs;

	parse_command_line(argc, argv);
	if (!silent) {
		printf ("x86info v1.5.  Dave Jones 2001\n");
		printf ("Feedback to <davej@suse.de>.\n\n");
	}

	if ((HaveCPUID())==0) {
		printf ("No CPUID instruction available.\n");
		printf ("No further information available for this CPU.\n");
		return(0);
	}

	nrCPUs = sysconf (_SC_NPROCESSORS_CONF);
	if (!silent) {
		printf ("Found %d CPU", nrCPUs);
		if (nrCPUs > 1)
			printf ("s");
		printf ("\n");
	}

	for (i=0; i<nrCPUs; i++) {
		if (show_registers)
			dumpregs(i);
		identify(i);

		/*
		 * Doing this per-cpu is a problem, as we can't
		 * schedule userspace code per-cpu. */
		if (show_MHz)
			estimate_MHz(i);
	}

	if (nrCPUs > 1 && used_UP==1 && (!silent)) {
		printf ("WARNING: Detected SMP, but cpuid driver not loaded.\n");
		printf ("Used single CPU routines. Results inaccurate.\n");
	}
	return (0);
}

