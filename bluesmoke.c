/*
 *  $Id: bluesmoke.c,v 1.1 2001/04/20 04:34:44 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Dump MCA registers.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "x86info.h"

#define MCG_CAP 0x0179
#define MCG_STAT 0x17a
#define MCG_CTL 0x17b

#define MC_CTL 0x0400
#define MC_STATUS 0x0401
#define MC_ADDR 0x402
#define MC_MISC 0x403

extern int show_bluesmoke;


void decode_bluesmoke(int cpunum)
{
	unsigned long long val=0;
	int banks, i;

	if (getuid()!=0) {
		printf ("Need to be root to access MSRs.\n");
		return;
	}

	if (rdmsr(cpunum, MCG_CAP, &val) != 1)
		return;

	if ((val & (1<<8)) == 0) {
		printf ("Erk, MCG_CTL not present!\n");
		return;
	}
	banks = val & 0xf;

	printf ("Number of reporting banks : %d\n", banks);

	printf("\t\t\t\t31       23       15       7 \n");
	for (i=0; i<banks; i++) {
		printf ("Bank: %d (0x%x)\n", i, MC_CTL+i*4);
		printf ("MC%dCTL:    ", i);
		dumpmsr_bin (cpunum, MC_CTL+i*4);
		printf ("MC%dSTATUS: ", i);
		dumpmsr_bin (cpunum, MC_STATUS+i*4);
		printf ("MC%dADDR:   ", i);
		dumpmsr_bin (cpunum, MC_ADDR+i*4);
		printf ("MC%dMISC:   ", i);
		dumpmsr_bin (cpunum, MC_MISC+i*4);
		printf ("\n");
	}
}
