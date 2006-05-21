/*
 *  $Id: bluesmoke.c,v 1.9 2003/04/11 00:17:20 davej Exp $
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
#include "../x86info.h"

#define MCG_CAP 0x0179
#define MCG_CTL_PBIT	8
#define MCG_EXT_PBIT	9
#define MCG_STATUS 0x17a
#define MCG_CTL 0x17b
#define MCG_EXT 0x180

#define MC_CTL 0x0400
#define MC_STATUS 0x0401
#define MC_ADDR 0x402

#define PENTIUM4(family)	(family == 0xf)

void decode_Intel_bluesmoke(int cpunum, int family)
{
	unsigned long long val, val2;
	int banks, i, extcount = 0;

	if (!user_is_root)
		return;

	if (read_msr(cpunum, MCG_CAP, &val) != 1)
		return;

	banks = val & 0xff;

	printf ("\nNumber of reporting banks : %d\n\n", banks);

	if (PENTIUM4(family)) {
		if ((val & (1<<MCG_EXT_PBIT))) {
			extcount = (val >> 16) & 0xff;
			printf ("Number of extended MC registers : %d\n\n", extcount);
		}
		else
			printf ("Erk, MCG_EXT not present! :%016llx:\n", val);
	}
	else
	if ((val & (1<<MCG_CTL_PBIT)) == 0)
		printf ("Erk, MCG_CTL not present! :%016llx:\n", val);

	if (read_msr(cpunum, MCG_CTL, &val) == 1) {
		printf ("MCG_CTL:\n");

		printf (" Data cache check %sabled\n", val & (1<<0) ? "en" : "dis");
		if ((val & (1<<0)) == 1) {
			if (read_msr(cpunum, MC_CTL, &val2) == 1) {
				printf ("  ECC 1 bit error reporting %sabled\n", val2 & (1<<0) ? "en" : "dis");
				printf ("  ECC multi bit error reporting %sabled\n", val2 & (1<<1) ? "en" : "dis");
				printf ("  Data cache data parity %sabled\n", val2 & (1<<2) ? "en" : "dis");
				printf ("  Data cache main tag parity %sabled\n", val2 & (1<<3) ? "en" : "dis");
				printf ("  Data cache snoop tag parity %sabled\n", val2 & (1<<4) ? "en" : "dis");
				printf ("  L1 TLB parity %sabled\n", val2 & (1<<5) ? "en" : "dis");
				printf ("  L2 TLB parity %sabled\n", val2 & (1<<6) ? "en" : "dis");
			}
		}

		printf (" Instruction cache check %sabled\n", val & (1<<1) ? "en" : "dis");
		if (((val & (1<<1)) == 2) && (banks>1)) {
			if (read_msr(cpunum, MC_CTL+4, &val2) == 1) {
				printf ("  ECC 1 bit error reporting %sabled\n", val2 & (1<<0) ? "en" : "dis");
				printf ("  ECC multi bit error reporting %sabled\n", val2 & (1<<1) ? "en" : "dis");
				printf ("  Instruction cache data parity %sabled\n", val2 & (1<<2) ? "en" : "dis");
				printf ("  IC main tag parity %sabled\n", val2 & (1<<3) ? "en" : "dis");
				printf ("  IC snoop tag parity %sabled\n", val2 & (1<<4) ? "en" : "dis");
				printf ("  L1 TLB parity %sabled\n", val2 & (1<<5) ? "en" : "dis");
				printf ("  L2 TLB parity %sabled\n", val2 & (1<<6) ? "en" : "dis");
				printf ("  Predecode array parity %sabled\n", val2 & (1<<7) ? "en" : "dis");
				printf ("  Target selector parity %sabled\n", val2 & (1<<8) ? "en" : "dis");
				printf ("  Read data error %sabled\n", val2 & (1<<9) ? "en" : "dis");
			}
		}

		printf (" Bus unit check %sabled\n", val & (1<<2) ? "en" : "dis");
		if ((val & (1<<2)) == 4 && (banks>2)) {
			if (read_msr(cpunum, MC_CTL+8, &val2) == 1) {
				printf ("  External L2 tag parity error %sabled\n", val2 & (1<<0) ? "en" : "dis");
				printf ("  L2 partial tag parity error %sabled\n", val2 & (1<<1) ? "en" : "dis");
				printf ("  System ECC TLB reload error %sabled\n", val2 & (1<<2) ? "en" : "dis");
				printf ("  L2 ECC TLB reload error %sabled\n", val2 & (1<<3) ? "en" : "dis");
				printf ("  L2 ECC K7 deallocate %sabled\n", val2 & (1<<4) ? "en" : "dis");
				printf ("  L2 ECC probe deallocate %sabled\n", val2 & (1<<5) ? "en" : "dis");
				printf ("  System datareaderror reporting %sabled\n", val2 & (1<<6) ? "en" : "dis");
			}
		}

		printf (" Load/Store unit check %sabled\n", val & (1<<3) ? "en" : "dis");
		if ((val & (1<<3)) == 8 && (banks>3)) {
			if (read_msr(cpunum, MC_CTL+12, &val2) == 1) {
				printf ("  Read data error enable (loads) %sabled\n", val2 & (1<<0) ? "en" : "dis");
				printf ("  Read data error enable (stores) %sabled\n", val2 & (1<<1) ? "en" : "dis");
			}
		}
	}
	printf ("\n");

	for (i=0; i<banks; i++) {
		printf ("Bank: %d (0x%x)\n", i, (unsigned int)MC_CTL+i*4);
		printf ("MC%dCTL:    ", i);
		dumpmsr_bin (cpunum, MC_CTL+i*4, 64);
		printf ("MC%dSTATUS: ", i);
		dumpmsr_bin (cpunum, MC_STATUS+i*4, 64);
		printf ("MC%dADDR:   ", i);
		dumpmsr_bin (cpunum, MC_ADDR+i*4, 64);
		printf ("\n");
	}
}
