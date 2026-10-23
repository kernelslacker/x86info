/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Longhaul register decoding.
 */
#include <stdio.h>
#include <x86info.h>
#include "centaur.h"
#include "powersaver.h"

void decode_longhaul(struct cpudata *cpu)
{
	union msr_powersaver longhaul;

	if (read_msr(cpu->number, MSR_POWERSAVER, &longhaul.val) == 1) {
		dumpmsr (cpu->number, MSR_POWERSAVER, 64);

		if (longhaul.bits.RevisionID & 1)
			printf("\tSoftVID support\n");
		if (longhaul.bits.RevisionID & 2)
			printf("\tSoftBSEL support\n");
		if (longhaul.bits.RevisionID == 0)
			printf("\tSoftware clock multiplier only: No Softvid\n");

		if (longhaul.bits.EnableSoftBusRatio==1)
			printf("\tEnableSoftBusRatio=Enabled\n");
		if (longhaul.bits.EnableSoftVID==1)
			printf("\tEnableSoftVID=Enabled\n");
		if (longhaul.bits.EnableSoftBSEL==1)
			printf("\tEnableSoftBSEL=Enabled\n");

		printf("\tSoftBusRatio4=%s\n", longhaul.bits.SoftBusRatio4 ? "1" : "0");
		printf("\tSoftBusRatio=");
		binary (4, longhaul.bits.SoftBusRatio);

		if (longhaul.bits.RevisionID & 1)
			printf("\tVRM Rev=%s\n",
				longhaul.bits.VRMRev ? "Mobile VRM" : "VRM 8.5");

		printf("\tMaxMHzBR4: %s\n", longhaul.bits.MaxMHzBR4 ? "1" : "0");
		printf("\tMaxMHzBR: ");
		binary (4, longhaul.bits.MaxMHzBR);
		printf("\tMaximumVID: ");
		binary (5, longhaul.bits.MaximumVID);
		printf("\tMaxMHzFSB: ");
		binary (2, longhaul.bits.MaxMHzFSB);
		printf("\tMinMHzBR4: %s\n", longhaul.bits.MinMHzBR4 ? "1" : "0");
		printf("\tMinMHzBR: ");
		binary (4, longhaul.bits.MinMHzBR);
		printf("\tMinimumVID: ");
		binary (5, longhaul.bits.MinimumVID);
		printf("\tMinMHzFSB: ");
		binary (2, longhaul.bits.MinMHzFSB);
	}
}
