/*
 *  $Id: x86info.c,v 1.20 2001/05/22 14:53:22 davej Exp $
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

int used_UP = 0;

void usage (char *programname)
{
	printf ("Usage: %s [<switches>]\n\
-a, --all\n\
-c, --cacheinfo \n\
-f, --flags\n\
-mhz, --mhz\n\
-m, --msr\n\
-r, --registers\n\
-s, --smoke\n", programname);
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

		if ((!strcmp(arg, "-s") || !strcmp(arg, "--smoke")))
			show_bluesmoke = 1;

		if ((!strcmp(arg, "?") || !strcmp(arg, "--help")))
			usage(argv[0]);	
	}
}


int main (int argc, char **argv)
{
	unsigned int i, nrCPUs;

	printf ("x86info v1.4.  Dave Jones 2001\n");
	printf ("Feedback to <davej@suse.de>.\n\n");

	parse_command_line(argc, argv);

	/* FIXME: Insert code here to test if CPUID instruction is available */

	nrCPUs = sysconf (_SC_NPROCESSORS_CONF);
	printf ("Found %d CPU", nrCPUs);
	if (nrCPUs > 1)
		printf ("s");
	printf ("\n");

	for (i = 0; i<nrCPUs; i++) {
		if (show_registers)
			dumpregs(i);
		identify(i);
	}
	if (show_MHz)
		estimate_MHz();

	if (nrCPUs > 1 && used_UP==1) {
		printf ("WARNING: Detected SMP, but cpuid driver not loaded.\n");
		printf ("Used single CPU routines. Results inaccurate.\n");
	}
	return (0);
}

