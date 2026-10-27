/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel family 6 specific decoding. (Pentium family)
 */

#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include "intel.h"

void identify_intel_family6pentium(struct cpudata *cpu)
{
	switch (model(cpu)) {
	case 0x0:
		add_to_cpuname("Pentium Pro A-Step");
		cpu->connector = CONN_SOCKET_8;
		break;
	case 0x1:
		add_to_cpuname("Pentium Pro");
		cpu->connector = CONN_SOCKET_8;
		switch (cpu->stepping) {
		case 1:
			add_to_cpuname(" [B0]");
			break;
		case 2:
			add_to_cpuname(" [C0]");
			break;
		case 6:
			add_to_cpuname(" [sA0]");
			break;
		case 7:
			add_to_cpuname(" [sA1]");
			break;
		case 9:
			add_to_cpuname(" [sB1]");
			break;
		}
		break;
	case 0x3:
		add_to_cpuname("Pentium II ");
		cpu->connector = CONN_SLOT_1;
		switch (cpu->stepping) {
		case 2:
			add_to_cpuname("Overdrive [tdB0]");
			break;
		case 3:
			add_to_cpuname("(Klamath) [C0]");
			break;
		case 4:
			add_to_cpuname("(Klamath) [C1]");
			break;
		}
		break;
	case 0x4:
		//Does this exist? Its not in Intels spec update.
		cpu->connector = CONN_SLOT_1;
		add_to_cpuname("Pentium II (Deschutes?)");
		break;
	case 0x5:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->cachesize_L2) {
			case 0:
				add_to_cpuname("Celeron (Covington)");
				break;

			case 256:
				add_to_cpuname("Mobile Pentium II (Dixon)");
				break;

			case 512:
				switch (cpu->stepping) {
				case 0:
					add_to_cpuname("Pentium II [dA0]");
					break;
				case 1:
					add_to_cpuname("Pentium II (Deschutes) [dA1]");
					break;
				case 2:
					add_to_cpuname("Pentium II (Deschutes) [dB0]");
					break;
				case 3:
					add_to_cpuname("Pentium II (Deschutes) [dB1]");
					break;
				default:
					add_to_cpuname("Pentium II");
					break;
				}
		}
		break;
	case 0x6:
		cpu->connector = CONN_SOCKET_370;
		if (cpu->cachesize_L2 == 128) {
			add_to_cpuname("Celeron (Mendocino)");
			break;
		}
		switch (cpu->stepping) {
		case 0:
			add_to_cpuname("Celeron-A [mA0]");
			break;
		case 5:
			add_to_cpuname("Celeron-A [mB0]");
			break;
		case 0xA:
			add_to_cpuname("Mobile Pentium II [mdA0]");
			break;
		default:
			add_to_cpuname("Celeron / Mobile Pentium II");
			break;
		}
		break;
	case 0x7:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->stepping) {
		case 2:
			add_to_cpuname("Pentium III (Katmai) [kB0]");
			break;
		case 3:
			add_to_cpuname("Pentium III (Katmai) [kC0]");
			break;
		default:
			add_to_cpuname("Pentium III/Pentium III Xeon");
			break;
		}
		decode_serial_number(cpu);
		break;
	case 0x8:
		switch (cpu->brand) {
		case 2:
			add_to_cpuname("Pentium III/Pentium III-M (Coppermine)");
			switch (cpu->stepping) {
			case 1:
				add_to_cpuname(" [cA2]");
				break;
			case 3:
				add_to_cpuname(" [cB0]");
				break;
			case 6:
				add_to_cpuname(" [cC0]");
				break;
			case 0xA:
				add_to_cpuname(" [cD0]");
				break;
			}
			break;
		case 3:
			add_to_cpuname("Pentium III Xeon");
			switch (cpu->stepping) {
			case 1:
				add_to_cpuname(" [A2]");
				break;
			case 3:
				add_to_cpuname(" [B0]");
				break;
			case 6:
				add_to_cpuname(" [C0]");
				break;
			}
			break;

		case 8:
			// cpu->connector = CONN_BGA2; - Could also be Micro-PGA2
			add_to_cpuname("Mobile Pentium III");
			break;

		default:
			cpu->connector = CONN_SOCKET_370_FCPGA;
			if (cpu->cachesize_L2 == 128) {
				add_to_cpuname("Celeron");
			} else {
				add_to_cpuname("Pentium III");
			}
			add_to_cpuname(" (Coppermine)");
			switch (cpu->stepping) {
			case 1:
				add_to_cpuname(" [cA2]");
				break;
			case 3:
				add_to_cpuname(" [cB0]");
				break;
			case 6:
				add_to_cpuname(" [cC0]");
				break;
			case 0xA:
				add_to_cpuname(" [cD0]");
				break;
			}
			break;
		}
		decode_serial_number(cpu);
		break;

	case 0x9:
		add_to_cpuname("Pentium M (Banias)");
		break;

	case 0xa:
		cpu->connector = CONN_SLOT_1;
		switch (cpu->brand) {
		case 0:
			add_to_cpuname("Pentium II (Deschutes)");
			break;
		case 1:
			add_to_cpuname("Celeron");
			break;
		case 2:
			add_to_cpuname("Pentium III");
			decode_serial_number(cpu);
			break;
		case 3:
			// FSB=100
			add_to_cpuname("Pentium III Xeon");
			decode_serial_number(cpu);
			switch (cpu->stepping) {
				case 0:
					add_to_cpuname(" [A0]");
					break;
				case 1:
					add_to_cpuname(" [A1]");
					break;
				case 4:
					add_to_cpuname(" [B0]");
					break;
			}
			break;
		case 4:
			add_to_cpuname("Pentium III (Cascades)");
			decode_serial_number(cpu);
			break;
		default:
			add_to_cpuname("Unknown CPU");
			break;
		}
		break;

	case 0xb:
		switch (cpu->brand) {
			case 1:
			case 3:
				cpu->connector = CONN_SLOT_1;
				add_to_cpuname("Celeron (Tualatin) [tA1/cA2]");
				break;
			case 6:
				cpu->connector = CONN_MICROFCBGA;
				add_to_cpuname("Pentium III-M");
				decode_serial_number(cpu);
				break;
			default:
				cpu->connector = CONN_SLOT_1;
				switch (cpu->stepping) {
				case 1:
					add_to_cpuname("Pentium III (Tualatin) [tA1/cA2]");
					break;
				case 4:
					add_to_cpuname("Pentium III [B-1]");
					break;
				default:
					add_to_cpuname("Unknown CPU");
					break;
				}
				decode_serial_number(cpu);
				break;
		}
		break;

	case 0xd:
		add_to_cpuname("Pentium M ");
		cpu->connector = CONN_MICROFCBGA;
		switch (cpu->stepping) {
			case 1:	add_to_cpuname("(Dothan) [A-1]");
					break;

			case 2:	add_to_cpuname("(Dothan) [A-2]");
					break;

			case 6:	add_to_cpuname("(Dothan) [B-1]");
					break;

			case 8:	add_to_cpuname("(Dothan) [C-0]");
				break;
		}
		break;
	/*
	 * ARGH. Intel made some Core CPUs without setting the efamily or emodel to 1.
	 */
	case 0xf:
		add_to_cpuname("Core 2 ");
		switch (cpu->stepping) {
		case 2:
			add_to_cpuname("Duo [L2]");
			break;
		case 5:
			add_to_cpuname("Duo (Conroe XE)");
			break;
		case 6:
			add_to_cpuname("Duo ");
			if (cpu->cachesize_L2 == 2048) {
				add_to_cpuname("[B2]");
				ensure_MHz(cpu);
				if (MHz_matches(cpu->MHz, 1860))
					add_to_cpuname("[SL9SA] E6300");
				if (MHz_matches(cpu->MHz, 2130))
					add_to_cpuname("[SL9S9] E6400");
			}
			break;
		case 7:
			add_to_cpuname("Quad (Kentsfield) ");
			/* The SKU is only distinguishable by frequency */
			ensure_MHz(cpu);
			if (MHz_matches(cpu->MHz, 2930))
				add_to_cpuname("[SL9UK/SLACP] [B3/G0] QX6800");
			else if (MHz_matches(cpu->MHz, 2660))
				add_to_cpuname("[SL9UL] [B3] QX6700");
			else if (MHz_matches(cpu->MHz, 2400))
				add_to_cpuname("[SL9UM] [B3] QX6600");
			break;

		case 0xa:
			add_to_cpuname("[E1]");
			break;

		case 0xb:
			if (cpu->cachesize_L2 == 2048) {
				add_to_cpuname("Duo [G0]");
			} else
				add_to_cpuname("Quad ");
			break;

		case 0xd:
			add_to_cpuname("Duo [M0]");
			break;
		}
		break;

	default:
                add_to_cpuname("Unknown model. ");
	}
}
