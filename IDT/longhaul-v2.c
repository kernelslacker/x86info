/*
 *  $Id: longhaul-v2.c,v 1.3 2003/07/15 19:04:53 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  IDT/Centaur specific parts.
 */
#include <stdio.h>
#include "../x86info.h"

union msr_longhaul {
	struct {
		unsigned RevisionID:4,	// 3:0
		RevisionKey:4,		// 7:4
		EnableSoftBusRatio:1,	// 8
		EnableSoftVID:1,	// 9
		EnableSoftBSEL:1,	// 10
		Reserved:3,		// 11:13
		SoftBusRatio4:1,	// 14
		VRMRev:1,		// 15
		SoftBusRatio:4,		// 19:16
		SoftVID:5,		// 24:20
		Reserved2:3,		// 27:25
		SoftBSEL:2,		// 29:28
		Reserved3:2,		// 31:30
		MaxMHzBR:4,		// 35:32
		MaximumVID:5,		// 40:36
		MaxMHzFSB:2,		// 42:41
		MaxMHzBR4:1,		// 43
		Reserved4:4,		// 47:44
		MinMHzBR:4,		// 51:48
		MinimumVID:5,		// 56:52
		MinMHzFSB:2,		// 58:57
		MinMHzBR4:1,		// 59
		Reserved5:4;		// 63:60
	} bits;
	unsigned long long val;
};

void decode_longhaul2(struct cpudata *cpu)
{
	union msr_longhaul longhaul;

	if (read_msr(cpu->number, 0x110A, &longhaul.val) == 1) {
		dumpmsr (cpu->number, 0x110A, 64);

		if (longhaul.bits.RevisionID & 1)
			printf ("\tSoftVID support\n");
		if (longhaul.bits.RevisionID & 2)
			printf ("\tSoftBSEL support\n");
		if (longhaul.bits.RevisionID == 0)
			printf ("\tSoftware clock multiplier only: No Softvid\n");

		if (longhaul.bits.EnableSoftBusRatio==1)
			printf ("\tEnableSoftBusRatio=Enabled\n");
		if (longhaul.bits.EnableSoftVID==1)
			printf ("\tEnableSoftVID=Enabled\n");
		if (longhaul.bits.EnableSoftBSEL==1)
			printf ("\tEnableSoftBSEL=Enabled\n");

		printf ("\tSoftBusRatio4=%s\n", longhaul.bits.SoftBusRatio4 ? "1" : "0");
		printf ("\tSoftBusRatio=");
		binary (4, longhaul.bits.SoftBusRatio);

		if (longhaul.bits.RevisionID & 1)
			printf ("\tVRM Rev=%s\n",
				longhaul.bits.VRMRev ? "Mobile VRM" : "VRM 8.5");

		printf ("\tMaxMHzBR4: %s\n", longhaul.bits.MaxMHzBR4 ? "1" : "0");
		printf ("\tMaxMHzBR: ");
		binary (4, longhaul.bits.MaxMHzBR);
		printf ("\tMaximumVID: ");
		binary (5, longhaul.bits.MaximumVID);
		printf ("\tMaxMHzFSB: ");
		binary (2, longhaul.bits.MaxMHzFSB);
		printf ("\tMinMHzBR4: %s\n", longhaul.bits.MinMHzBR4 ? "1" : "0");
		printf ("\tMinMHzBR: ");
		binary (4, longhaul.bits.MinMHzBR);
		printf ("\tMinimumVID: ");
		binary (4, longhaul.bits.MinimumVID);
		printf ("\tMinMHzFSB: ");
		binary (2, longhaul.bits.MinMHzFSB);
	}
}
