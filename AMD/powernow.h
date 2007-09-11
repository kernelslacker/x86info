/*
 *  $Id: powernow.h,v 1.4 2003/01/16 17:09:18 davej Exp $
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

#define MSR_PSTATE_LIMIT	0xc0010061
#define MSR_PSTATE_STATUS	0xc0010063
#define MSR_PSTATE		0xc0010064

union msr_vidctl {
	struct {
		unsigned FID:5,			// 4:0
		reserved1:3,	// 7:5
		VID:5,			// 12:8
		reserved2:3,	// 15:13
		FIDC:1,			// 16
		VIDC:1,			// 17
		reserved3:2,	// 19:18
		FIDCHGRATIO:1,	// 20
		reserved4:11,	// 31-21
		SGTC:20,		// 32:51
		reserved5:12;	// 63:52
	} bits;
	unsigned long long val;
};

union msr_fidvidstatus {
	struct {
		unsigned CFID:5,			// 4:0
		reserved1:3,	// 7:5
		SFID:5,			// 12:8
		reserved2:3,	// 15:13
		MFID:5,			// 20:16
		reserved3:11,	// 31:21
		CVID:5,			// 36:32
		reserved4:3,	// 39:37
		SVID:5,			// 44:40
		reserved5:3,	// 47:45
		MVID:5,			// 52:48
		reserved6:11;	// 63:53
	} bits;
	unsigned long long val;
};

union k8_msr_fidvidstatus {
	struct {
		unsigned cfid:6;
		unsigned res1:2;
		unsigned sfid:6;
		unsigned res2:2;
		unsigned mfid:6;
		unsigned res3:2;
		unsigned mrampvid:5;
		unsigned res4:2;
		unsigned fvpending:1;
		unsigned cvid:5;
		unsigned res5:3;
		unsigned svid:5;
		unsigned res6:3;
		unsigned maxvid:5;
		unsigned res7:3;
		unsigned minvid:5;
		unsigned res8:3;
	} bits;
	unsigned long long val;
};

union msr_pstate {
	struct {
		unsigned fid:6;
		unsigned did:3;
		unsigned vid:7;
		unsigned res1:6;
		unsigned nbdid:1;
		unsigned res2:2;
		unsigned nbvid:7;
		unsigned iddval:8;
		unsigned idddiv:2;
		unsigned res3:21;
		unsigned en:1;
	} bits;
	unsigned long long val;
};

extern double mobile_vid_table[32];
extern double fid_codes[32];

