/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Machine check MSR numbers and MCG_CAP bits.
 */

#ifndef X86INFO_MACHINE_CHECK_H
#define X86INFO_MACHINE_CHECK_H

#define MCG_CAP		0x0179
#define MCG_STATUS	0x017a
#define MCG_CTL		0x017b
#define MCG_EXT		0x0180

#define MCG_CTL_PBIT	8
#define MCG_EXT_PBIT	9

#define MC_CTL		0x0400
#define MC_STATUS	0x0401
#define MC_ADDR		0x0402
#define MC_MISC		0x0403

#endif
