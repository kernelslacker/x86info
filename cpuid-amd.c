/* x86info Dave Jones.
 * Based on 'cpuid' by Phil Karn, KA9Q.
 * May be used under the terms of the GNU Public License (GPL)
 *
 * AMD-specific information
 *
 */

#include <stdio.h>
#include "x86info.h"

void doamd(int maxi, struct cpudata *cpu)
{
	unsigned int i;
	unsigned long maxei, eax, ebx, ecx, edx;

	cpu->vendor = VENDOR_AMD;

	cpuid (0x80000000, &maxei, NULL, NULL, NULL);
	if (maxei != 0){

		/* Dump extended info in raw hex */
		for (i=0x80000000;i<=maxei;i++){
			cpuid (i, &eax, &ebx, &ecx, &edx);
			printf ("eax in: 0x%x, eax = %08lx ebx = %08lx ecx = %08lx edx = %08lx\n",i,eax,ebx,ecx,edx);
		}
	}

	if(maxi >= 0x00000001) {
		cpuid (0x00000001, &eax, &ebx, &ecx, &edx);
		cpu->stepping = eax & 0xf;
		cpu->model = (eax >> 4) & 0xf;
		cpu->family = (eax >> 8) & 0xf;

		printf ("Family: %d Model: %d [", cpu->family, cpu->model);
		switch (cpu->family) {
			case 4:
				printf("Am486/Am5x86");
				break;

			case 5:
				switch (cpu->model) {
					case 0 ... 3:
						printf ("K5");
						break;
					case 4 ... 7:
						printf ("K6");
						break;
					case 8:
						printf ("K6-2");
						break;
					case 9:
						printf ("K6-3");
						break;
					case 13:
						printf ("K6-2+");
					default:
						printf ("Unknown CPU");
						break;
				}
				break;

			case 6:
				printf("K7/Athlon");
				break;
		}
		printf("]\n");
	}
	
	/* Check for presence of extended info */
	if (maxei == 0)
		return;

	if(maxei >= 0x80000001) {
		cpuid (0x80000001, &eax, &ebx, &ecx, &edx);

		printf("Feature flags:\n");
		decode_feature_flags(cpu, edx);
	}

	if (maxei >= 0x80000002) {
		/* Processor identification string */
		char namestring[49],*cp;
		unsigned int j;
		cp = namestring;
		for (j=0x80000002;j<=0x80000004;j++) {
			cpuid (j, &eax, &ebx, &ecx, &edx);

			for (i=0;i<4;i++)
				*cp++ = eax >> (8*i);
			for (i=0;i<4;i++)
				*cp++ = ebx >> (8*i);
			for (i=0;i<4;i++)
				*cp++ = ecx >> (8*i);
			for (i=0;i<4;i++)
				*cp++ = edx >> (8*i);
		}
		printf("Processor name string: %s\n",namestring);
	}

	if (maxei >= 0x80000005) {
		/* TLB and cache info */
		cpuid (0x80000005, &eax, &ebx, &ecx, &edx);
		printf ("Data TLB: associativity %ld #entries %ld\n",
			ebx >> 24,(ebx >> 16) & 0xff);
		printf ("Instruction TLB: associativity %ld #entries %ld\n",
			(ebx >> 8) & 0xff,ebx & 0xff);
		printf ("L1 Data cache: size %ld KB associativity %lx lines per tag %ld line size %ld\n",
			ecx >> 24,(ecx>>16) & 0xff,(ecx >> 8)&0xff,ecx&0xff);
		printf("L1 Instruction cache: size %ld KB associativity %lx lines per tag %ld line size %ld\n",
			edx >> 24,(edx>>16) & 0xff,(edx >> 8)&0xff,edx&0xff);
	}
  
	/* check K6-III (and later?) on-chip L2 cache size */
	if (maxei >= 0x80000006) {
		cpuid (0x80000006, &eax, &ebx, &ecx, &edx);
		printf ("L2 (on CPU) cache: %ld KB associativity %lx lines per tag %ld line size %ld\n",
		ecx >> 16, (ecx >> 12) & 0x0f, (ecx >> 8) & 0x0f, ecx & 0xff);
	}
}
