/*
 *  $Id: x86info.c,v 1.48 2001/12/13 18:47:31 davej Exp $
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

#ifndef linux

#ifdef __WIN32__
#include <sys/types.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#endif /* __WIN32__ */

#endif /* linux */

#include "x86info.h"

int show_msr=0;
int show_registers=0;
int show_flags=0;
int show_cacheinfo=0;
int show_all=0;
int show_MHz=0;
int show_mptable=0;
int show_bluesmoke=0;
int show_eblcr=0;

int silent = 0;
int used_UP = 0;
int user_is_root = 1;
int need_root = 0;

void usage (char *programname)
{
	printf ("Usage: %s [<switches>]\n\
-a,   --all\n\
-c,   --cacheinfo \n\
-f,   --flags\n\
-mhz, --mhz\n\
-mp,  --mptable\n\
-m,   --msr\n\
      --mult\n\
-r,   --registers\n\
-s,   --show-bluesmoke\n\
\n", programname);
	exit (0);
}

static void parse_command_line (int argc, char **argv)
{
	char **argp, *arg;

	for (argp = argv+1; argp <= argv + argc && (arg = *argp); argp++) {
		if ((!strcmp(arg, "-a") || !strcmp(arg, "--all"))) {
			show_bluesmoke =1;
			show_cacheinfo = 1;
			show_eblcr =1;
			show_flags = 1;
			show_mptable =1;
			show_msr = 1;
			show_MHz = 1;
			show_registers = 1;
			need_root = 1;
		}

		if ((!strcmp(arg, "-c") || !strcmp(arg, "--cache")))
			show_cacheinfo = 1;

		if ((!strcmp(arg, "-f") || !strcmp(arg, "--flags")))
			show_flags = 1;

		if ((!strcmp(arg, "-m") || !strcmp(arg, "--msr"))) {
			need_root = 1;
			show_msr = 1;
		}

		if (!strcmp(arg, "--mult")) {
			need_root = 1;
			show_eblcr = 1;
		}

		if ((!strcmp(arg, "-mhz") || !strcmp(arg, "--mhz")))
			show_MHz = 1;

		if ((!strcmp(arg, "-mp") || !strcmp(arg, "--mptable"))) {
			need_root = 1;
			show_mptable = 1;
		}

		if ((!strcmp(arg, "-r") || !strcmp(arg, "--registers")))
			show_registers = 1;

		if ((!strcmp(arg, "-s") || !strcmp(arg, "--show-bluesmoke"))) {
			need_root = 1;
			show_bluesmoke = 1;
		}

		if ((!strcmp(arg, "?") || !strcmp(arg, "--help")))
			usage(argv[0]);	
	}
}


int main (int argc, char **argv)
{
	unsigned int i, nrCPUs=1, nrSMPCPUs;
	struct cpudata cpu;

	parse_command_line(argc, argv);
	if (!silent) {
		printf ("x86info v1.7.  Dave Jones 2001\n");
		printf ("Feedback to <davej@suse.de>.\n\n");
	}

	if ((HaveCPUID())==0) {
		printf ("No CPUID instruction available.\n");
		printf ("No further information available for this CPU.\n");
		return(0);
	}

	if (getuid()!=0)
		user_is_root=0;

	if (need_root==1 && user_is_root==0)
		printf ("Need to be root to use specified options.\n");

#if defined __WIN32__
	{
		SYSTEM_INFO sys_info;
		GetSystemInfo(&sys_info);
		nrCPUs = sys_info.dwNumberOfProcessors;
	}
#elif defined _SC_NPROCESSORS_CONF	/* Linux */
	nrCPUs = sysconf (_SC_NPROCESSORS_CONF);
#else /* BSD */
	{
		int mib[2] = { CTL_HW, HW_NCPU };
		size_t len;
		len = sizeof(nrCPUs);
		sysctl(mib, 2, &nrCPUs, &len, NULL, 0);
	}
#endif /* __WIN32__ */

	if (!silent) {
		printf ("Found %u CPU", nrCPUs);
		if (nrCPUs > 1)
			printf ("s");

		/* Check mptable if present. This way we get number of CPUs
		   on SMP systems that have booted UP kernels. */
		if (user_is_root==1) {
			issmp (&nrSMPCPUs, 0);
			if (nrSMPCPUs > nrCPUs) {
				printf (", but found %d CPUs in MPTable.", nrSMPCPUs);
			}
		}
		printf ("\n");
	}

	/*
	 * can't have less than 1 CPU, or more than
	 * 65535 (some arbitrary large number)
	 */
	if ((nrCPUs < 1) || (nrCPUs > 65535)) {
		printf("CPU count is bogus: defaulting to 1 CPU.\n");
		nrCPUs = 1;
	}

	if (show_mptable && user_is_root)
		issmp (&nrSMPCPUs, 1);

	for (i=0; i<nrCPUs; i++) {
		cpu.number = i;

		if (!silent && nrCPUs!=1)
			printf ("CPU #%u\n", i+1);

		identify (&cpu);

		/*
		 * Doing this per-cpu is a problem, as we can't
		 * schedule userspace code per-cpu.
		 * Although running nrCPUs * threads would probably work.
		 */
		if (show_MHz)
			estimate_MHz(i);
	}

	if (nrCPUs > 1 && used_UP==1 && (!silent)) {
		printf ("\nWARNING: Detected SMP, but unable to access cpuid driver.\n");
		printf ("Used single CPU routines. Results inaccurate.\n");
	}

	return (0);
}
