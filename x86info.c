/*
 *  $Id: x86info.c,v 1.10 2001/03/14 18:45:53 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include "x86info.h"

int show_msr=0;
int show_registers=0;
int show_flags=0;
int show_cacheinfo=0;
int show_all=0;

void usage (char *programname)
{
	printf ("Usage: %s [<switches>]\n\
-a, --all\n\
-c, --cacheinfo \n\
-f, --flags\n\
-m, --msr\n\
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
			show_msr =1;
		}

		if ((!strcmp(arg, "-c") || !strcmp(arg, "--cache")))
			show_cacheinfo = 1;

		if ((!strcmp(arg, "-f") || !strcmp(arg, "--flags")))
			show_flags = 1;

		if ((!strcmp(arg, "-m") || !strcmp(arg, "--msr")))
			show_msr= 1;

		if ((!strcmp(arg, "-r") || !strcmp(arg, "--registers")))
			show_registers = 1;
		
		if ((!strcmp(arg, "?") || !strcmp(arg, "--help")))
			usage(argv[0]);	
	}
}


int main (int argc, char **argv)
{
	unsigned int i,n, nrCPUs;

	printf ("x86info v1.0.  Dave Jones 2001\n");
	printf ("Feedback to <davej@suse.de>.\n\n");

	parse_command_line(argc, argv);

	/* FIXME: Insert code here to test if CPUID instruction is available */

	nrCPUs = sysconf (_SC_NPROCESSORS_CONF);
	printf ("Found %d CPU", nrCPUs);
	if (nrCPUs > 1)
		printf ("s");
	printf ("\n");

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
