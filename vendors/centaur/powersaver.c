/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Longhaul and Powersaver MSR decoding.
 */

#include <stdio.h>
#include <x86info.h>
#include "centaur.h"
#include "powersaver.h"

static void show_bit(const char *name, const char *sep, unsigned int v)
{
	printf("\t%s%s%s\n", name, sep, v ? "1" : "0");
}

static void show_field(const char *name, const char *sep, unsigned int width,
		       unsigned int v)
{
	printf("\t%s%s", name, sep);
	binary(width, v);
}

static void show_vrm(unsigned int v)
{
	printf("\tVRM Rev=%s\n", v ? "Mobile VRM" : "VRM 8.5");
}

static void show_bus_ratio(const union msr_powersaver *ps)
{
	show_bit("SoftBusRatio4", "=", ps->bits.SoftBusRatio4);
	show_field("SoftBusRatio", "=", 4, ps->bits.SoftBusRatio);
}

static void decode_longhaul(const union msr_powersaver *lh)
{
	if (lh->bits.RevisionID & 1)
		printf("\tSoftVID support\n");
	if (lh->bits.RevisionID & 2)
		printf("\tSoftBSEL support\n");
	if (lh->bits.RevisionID == 0)
		printf("\tSoftware clock multiplier only: No Softvid\n");

	if (lh->bits.EnableSoftBusRatio == 1)
		printf("\tEnableSoftBusRatio=Enabled\n");
	if (lh->bits.EnableSoftVID == 1)
		printf("\tEnableSoftVID=Enabled\n");
	if (lh->bits.EnableSoftBSEL == 1)
		printf("\tEnableSoftBSEL=Enabled\n");

	show_bus_ratio(lh);

	if (lh->bits.RevisionID & 1)
		show_vrm(lh->bits.VRMRev);

	show_bit("MaxMHzBR4", ": ", lh->bits.MaxMHzBR4);
	show_field("MaxMHzBR", ": ", 4, lh->bits.MaxMHzBR);
	show_field("MaximumVID", ": ", 5, lh->bits.MaximumVID);
	show_field("MaxMHzFSB", ": ", 2, lh->bits.MaxMHzFSB);
	show_bit("MinMHzBR4", ": ", lh->bits.MinMHzBR4);
	show_field("MinMHzBR", ": ", 4, lh->bits.MinMHzBR);
	show_field("MinimumVID", ": ", 5, lh->bits.MinimumVID);
	show_field("MinMHzFSB", ": ", 2, lh->bits.MinMHzFSB);
}

static void decode_powersaver(const union msr_powersaver *ps)
{
	printf(" RevisionID: %x : ", ps->bits.RevisionID);
	switch (ps->bits.RevisionID) {
	case 0x0:
		printf("Initial revision (Software clock multiplier only, no SoftVID)\n");
		break;
	case 0x1:
		printf("SoftVID support\n");
		break;
	default:
		printf("Unknown (0x%x).\n", ps->bits.RevisionID);
		break;
	}

	printf(" Software clock multiplier is ");
	if (ps->bits.EnableSoftBusRatio == 0) {
		printf("disabled\n");
	} else {
		printf("enabled\n");
		show_bit("MaxMHzBR4", ": ", ps->bits.MaxMHzBR4);
		show_field("MaxMHzBR", ": ", 4, ps->bits.MaxMHzBR);
		show_bit("MinMHzBR4", ": ", ps->bits.MinMHzBR4);
		show_field("MinMHzBR", ": ", 4, ps->bits.MinMHzBR);
	}

	/* these bits invalid if revision == 0*/
	if (ps->bits.RevisionID == 0)
		return;

	printf(" Software VID is ");
	if (ps->bits.EnableSoftVID == 0) {
		printf("disabled\n");
	} else {
		printf("enabled\n");
		show_vrm(ps->bits.VRMRev);
		show_field("MinimumVID", ": ", 5, ps->bits.MinimumVID);
		show_field("MaximumVID", ": ", 5, ps->bits.MaximumVID);
	}

	if (ps->bits.EnableSoftBusRatio == 1) {
		printf("\tEnableSoftBusRatio=Enabled\n");
		show_field("MaxMHzFSB", ": ", 2, ps->bits.MaxMHzFSB);
		show_field("MinMHzFSB", ": ", 2, ps->bits.MinMHzFSB);
	}

	show_bus_ratio(ps);
}

void decode_c3_power(struct cpudata *cpu, enum c3_power gen)
{
	union msr_powersaver ps;

	if (gen == C3_POWERSAVER) {
		dumpmsr(cpu->number, MSR_POWERSAVER, 64, true);
		printf("\n");
	}

	if (read_msr(cpu->number, MSR_POWERSAVER, &ps.val) != 1) {
		if (gen == C3_POWERSAVER)
			printf("Something went wrong reading MSR_POWERSAVER\n");
		return;
	}

	if (gen == C3_LONGHAUL) {
		dumpmsr(cpu->number, MSR_POWERSAVER, 64, true);
		decode_longhaul(&ps);
	} else {
		decode_powersaver(&ps);
	}
}
