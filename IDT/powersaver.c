/*
 *  $Id: powersaver.c,v 1.1 2003/01/27 17:33:16 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  C3 specific information
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "IDT.h"
#include "powersaver.h"

void decode_powersaver(struct cpudata *cpu)
{
	union msr_powersaver ps;

	if (!user_is_root)
		return;

	dumpmsr(cpu->number, MSR_POWERSAVER, 64);
	printf("\n");

	if (read_msr(cpu->number, MSR_POWERSAVER, &ps.val) != 1) {
		printf ("Something went wrong reading MSR_POWERSAVER\n");
		return;
	}

	printf (" RevisionID: %x : ", ps.bits.RevisionID);
	switch (ps.bits.RevisionID) {
		case 0xb:	printf ("Initial revision (Software clock multiplier only, no SoftVID)\n");
					break;
		case 0x1b:	printf ("SoftVID support\n");
					break;
		default:	printf ("Unknown.\n");
					break;
	}

	printf (" Software clock multiplier is ");
	if (ps.bits.EnableSoftBusRatio == 0)
		printf ("disabled\n");
	else
		printf ("enabled\n");

	/* these bits invalid if revision == 0*/
	if (ps.bits.RevisionID != 0) {
		printf (" Software VID is ");
		if (ps.bits.EnableSoftVID == 0)
			printf ("disabled\n");
		else
			printf ("enabled\n");

		printf ("VRM Rev: ");
		if (ps.bits.VRMRev == 0)
			printf ("VRM 8.5\n");
		else
			printf ("Mobile VRM\n");
	}
}

