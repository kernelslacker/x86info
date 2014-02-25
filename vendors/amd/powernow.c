/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Decode powernow registers.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pci/pci.h>
#include <x86info.h>
#include "amd.h"
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

static void decode_fidvid(struct cpudata *cpu)
{
	union msr_vidctl vidctl;
	union msr_fidvidstatus fidvidstatus;

	dumpmsr(cpu->number, MSR_FID_VID_CTL, 64);
	dumpmsr(cpu->number, MSR_FID_VID_STATUS, 64);
	printf("\n");

	if (read_msr(cpu->number, MSR_FID_VID_CTL, &vidctl.val) != 1) {
		printf("Something went wrong reading MSR_FID_VID_CTL\n");
		return;
	}

	printf("FID changes %s happen\n", vidctl.bits.FIDC ? "will" : "won't");
	printf("VID changes %s happen\n", vidctl.bits.VIDC ? "will" : "won't");

	if (vidctl.bits.VIDC)
		printf("Current VID multiplier code: %0.3f\n",
			mobile_vid_table[vidctl.bits.VID]);
	if (vidctl.bits.FIDC)
		printf("Current FSB multiplier code: %.1f\n",
			fid_codes[vidctl.bits.FID]);

	/* Now dump the status */
	if (read_msr(cpu->number, MSR_FID_VID_STATUS, &fidvidstatus.val) != 1) {
		printf("Something went wrong reading MSR_FID_VID_STATUS\n");
		return;
	}


	printf("Voltage ID codes: Maximum=%0.3fV Startup=%0.3fV Currently=%0.3fV\n",
		mobile_vid_table[fidvidstatus.bits.MVID],
		mobile_vid_table[fidvidstatus.bits.SVID],
		mobile_vid_table[fidvidstatus.bits.CVID]);

	printf("Frequency ID codes: Maximum=%.1fx Startup=%.1fx Currently=%.1fx\n",
		fid_codes[fidvidstatus.bits.MFID],
		fid_codes[fidvidstatus.bits.SFID],
		fid_codes[fidvidstatus.bits.CFID]);

//	printf("Voltage ID codes: Maximum=0x%x Startup=0x%x Currently=0x%x\n",
//		fidvidstatus.MVID, fidvidstatus.SVID, fidvidstatus.CVID);
//	printf("Frequency ID codes: Maximum=0x%x Startup=0x%x Currently=0x%x\n",
//		fidvidstatus.MFID, fidvidstatus.SFID, fidvidstatus.CFID);

	if (show_bios) {
		printf("Decoding BIOS PST tables (maxfid=%x, startvid=%x)\n",
			fidvidstatus.bits.MFID, fidvidstatus.bits.SVID);
		dump_PSB(cpu, fidvidstatus.bits.MFID, fidvidstatus.bits.SVID);
	}
}

static double k8_vid_table[32] = {
	1.550, 1.525, 1.500, 1.475, 1.450, 1.425, 1.400, 1.375,
	1.350, 1.325, 1.300, 1.275, 1.250, 1.225, 1.200, 1.175,
	1.150, 1.125, 1.100, 1.075, 1.050, 1.025, 1.000, 0.975,
	0.950, 0.925, 0.900, 0.875, 0.850, 0.825, 0.800, 0.000,
};

static int k8_fid_codes[43] = {
	4, -1, 5, -1, 6, -1, 7, -1, 8, -1, 9, -1, 10, -1, 11, -1,
	12, -1, 13, -1, 14, -1, 15, -1, 16, -1, 17, -1, 18, -1, 19, -1,
	20, -1, 21, -1, 22, -1, 23, -1, 24, -1, -25
};

