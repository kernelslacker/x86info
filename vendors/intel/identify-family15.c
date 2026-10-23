/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel family 15 specific identification.
 */

#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include "intel.h"

void identify_intel_family15(struct cpudata *cpu)
{
	switch (model(cpu)) {
	case 0x0:	/* Family 15 */
		cpu->connector = CONN_SOCKET_423;
		add_to_cpuname("Pentium 4");
		switch (cpu->stepping) {
		case 7:
			add_to_cpuname(" [B2]");
			break;
		case 0xA:
			add_to_cpuname(" [C1]");
			break;
		}
		break;

	case 0x1:
		cpu->connector = CONN_SOCKET_423;
		add_to_cpuname("Pentium 4 (Willamette)");
		switch (cpu->stepping) {
		case 1:
			add_to_cpuname(" [C0]");
			break;
		case 2:
			add_to_cpuname(" [D0]");
			break;
		case 3:
			add_to_cpuname(" [E0]");
			break;
		}
		break;
	case 0x2:
		cpu->connector = CONN_SOCKET_478;
		switch (cpu->brand) {
			case 15:
				add_to_cpuname("Celeron (P4 core)");
				break;
			case 7:
			default:
				add_to_cpuname("Pentium 4 (Northwood)");
				break;
		}
		switch (cpu->stepping) {
		case 2:
			add_to_cpuname(" [A0]");
			break;
		case 4:
			add_to_cpuname(" [B0]");
			break;
		case 5:
			/*[M0] */
			/* P4 Extreme edition.*/

			/* 400FSB B1 512K L2 */
			add_to_cpuname(" [M0]");
			break;
		case 6:
			add_to_cpuname(" [C0]");
			break;
		case 7:
			add_to_cpuname(" [C1]");
			break;
		case 9:
			add_to_cpuname(" [D1]");
			break;
		}
		break;
	case 0x3:
		switch (cpu->stepping) {
		case 3:
			add_to_cpuname("Pentium 4 (Prescott) [C0]");
			break;
		case 4:
			add_to_cpuname("Pentium 4 (Prescott) [D0]");
			break;
		}
		break;

	case 0x4:
		add_to_cpuname("Pentium 4 ");
		switch (cpu->stepping) {
		case 1:
			add_to_cpuname("(Prescott) [E0]");
			break;
		case 3:
			add_to_cpuname("(Prescott) [N0]");
			break;
		case 4:
			add_to_cpuname("Extreme Edition [A0]");
			break;
		default:
			add_to_cpuname("D (Foster)");
			break;
		}
		break;

	case 0x5:
		cpu->connector = CONN_SOCKET_603;
		add_to_cpuname("Pentium 4 Xeon (Foster)");
		break;
	default:
		add_to_cpuname("Unknown CPU");
		break;
	}
}
