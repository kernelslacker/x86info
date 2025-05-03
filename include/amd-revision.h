#pragma once

/*
 *  Copyright (C) 2007 Advanced Micro Devices, Inc.
 *                     Andreas Herrmann <andreas.herrmann3@amd.com>
 *  Your use of this code is subject to the terms and conditions of the
 *  GNU general public license version 2. See "COPYING" or
 *  http://www.gnu.org/licenses/gpl2.html
 */

#include <x86info.h>

/* name ids to concatenate cpu name */
#define _OPTERON	0x0001
#define _OPTERON_DC	0x0002
#define _ATHLON64	0x0004
#define _ATHLON64_X2	0x0008
#define _ATHLON64_FX	0x0010
#define _ATHLON64_FX_DC	0x0020
#define _ATHLON64_M	0x0040
#define _SEMPRON	0x0080
#define _SEMPRON_M	0x0100
#define _ATHLON_XPM	0x0200
#define _TURION		0x0400
#define _TURION_X2	0x0800

struct id_string {
	int id;
	const char *name;
};

/* Note: For newer K8 NPT parts naming switched from "Athlon 64"
   to "Athlon" etc., but we stick to the old naming here. */
static struct id_string k8_names[] = {
	{_OPTERON, "Opteron"},
	{_OPTERON_DC, "Dual-Core Opteron"},
	{_ATHLON64, "Athlon 64"},
	{_ATHLON64_X2, "Athlon 64 X2 Dual-Core"},
	{_ATHLON64_FX, "Athlon 64 FX"},
	{_ATHLON64_FX_DC, "Athlon 64 FX Dual-Core"},
	{_ATHLON64_M, "Mobile Athlon 64"},
	{_SEMPRON, "Sempron"},
	{_SEMPRON_M, "Mobile Sempron"},
	{_ATHLON_XPM, "Mobile Athlon XP-M"},
	{_TURION, "Turion"},
	{_TURION_X2, "Turion 64 X2"},
};
get_name(k8, int, k8_names);

struct k8_rev {
	int eax;
	int socketid;
	int nameid;
	const char* rev;
};

static struct k8_rev k8_revisions[] = {
	{0x00f00, CONN_SOCKET_754, _ATHLON64, "SH-A0"},
	{0x00f01, CONN_SOCKET_754, _ATHLON64, "SH-A2"},
	{0x00f10, CONN_SOCKET_940, _ATHLON64, "SH-A0"},
	{0x00f11, CONN_SOCKET_940, _ATHLON64, "SH-A2"},
	{0x00f40, CONN_SOCKET_754, _ATHLON64, "SH-B0"},
	{0x00f50, CONN_SOCKET_940, _OPTERON, "SH-B0"},
	{0x00f51, CONN_SOCKET_940, _OPTERON, "SH-B3"},
	{0x00f48, CONN_SOCKET_754, _ATHLON64|_ATHLON64_M|_ATHLON_XPM, "SH-C0"},
	{0x00f58, CONN_SOCKET_940, _OPTERON|_ATHLON64_FX, "SH-C0"},
	{0x00f4a, CONN_SOCKET_754, _ATHLON64|_ATHLON64_M|_ATHLON_XPM, "SH-CG"},
	{0x00f7a, CONN_SOCKET_939, _ATHLON64|_ATHLON64_FX, "SH-CG"},
	{0x00f5a, CONN_SOCKET_940, _OPTERON|_ATHLON64_FX, "SH-CG"},
	{0x00fe0, CONN_SOCKET_754,
	 _ATHLON64|_ATHLON64_M|_SEMPRON|_SEMPRON_M|_ATHLON_XPM, "DH-CG"},
	{0x00fc0, CONN_SOCKET_754,
	 _ATHLON64|_ATHLON64_M|_SEMPRON|_SEMPRON_M|_ATHLON_XPM, "DH-CG"},
	{0x00ff0, CONN_SOCKET_939, _ATHLON64|_SEMPRON, "DH-CG"},
	{0x00f82, CONN_SOCKET_754,
	 _ATHLON64|_ATHLON64_M|_SEMPRON_M|_ATHLON_XPM, "CH-CG"},
	{0x00fb2, CONN_SOCKET_939, _ATHLON64, "CH-CG"},
	{0x10f40, CONN_SOCKET_754, _ATHLON64|_ATHLON64_M|_ATHLON_XPM, "SH-D0"},
	{0x10f70, CONN_SOCKET_939, _ATHLON64|_ATHLON64_FX, "SH-D0"},
	{0x10f50, CONN_SOCKET_940, _OPTERON|_ATHLON64_FX, "SH-D0"},
	{0x10ff0, CONN_SOCKET_939, _ATHLON64|_SEMPRON, "DH-D0"},
	{0x10fc0, CONN_SOCKET_754,
	 _ATHLON64|_ATHLON64_M|_SEMPRON|_SEMPRON_M|_ATHLON_XPM, "DH-D0"},
	{0x10f80, CONN_SOCKET_754,
	 _ATHLON64|_ATHLON64_M|_SEMPRON_M|_ATHLON_XPM, "CH-D0"},
	{0x10fb0, CONN_SOCKET_939, _ATHLON64, "CH-D0"},
	{0x20f10, CONN_SOCKET_940, _OPTERON_DC, "JH-E1"},
	{0x20fc0, CONN_SOCKET_754, _SEMPRON, "DH-E3"},
	{0x20ff0, CONN_SOCKET_939, _ATHLON64|_SEMPRON, "DH-E3"},
	{0x20f51, CONN_SOCKET_940, _OPTERON, "SH-E4"},
	{0x20f71, CONN_SOCKET_939, _OPTERON|_ATHLON64|_ATHLON64_FX, "SH-E4"},
	{0x20fb1, CONN_SOCKET_939, _ATHLON64_X2, "BH-E4"},
	{0x20f42, CONN_SOCKET_754, _ATHLON64_M|_TURION, "SH-E5"},
	{0x20fc2, CONN_SOCKET_754, _SEMPRON|_SEMPRON_M, "DH-E6"},
	{0x20ff2, CONN_SOCKET_939, _ATHLON64|_SEMPRON, "DH-E6"},
	{0x20f12, CONN_SOCKET_940, _OPTERON_DC, "JH-E6"},
	{0x20f32, CONN_SOCKET_939, _OPTERON_DC|_ATHLON64_X2, "JH-E6"},

