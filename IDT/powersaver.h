/*
 *  $Id: powersaver.h,v 1.1 2003/01/27 17:33:16 davej Exp $
 *  This file is part of x86info.
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  AMD-specific information
 *
 */

#include <sys/types.h>

#define MSR_POWERSAVER	0x110a

union msr_powersaver {
	struct {
		unsigned RevisionID:4,	// 3:0
		RevisionKey:4,			// 7:4
		EnableSoftBusRatio:1,	// 8
		EnableSoftVID:1,		// 9
		Reserved:4,				// 10:13
		SoftBusRatio4:1,		// 14
		VRMRev:1,				// 15
		SoftBusRatio:4,			// 16:19
		SoftVID:5,				// 20:24
		Reserved2:7,			// 25:31
		MaxMHzBR:4,				// 32:35
		MaximumVID:5,			// 36:40
		MaxMHzFSB:2,			// 41:42
		MaxMHzBR4:1,			// 43
		Reserved3:4,			// 44:47
		MinMHzBR:4,				// 48:51
		MinimumVID:5,			// 52:56
		MinMHzFSB:2,			// 57:58
		MinMHzBR4:1,			// 59
		Reserved4:4;			// 60:63
	} bits;
	unsigned long long val;
};

