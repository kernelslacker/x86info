/*
 *  $Id: MSR-C3.c,v 1.2 2001/12/11 01:16:14 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  IDT/Centaur specific parts.
 */
#include <stdio.h>
#include "../x86info.h"

void dump_C3_MSR (struct cpudata *cpu)
{
	unsigned long long val=0;
	int longhaul=0;

	if (!user_is_root)
		return;

	printf ("FCR: ");
	dumpmsr (cpu->number, 0x1107, 32);

	printf ("Longhaul v");
	if (cpu->model==6 || (cpu->model==7 && cpu->stepping==0))
		printf ("1.0");
	if (cpu->model==7 && cpu->stepping>0) {
		printf ("2.0");
		longhaul=2;
	}
	if (cpu->model==8)
		printf ("3.0");
	printf (" present\n");

	if (longhaul==2 && read_msr(cpu->number, 0x110A, &val) == 1) {
		dumpmsr (cpu->number, 0x110A, 64);
		if (val & 1)
			printf ("\tSoftVID support\n");
		if (val & 2)
			printf ("\tSoftBSEL support\n");
		if (val==0)
			printf ("\tSoftware clock multiplier only: No Softvid\n");

		printf ("\tRevision key: %llu\n", (val & (1<<7|1<<6|1<<5|1<<4)) >> 4);
		if (val & (1<<8))
			printf ("\tEnableSoftBusRatio=Enabled\n");
		if (val & (1<<9))
			printf ("\tEnableSoftVid=Enabled\n");
		if (val & (1<<10))
			printf ("\tEnableSoftBSEL=Enabled\n");

		printf ("\tSoftBusRatio=%llu\n", val & 1<<14);
		if (val & 0xf)
			printf ("\tVRM Rev=%s\n",
				((val & 1<<15)) ? "Mobile VRM" : "VRM 8.5");

		val = val>>32;
		binary32 (val);
		printf ("\tMaxMHzBR: %s%s%s%s\n",
			(((val & (1<<3)) >>3) ? "1" : "0"),
			(((val & (1<<2)) >>2) ? "1" : "0"),
			(((val & (1<<1)) >>1) ? "1" : "0"),
			(((val & (1<<0)) >>0) ? "1" : "0"));
		printf ("\tMaximumVID: %s%s%s%s%s\n",
			(((val & (1<<8)) >>8) ? "1" : "0"),
			(((val & (1<<7)) >>7) ? "1" : "0"),
			(((val & (1<<6)) >>6) ? "1" : "0"),
			(((val & (1<<5)) >>5) ? "1" : "0"),
			(((val & (1<<4)) >>4) ? "1" : "0"));
		printf ("\tMaxMHzFSB: %s%s\n",
			(((val & (1<<10)) >>10) ? "1" : "0"),
			(((val & (1<<9))  >>9) ? "1" : "0"));
		printf ("\tMinMHzBR: %s%s%s%s\n",
			(((val & (1<<19)) >>19) ? "1" : "0"),
			(((val & (1<<18)) >>18) ? "1" : "0"),
			(((val & (1<<17)) >>17) ? "1" : "0"),
			(((val & (1<<16)) >>16) ? "1" : "0"));
		printf ("\tMinimumVID: %s%s%s%s\n",
			(((val & (1<<23)) >>23) ? "1" : "0"),
			(((val & (1<<22)) >>22) ? "1" : "0"),
			(((val & (1<<21)) >>21) ? "1" : "0"),
			(((val & (1<<20)) >>20) ? "1" : "0"));
		printf ("\tMinMHzFSB: %s%s\n",
			(((val & (1<<26)) >>26) ? "1" : "0"),
			(((val & (1<<25)) >>25) ? "1" : "0"));
	}
}
