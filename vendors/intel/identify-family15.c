/*
 *  (C) 2008 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel family 15 specific identification.
 */

#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include "Intel.h"

static char p4_423_datasheet[]="http://developer.intel.com/design/pentium4/datashts/24919805.pdf";
static char p4_478_datasheet[]="http://developer.intel.com/design/pentium4/datashts/24988703.pdf\n\thttp://developer.intel.com/design/pentium4/datashts/29864304.pdf";
static char p4_errata[]="http://developer.intel.com/design/pentium4/specupdt/249199.htm";

static char *intel_nameptr;
#define add_to_cpuname(x)   intel_nameptr += snprintf(intel_nameptr, sizeof(x), "%s", x)

void Identify_Intel_family15(struct cpudata *cpu)
{
	intel_nameptr = cpu->name;

	switch (model(cpu)) {
	case 0x0:	/* Family 15 */
		cpu->connector = CONN_SOCKET_423;
		cpu->datasheet_url = strdup(p4_423_datasheet);
		cpu->errata_url = strdup(p4_errata);
		add_to_cpuname("Pentium 4");
		switch (cpu->stepping) {
		case 7:
			//SL4QD SL4SF = 1.3GHz
			//SL4SG SL4SC = 1.4GHz
			//SL4SH SL4TY = 1.5GHz
			add_to_cpuname(" [B2]");
			break;
		case 0xA:
			//SL5FW SL5GC 1.3GHz
			//SL4WS SL4X2 SL59U SL5N7 1.4GHz
			//SL4WT SL4X3 SL59V SL5NB 1.5GHz
			//SL4WU SL4X4 SL5US SL5UW 1.6GHz
			//SL57W SL57V SL59X SL5N9 1.7GHz
			//SL4WV SL4X5 SL5UT SL5UV 1.8GHz
			add_to_cpuname(" [C1]");
			break;
		}
		break;

	case 0x1:
		cpu->connector = CONN_SOCKET_423;
		add_to_cpuname("Pentium 4 (Willamette)");
		cpu->datasheet_url = strdup(p4_423_datasheet);
		cpu->errata_url = strdup(p4_errata);
		switch (cpu->stepping) {
		case 1:
			//400FSB 256K L2
			//SSpec MHz L3
			//SL5G8 1.6 1M
			//SL5S4 1.6 1M
			//SL5FZ 1.4 512K
			//SL5RZ 1.4 512K
			//SL5G2 1.5 512K
			//SL5RW 1.5 512K
			add_to_cpuname(" [C0]");
			break;
		case 2:
			//SL5TG SL5UE 1.4GHz
			//SL5SX SL5TJ SL5UF SL62Y SL5TN 1.5GHz
			//SL5VL SL5UL SL5VH SL5UJ 1.6GHz
			//SL5SY SL5TK SL5UG SL62Z 1.7GHz
			//SL5VM SL5VM SL5VJ SL5UK 1.8GHz
			//SL5VN SL5WH SL5VK SL5WG 1.9GHz
			//SL5SZ SL5TQ SL5TL 2GHz
			add_to_cpuname(" [D0]");
			break;
		case 3:
			//SL6BC SL679 1.6GHz
			//SL6BD SL67A 1.7GHz
			//SL6BE SL78B 1.8GHz
			//SL6BF SL67C 1.9GHz
			add_to_cpuname(" [E0]");
			break;
		}
		break;
	case 0x2:
		cpu->connector = CONN_SOCKET_478;
		cpu->datasheet_url = strdup(p4_478_datasheet);
		cpu->errata_url = strdup(p4_errata);
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
			//512K L2
			//          L3
			//SL6GZ 1.5 1M
			//SL6KB 1.5 1M
			//SL6H2 1.9 2M
			//SL6KC 1.9 2M
			//SL66Z 2.0 1M
			//SL6KD 2.0 1M
			add_to_cpuname(" [A0]");
			break;
		case 4:
			//SL66B 1.6GHz
			//SL63X SL62P SL6BQ 1.8GHz
			//SL6BR SL5YR 2GHz
			//SL5YS SL6BS SL5ZU 2.2GHz
			//SL6B3 SL67Y 2.26GHz (533MHz FSB)
			//SL6BT SL65R SL67R 2.4GHz (400MHz FSB)
			//SL6B4 SL67Z 2.4GHz (533MHz FSB)
			//SL6B5 SL6B2 2.53GHz (533MHz FSB)
			add_to_cpuname(" [B0]");
			break;
		case 5:
			/*[M0] */
			//SL6Z3 2.4GHz (800FSB)
			//SL6Z5 2.8GHz (800FSB)
			/* P4 Extreme edition.*/
			//SL7AA 3.2GHz (800FSB) 2MB L3 cache
			//SL7CH 3.4GHz (800FSB) 2MB L3 cache

			/* 400FSB B1 512K L2 */
			//SL6YJ 2.0 1M L3
			//SL6Z6 2.0 1M L3
			//SL6Z2 2.5 1M L3
			//SL6Z7 2.5 1M L3
			//SL6YL 2.8 2M L3
			//SL6Z8 2.8 2M L3
			add_to_cpuname(" [M0]");
			break;
		case 6:
			//400FSB 512K L2
			//SL79V 3.0 4M L3
			//SL79Z 2.7 2M L3
			//SL7A5 2.2 2M L3
			add_to_cpuname(" [C0]");
			break;
		case 7:
			//SL6HL SL6K6 2.8GHz (533MHz FSB)
			//SL6LA SL6S6 1.8GHz
			//SL6GQ SL6S7 SL6E7 2GHz
			//SL6GR SL6SB SL6EB 2.2GHz
			//SL6DU SL6RY SL6EE 2.26GHz (533FSB)
			//SL6EF SL6DV SL6S9 SL6RZ SL6E9 2.4GHz (533FSB)
			//SL6SA 2.5GHz (400FSB)
			//SL6EG SL6S2 SL6DW 2.53GHz (533FSB)
			//SL6SB 2.6GHz (400FSB)
			//SL6S3 SL6SK 2.66GHz (533FSB)
			//SL6S4 SL6SL 2.8GHz (533FSB)
			//SL6S5 SL6K7 SL6SM SL6JJ 3.06GHz (533FSB)
			add_to_cpuname(" [C1]");
			break;
		case 9:
			//SL6QL 1.8GHz
			//SL6QM SL6PK 2.0GHz
			//SL6QN SL6PL 2.2GHz
			//SL6QR SL6PB 2.26GHz (533FSB)
			//SL6QP SL6PM 2.4GHz
			//SL6QB SL6PC 2.4GHz (533FSB)
			//SL6WF SL6WR 2.4GHz (800FSB)
			//SL6QQ 2.5GHz
			//SL6Q9 SL6PD 2.53GHz (533FSB)
			//SL6QR 2.6GHz
			//SL6WH SL6WS 2.6GHz (800FSB)
			//SL6QA SL6PE 2.66GHz (533FSB)
			//SL6QB SL6PF 2.8GHz (533FSB)
			//SL6WJ SL6WT 2.8GHz (800FSB)
			//SL6WU SL6WK 3GHz (800FSB)
			//SL6QC SL6PG 3.06GHz (533FSB)
			//SL6WG SL6WE 3.2GHz (800FSB)
			//SL793 3.4GHz (800FSB)
			add_to_cpuname(" [D1]");
			break;
		}
		break;
	case 0x3:
		switch (cpu->stepping) {
		case 3:
			/*
			sspec speed   fsb l2 90nm
			SL7D7 2.26GHz 533 512K
			SL7FY 2.4GHz  800 1M
			SL7E8 2.4GHz  533 1M
			SL7E9 2.66GHz 533 1M
			SL7D8 2.8GHz  533 1M
			SL79K 2.8GHz  800 1M
			SL79L 3.0GHz  800 1M
			SL79M 3.2GHz  800 1M
			SL7B8 3.2GHz  800 1M
			SL7B9 3.4GHz  800 1M
			SL7AJ 3.4GHz  800 1M

			process = "0.09u";
			125 million transistors
			112mm2 die size
			pipeline_stages=31
			*/
			add_to_cpuname("Pentium 4 (Prescott) [C0]");
			break;
		case 4:
			/*
			1M L2 90nm
			sspec speed   fsb
			SL7E2 2.8GHz  533
			SL7E3 2.8GHz  800
			SL7KA 2.8GHz  800
			SL7K9 2.8GHz  533
			SL7E4 3.0GHz  800
			SL7KB 3.0GHz  800
			SL7L4 3.0GHz  800
			SL7L5 3.2GHz  800
			SL7E5 3.2GHz  800
			SL7KC 3.2GHz  800
			SL7E6 3.4GHz  800
			SL7KD 3.4GHz  800
			SL7YP 2.4GHz  533
			SL7YU 2.66GHz 533
			SL7J4 2.8GHz  533
			SL7J5 2.8GHz  800
			SL7KH 2.8GHz  533
			SL7KJ 2.8GHz  800
			SL7YV 2.93GHz 533
			SL7J6 3.0GHz  800
			SL7KK 3.0GHz  800
			SL7J7 3.2GHz  800
			SL7KL 3.2GHz  800
			SL7LA 3.2GHz  800
			SL7J8 3.4GHz  800
			SL7KM 3.4GHz  800
			SL7L8 3.4GHz  800
			SL7J9 3.6GHz  800
			SL7KN 3.6GHz  800
			SL7L9 3.6GHz  800
			 */
			add_to_cpuname("Pentium 4 (Prescott) [D0]");
			break;
		}
		break;

	case 0x4:
		add_to_cpuname("Pentium 4 ");
		switch (cpu->stepping) {
		case 1:
			/*
			 1M L2 90nm
			SL88F 2.4GHz  533
			SL8B3 2.66GHz 533
			SL88G 2.8GHz  533
			SL88H 2.8GHz  800
			SL7PL 2.8GHz  800
			SL7PK 2.8GHz  533
			SL7PM 3GHz    800
			SL88J 3GHz    800
			SL7PN 3.2GHz  800
			SL88K 3.2GHz  800
			SL88L 3.4GHz  800
			SL7PP 3.4GHz  800
			SL7PT 2.66GHz 533
			SL82P 2.8GHz  800
			SL7PR 2.8GHz  800
			SL8HX 2.8GHz  800
			SL85U 2.66GHz 533
			SL8J8 2.66GHz 533
			SL85V 2.93GHz 533
			SL8J9 2.93GHz 533
			SL87L 3.06GHz 533
			SL8JA 3.06GHz 533
			SL82X 3.0GHz  800
			SL7PU 3.0GHz  800
			SL8HZ 3.0GHz  800
			SL7PW 3.2GHz  800
			SL7PX 3.2GHz  800
			SL82Z 3.2GHz  800
			SL8J2 3.2GHz  800
			SL7PY 3.4GHz  800
			SL7PZ 3.4GHz  800
			SL833 3.4GHz  800
			SL7ZW 3.4GHz  800
			SL8J5 3.4GHz  800
			SL84X 3.6GHz  800
			SL7Q2 3.6GHz  800
			SL7NZ 3.6GHz  800
			SL8J6 3.6GHz  800
			SL82U 3.8GHz  800
			SL84Y 3.8GHz  800
			SL7P2 3.8GHz  800
			SL8J7 3.8GHz  800
			 */
			/*
			 8MB L3 [C-0]
			 SL8EY 3.3GHz 667
			 SL8EW 3GHz   667
			 4MB L3
			 SL8ED 2.8GHz 667
			 */
			add_to_cpuname("(Prescott) [E0]");
			break;
		case 3:
			/*
			 2M L2 90nm
			 SL7Z9 3.0GHz  800
			 SL7Z8 3.2GHz  800
			 SL8Z7 3.4GHz  800
			 SL7Z5 3.6GHz  800
			 SL7Z4 3.73GHz 800
			 SL7Z3 3.8GHz  800
			 */
			add_to_cpuname("(Prescott) [N0]");
			break;
		case 4:
			/*
			   1Mx2 L2	800MHz FSB
			   SL88T	2.8GHz
			   SL88S	3GHz
			   SL88R	3.2GHz
			   SL8FK	3.2GHz
			 */
			add_to_cpuname("Extreme Edition [A0]");
			break;
		default:
			add_to_cpuname("D (Foster)");
			break;
		}
		break;

	case 0x5:
		cpu->connector = CONN_SOCKET_603;
//		cpu->datasheet_url = strdup(p4_478_datasheet);
//		cpu->errata_url = strdup(p4_errata);
		add_to_cpuname("Pentium 4 Xeon (Foster)");
		break;
	default:
		add_to_cpuname("Unknown CPU");
		break;
	}
}