static void k8_decode_fidvid(struct cpudata *cpu)
{
	union k8_msr_fidvidstatus fidvidstatus;

	dumpmsr(cpu->number, MSR_FID_VID_CTL, 64);
	dumpmsr(cpu->number, MSR_FID_VID_STATUS, 64);
	printf("\n");

	if (read_msr(cpu->number, MSR_FID_VID_STATUS, &fidvidstatus.val) != 1) {
		printf("Something went wrong reading MSR_FID_VID_STATUS\n");
		return;
	}

	printf("Voltage ID codes: Maximum=%0.3fV Startup=%0.3fV Currently=%0.3fV\n",
		k8_vid_table[fidvidstatus.bits.maxvid],
		k8_vid_table[fidvidstatus.bits.svid],
		k8_vid_table[fidvidstatus.bits.cvid]);

	printf("Frequency ID codes: Maximum=%dx Startup=%dx Currently=%dx\n",
		k8_fid_codes[fidvidstatus.bits.mfid],
		k8_fid_codes[fidvidstatus.bits.sfid],
		k8_fid_codes[fidvidstatus.bits.cfid]);
}

static int get_did(int family, union msr_pstate pstate)
{
	int t;

	if (family == 0x14)
		t = ((pstate.val >> 2) & 0x1f) + 4 + ((pstate.val & 0x3));
	else if (family == 0x12)
		t = pstate.val & 0xf;
	else
		t = pstate.bits.did;

	return t;
}

static int get_main_pll_fid(void)
{
	struct pci_filter filter_nb_misc = { -1, -1, -1, -1, 0x1022, 0x1703};
	struct pci_access *pacc;
	struct pci_dev *z = NULL;
	u8 val;

	pacc = pci_alloc();
	pci_init(pacc);
	pci_scan_bus(pacc);

	for (z=pacc->devices; z; z=z->next) {
		if (pci_filter_match(&filter_nb_misc, z))
			break;
	}

	val = 0;
	if (z) {
		val = pci_read_byte(z, 0xd4);
		val &= 0x3f;
	}

	pci_cleanup(pacc);

	return val;
}

static int get_cof(int family, union msr_pstate pstate)
{
	int t;
	int fid, did;

	did = get_did(family, pstate);

	t = 0x10;
	fid = pstate.bits.fid;
	if ((family == 0x10) || (family == 0x15))
		goto out;

	if (family == 0x11) {
		t = 0x8;
		goto out;
	}

	if (family == 0x12) {
		int f, d;
		t = 0x10;
		fid = (pstate.val >> 4) & 0x1f;

		switch (did) {
		case 0:
			goto out;
		case 2:
			did = 1; goto out;
		case 4:
			did = 2; goto out;
		case 6:
			did = 3; goto out;
		case 8: 
			did = 4; goto out;
		case 1:
			f = 2; d = 3; break;
		case 3:
			f = 1; d = 3; break;
		case 5:
			f = 1; d = 6; break;
		case 7:
			f = 1; d = 12; break;
		default:
			printf("Invalid divisor ID: %d\n", did);
			return 0;
		}
		return (100 * (fid + t) * f / d);
	}
	if (family == 0x14) {
		fid = get_main_pll_fid(); //from PCI
		return (((fid + 0x10) *100) * 4 / did);
	}

 out:
	return ((100 * (fid + t)) >> did);
 }

static int get_num_boost_states(void)
{
	struct pci_filter filter_nb_link = { -1, -1, -1, -1, 0x1022, 0};
	int dev_ids[4] = {0x1204, 0x1404, 0x1604, 0x1704};
	struct pci_access *pacc;
	struct pci_dev *z = NULL;
	u8 val;
	int i;

	pacc = pci_alloc();
	pci_init(pacc);
	pci_scan_bus(pacc);

	for (i=0; i<ARRAY_SIZE(dev_ids); i++) {
		filter_nb_link.device = dev_ids[i];
		for (z=pacc->devices; z; z=z->next) {
			if (pci_filter_match(&filter_nb_link, z))
				goto match;
		}
	}

 match:
	val = 0;
	if (z) {
		val = pci_read_byte(z, 0x15c);
		if (val & 3)
			printf("Boosting enabled\n");
		else
			printf("Boosting disabled\n");
		val = (val >> 2) & 7;
		printf("Number of boost states: %u\n", val);
	}

	pci_cleanup(pacc);
	return val;
}