	/* K8 NPT */
	{0x40f12, CONN_SOCKET_F, _OPTERON_DC, "JH-F2"},
	{0x40f13, CONN_SOCKET_F, _OPTERON_DC, "JH-F3"},
	{0x40f32, CONN_SOCKET_AM2,
	 _ATHLON64_X2|_ATHLON64_FX_DC|_OPTERON_DC, "JH-F2"},
	{0x40f33, CONN_SOCKET_AM2,
	 _ATHLON64_X2|_ATHLON64_FX_DC|_OPTERON_DC, "JH-F3"},
	{0x40f82, CONN_SOCKET_S1G1, _TURION_X2, "BH-F2"},
	{0x40fb2, CONN_SOCKET_AM2, _ATHLON64_X2, "BH-F2"},
	{0x40fc2, CONN_SOCKET_S1G1,_ATHLON64|_SEMPRON|_SEMPRON_M, "DH-F2"},
	{0x40ff2, CONN_SOCKET_AM2, _ATHLON64|_SEMPRON, "DH-F2"},
	{0x50ff2, CONN_SOCKET_AM2, _ATHLON64|_SEMPRON, "DH-F2"},
	{0x50ff3, CONN_SOCKET_AM2, _ATHLON64, "DH-F3"},
	{0x60f81, CONN_SOCKET_S1G1,_ATHLON64_X2, "BH-G1"},
	{0x60f82, CONN_SOCKET_S1G1,_ATHLON64_X2|_TURION_X2, "BH-G2"},
	{0x60fb1, CONN_SOCKET_AM2,_ATHLON64_X2, "BH-G1"},
	{0x60fb2, CONN_SOCKET_AM2,_ATHLON64_X2, "BH-G2"},
	{0x60fc2, CONN_SOCKET_S1G1,_SEMPRON_M, "DH-G2"},
	{0x60ff2, CONN_SOCKET_AM2,_SEMPRON|_ATHLON64, "DH-G2"},
	{0x70fc2, CONN_SOCKET_S1G1,_SEMPRON_M, "DH-G2"},
	{0x70ff1, CONN_SOCKET_AM2,_SEMPRON|_ATHLON64, "DH-G1"},
	{0x70ff2, CONN_SOCKET_AM2,_SEMPRON, "DH-G2"},
	{0xc0f13, CONN_SOCKET_F, _ATHLON64_FX, "JH-F3"},
};

struct fam10h_rev {
	int eax;
	const char* rev;
};

static struct fam10h_rev fam10h_revisions[] = {
	{0x0100f2a, "DR-BA"},
	{0x0100f22, "DR-B2"},
	{0x0100f23, "DR-B3"},
	{0x0100f42, "RB-C2"},
	{0x0100f43, "RB-C3"},
	{0x0100f52, "BL-C2"},
	{0x0100f53, "BL-C3"},
	{0x0100f62, "DA-C2"},
	{0x0100f63, "DA-C3"},
	{0x0100f80, "HY-D0"},
	{0x0100f81, "HY-D1"},
	{0x0100f91, "HY-D1"},
	{0x0100fa0, "PH-E0"},
};

static struct id_string fam11h_revisions[] = {
	{0x0200f31, "LG-B1"},
};
get_name(fam11h_revision, int, fam11h_revisions);

static struct id_string fam12h_revisions[] = {
	{0x0300f10, "LN1-B0"},
};
get_name(fam12h_revision, int, fam12h_revisions);

static struct id_string fam14h_revisions[] = {
	{0x00500f10, "ON-B0"},
	{0x00500f20, "ON-C0"},
};
get_name(fam14h_revision, int, fam14h_revisions);

static struct id_string fam15h_revisions[] = {

	{0x00600f12, "OR-B2"},
	{0x00600f20, "OR-C0"},
};
get_name(fam15h_revision, int, fam15h_revisions);


static struct id_string fam17h_revisions[] = {
	{0x800f11, "ZP-B1"},
};
get_name(fam17h_revision, int, fam17h_revisions);

static struct id_string fam19h_revisions[] = {
	{0xa40f41, "Ryzen 7 7735HS"},
};
get_name(fam19h_revision, int, fam19h_revisions);

static struct id_string fam1ah_revisions[] = {
	{0xb40f40, "Ryzen 9 9950X"},
};
get_name(fam1ah_revision, int, fam1ah_revisions);
