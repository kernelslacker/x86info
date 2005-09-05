/*
 *  $Id: powernow.c,v 1.10 2003/03/11 13:57:39 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific information
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "AMD.h"
#include "powernow.h"

double mobile_vid_table[32] = {
	2.000, 1.950, 1.900, 1.850, 1.800, 1.750, 1.700, 1.650,
	1.600, 1.550, 1.500, 1.450, 1.400, 1.350, 1.300, 0.000,
	1.275, 1.250, 1.225, 1.200, 1.175, 1.150, 1.125, 1.100,
	1.075, 1.050, 1.024, 1.000, 0.975, 0.950, 0.925, 0.000,
};

double fid_codes[32] = {
	11.0, 11.5, 12.0, 12.5, 5.0, 5.5, 6.0, 6.5,
	7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10, 10.5,
	3.0, 19.0, 4.0, 20.0, 13.0, 13.5, 14.0, 21.0,
	15.0, 22.5, 16.0, 16.5, 17.0, 18.0, -1, -1,
};

void decode_powernow(struct cpudata *cpu)
{
	unsigned long eax, ebx, ecx, edx;
	union msr_vidctl vidctl;
	union msr_fidvidstatus fidvidstatus;
	int can_scale_vid=0, can_scale_bus=0;

	if (cpu->maxei < 0x80000007)
		return;

	cpuid(cpu->number, 0x80000007, &eax, &ebx, &ecx, &edx);
	printf("PowerNOW! Technology information\n");
	printf("Available features:");

	if (edx & (1<<0))
		printf("\n\tTemperature sensing diode present.");

	if (edx & (1<<1)) {
		printf("\n\tBus divisor control");
		can_scale_bus=1;
	}

	if (edx & (1<<2)) {
		printf("\n\tVoltage ID control\n");
		can_scale_vid=1;
	}

	if (edx & (1<<3))
		printf ("\n\tThermal Trip\n");

	if (edx & (1<<4))
		printf ("\n\tThermal Monitoring\n");

	if (edx & (1<<5))
		printf ("\n\tSoftware Thermal Control\n");
	if (edx & (1<<6))
		printf ("100MHz multiplier control\n");

	if (!(edx & (1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6)))
		printf(" None\n");
	printf("\n");

	if (can_scale_bus==0 && can_scale_vid==0)
		return;

	if (!user_is_root)
		return;

	dumpmsr(cpu->number, MSR_FID_VID_CTL, 64);
	dumpmsr(cpu->number, MSR_FID_VID_STATUS, 64);
	printf("\n");

	if (read_msr(cpu->number, MSR_FID_VID_CTL, &vidctl.val) != 1) {
		printf ("Something went wrong reading MSR_FID_VID_CTL\n");
		return;
	}

	printf ("FID changes %s happen\n", vidctl.bits.FIDC ? "will" : "won't");
	printf ("VID changes %s happen\n", vidctl.bits.VIDC ? "will" : "won't");

	if (vidctl.bits.VIDC)
		printf ("Current VID multiplier code: %0.3f\n", mobile_vid_table[vidctl.bits.VID]);
	if (vidctl.bits.FIDC)
		printf ("Current FSB multiplier code: %.1f\n", fid_codes[vidctl.bits.FID]);

	/* Now dump the status */

	if (read_msr(cpu->number, MSR_FID_VID_STATUS, &fidvidstatus.val) != 1) {
		printf ("Something went wrong reading MSR_FID_VID_STATUS\n");
		return;
	}


	printf ("Voltage ID codes: Maximum=%0.3fV Startup=%0.3fV Currently=%0.3fV\n",
		mobile_vid_table[fidvidstatus.bits.MVID],
		mobile_vid_table[fidvidstatus.bits.SVID],
		mobile_vid_table[fidvidstatus.bits.CVID]);

	printf ("Frequency ID codes: Maximum=%.1fx Startup=%.1fx Currently=%.1fx\n",
		fid_codes[fidvidstatus.bits.MFID],
		fid_codes[fidvidstatus.bits.SFID],
		fid_codes[fidvidstatus.bits.CFID]);

//	printf ("Voltage ID codes: Maximum=0x%x Startup=0x%x Currently=0x%x\n",
//		fidvidstatus.MVID, fidvidstatus.SVID, fidvidstatus.CVID);
//	printf ("Frequency ID codes: Maximum=0x%x Startup=0x%x Currently=0x%x\n",
//		fidvidstatus.MFID, fidvidstatus.SFID, fidvidstatus.CFID);

	if (show_bios) {
		printf ("Decoding BIOS PST tables (maxfid=%x, startvid=%x)\n",
					fidvidstatus.bits.MFID, fidvidstatus.bits.SVID);
		dump_PSB(cpu, fidvidstatus.bits.MFID, fidvidstatus.bits.SVID);
	}
}