static void decode_pstates(struct cpudata *cpu, int has_cpb)
{
	int i, psmax, pscur, fam;
	union msr_pstate pstate;
	unsigned long long val;
	int boost_states = 0;

	fam = family(cpu);
	if (fam < 0x10)
		return;

	if (has_cpb)
		boost_states = get_num_boost_states();

	if (read_msr(cpu->number, MSR_PSTATE_LIMIT, &val) != 1) {
		printf("Something went wrong reading MSR_PSTATE_CUR_LIMIT\n");
		return;
	}
	psmax = (val >> 4) & 0x7;

	if (read_msr(cpu->number, MSR_PSTATE_STATUS, &val) != 1) {
		printf("Something went wrong reading MSR_PSTATE_STATUS\n");
		return;
	}
	pscur = val & 0x7;

	pscur += boost_states;
	psmax += boost_states;
	for (i=0; i<=psmax; i++) {
		if (read_msr(cpu->number, MSR_PSTATE + i, &pstate.val) != 1) {
			printf("Something went wrong reading MSR_PSTATE_%d\n",
				i);
			return;
		}
		if (i < boost_states) {
			printf("Pstate-Pb%d: %dMHz (boost state)\n",
			       i, get_cof(fam, pstate));
		} else if (pstate.bits.en) {
			/* show information only if pstate is enabled */
			printf("Pstate-P%d:  %dMHz%s\n",
			       i - boost_states, get_cof(fam, pstate),
			       (i == pscur) ? " (current)"  : "");
		}
	}
	printf("\n");
}

void decode_powernow(struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx;
	int can_scale_vid=0, can_scale_fid=0;
	int has_cpb = 0;

	if (cpu->maxei < 0x80000007)
		return;

	cpuid(cpu->number, 0x80000007, &eax, &ebx, &ecx, &edx);
	printf("PowerNOW! Technology information\n");
	printf("Available features:");

	if (edx & (1<<0))
		printf("\n\tTemperature sensing diode present.");
	if (edx & (1<<1)) {
		printf("\n\tFrequency ID control");
		can_scale_fid=1;
	}
	if (edx & (1<<2)) {
		printf("\n\tVoltage ID control");
		can_scale_vid=1;
	}
	if (edx & (1<<3))
		printf("\n\tThermal Trip");
	if (edx & (1<<4))
		printf("\n\tThermal Monitoring");
	if (edx & (1<<5))
		printf("\n\tSoftware Thermal Control");
	if (edx & (1<<6))
		printf("\n\t100MHz multiplier control");
	if (edx & (1<<7)) {
		printf("\n\tHardware P-state control");
		can_scale_fid = can_scale_vid = 1;
	}
	if (edx & (1<<8))
		printf("\n\tinvariant TSC");
	if (edx & (1<<9)) {
		printf("\n\tCore Performance Boost");
		has_cpb = 1;
	}
	if (edx & (1<<10))
		printf("\n\tread-only Effective Frequency Interface");
	if (!(edx & 0x1ff))
		printf(" None");

	cpuid(cpu->number, 6, &eax, &ebx, &ecx, &edx);
	if (ecx & 1)
		printf("\n\tEffective Frequency Interface");

	printf("\n\n");

	if (can_scale_fid==0 && can_scale_vid==0)
		return;

	if (!user_is_root)
		return;

	if (family(cpu) < 0xf)
		decode_fidvid(cpu);
	else if (family(cpu) == 0xf)
		k8_decode_fidvid(cpu);
	else if (family(cpu) >= 0x10)
		decode_pstates(cpu, has_cpb);
}
