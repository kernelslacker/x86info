/* x86info by Dave Jones
 * Based in part on 'cpuid' by Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#include <stdio.h>
#include "x86info.h"


int main (void)
{
	struct cpudata cpu;
	unsigned int i;
	unsigned long maxi, eax, ebx, ecx, edx;

	/* FIXME: Insert code here to test if CPUID instruction is available */

	/* Dump all the CPUID results in raw hex */
	cpuid (0, &maxi, NULL, NULL, NULL);
	maxi &= 0xffff;		/* The high-order word is non-zero on some Cyrix CPUs */
	for (i = 0; i <= maxi; i++) {
		cpuid (i, &eax, &ebx, &ecx, &edx);
		printf ("eax in: %i, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n", i, eax, ebx, ecx, edx);
	}

	/* Vendor ID and max CPUID level supported */
	cpuid (0, &eax, &ebx, &ecx, &edx);
	printf ("Vendor ID: \"");
	for (i = 0; i < 4; i++)
		putchar (ebx >> (8 * i));
	for (i = 0; i < 4; i++)
		putchar (edx >> (8 * i));
	for (i = 0; i < 4; i++)
		putchar (ecx >> (8 * i));
	printf ("\"; Max CPUID level %ld\n\n", maxi);

	switch (ebx) {
	case 0x756e6547:	/* Intel */
		dointel (maxi, &cpu);
		break;

	case 0x68747541:	/* AMD */
		doamd (maxi, &cpu);
		break;

	case 0x69727943:	/* Cyrix */
		docyrix (maxi, &cpu);
		break;

	case 0x746e6543:	/* IDT */
		doIDT (maxi, &cpu);
		break;

	default:
		printf ("Unknown vendor\n");
		break;
	}
	return (0);
}
