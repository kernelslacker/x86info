/*
 *  $Id: powernow.h,v 1.1 2002/11/19 20:03:28 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific information
 *
 */

#include <sys/types.h>

#define MSR_FID_VID_CTL		0xc0010041
#define MSR_FID_VID_STATUS	0xc0010042

struct msr_vidctl {
	unsigned long long val;
	__u16	reserved5:12;	// 63:52
	__u32	SGTC:20;		// 32:51
	__u16	reserved4:11;	// 31-21
	__u8	FIDCHGRATIO:1;	// 20
	__u8	reserved3:2;	// 19:18
	__u8	VIDC:1;			// 17
	__u8	FIDC:1;			// 16
	__u8	reserved2:3;	// 15:13
	__u8	VID:5;			// 12:8
	__u8	reserved1:3;	// 7:5
	__u8	FID:5;			// 4:0
};

struct msr_fidvidstatus {
	unsigned long long val;
	__u16	reserved6:11;	// 63:53
	__u8	MVID:5;			// 52:48
	__u8	reserved5:3;	// 47:45
	__u8	SVID:5;			// 44:40
	__u8	reserved4:3;	// 39:37
	__u8	CVID:5;			// 36:32
	__u16	reserved3:11;	// 31:21
	__u8	MFID:5;			// 20:16
	__u8	reserved2:3;	// 15:13
	__u8	SFID:5;			// 12:8
	__u8	reserved1:3;	// 7:5
	__u8	CFID:5;			// 4:0
};

