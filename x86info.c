/* x86info by Dave Jones
 * Based in part on 'cpuid' by Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#include <stdio.h>
#include <unistd.h>
#include "x86info.h"


int main (void)
{
	unsigned int i,n, nrCPUs;

	/* FIXME: Insert code here to test if CPUID instruction is available */

	nrCPUs = sysconf (_SC_NPROCESSORS_CONF);
	printf ("%d CPUs found\n", nrCPUs);

	for (i = 1; i<=nrCPUs; i++) {
	
		if (nrCPUs == 1)
			n = 0;
		else
			n = i;

		dumpregs(n);
		identify(n);
	}

	return (0);
}

