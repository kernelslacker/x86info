/*
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Decode and print the name of the connector the CPU plugs into.
 */

#include <stdio.h>
#include "x86info.h"

static struct {
	enum connector id;
	const char *name;
} connector_strings[] = {
	/* generic sockets */
	{CONN_SOCKET_3, "Socket 3 (PGA socket)"},
	{CONN_SOCKET_4, "Socket 4 (273 pin PGA)"},
	{CONN_SOCKET_5, "Socket 5 (296 pin PGA socket)"},
	{CONN_SOCKET_7, "Socket 7 (321 pin PGA socket)"},
	{CONN_SOCKET_370, "Socket 370  (370 Pin PGA)"},
	{CONN_SOCKET_370_FCPGA, "Socket 370 (FC-PGA) or (PPGA)"},
	/* AMD specific sockets */
	{CONN_SOCKET_5_7, "Socket 5/7 (296 Pin PGA)"},
	{CONN_SUPER_SOCKET_7, "Super Socket 7 (321 Pin PGA)"},
	{CONN_SLOT_A, "Slot A (242 Contact Cartridge)"},
	{CONN_SOCKET_A, "Socket A (462 Pin PGA)"},
	{CONN_SOCKET_A_OR_SLOT_A, "Socket A or Slot A"},
	{CONN_SOCKET_754, "Socket 754"},
	{CONN_SOCKET_939, "Socket 939"},
	{CONN_SOCKET_940, "Socket 940"},
	{CONN_SOCKET_S1G1, "Socket S1g1"},
	{CONN_SOCKET_S1G2, "Socket S1g2"},
	{CONN_SOCKET_S1G3, "Socket S1g3/S1g4"},
	{CONN_SOCKET_F, "Socket F (1207)"},
	{CONN_SOCKET_AM2, "Socket AM2"},
	{CONN_SOCKET_F_R2, "Socket Fr2 (1207)"},
	{CONN_SOCKET_AM3, "Socket AM2r2/AM3"},
	{CONN_SOCKET_G34, "Socket G34"},
	{CONN_SOCKET_ASB2, "Socket ASB2"},
	{CONN_SOCKET_C32, "Socket C32"},
	{CONN_SOCKET_FP1, "Socket FP1"},
	{CONN_SOCKET_FS1, "Socket FS1"},
	{CONN_SOCKET_FM1, "Socket FM1"},
	{CONN_SOCKET_FT1, "Socket FT1"},
	{CONN_SOCKET_FP2, "Socket FP2"},
	{CONN_SOCKET_FS1r2, "Socket FS1r2"},
	{CONN_SOCKET_FM2, "Socket FM2"},
	/* Intel specific sockets */
	{CONN_SOCKET_57B, "Socket 5/7 (320 Pin PGA)"},
	{CONN_MOBILE_7, "Mobile Module (320 Lead TCP)"},
	{CONN_SOCKET_8, "Socket 8 (387 pin Dual Cavity PGA)"},
	{CONN_SLOT_1, "Slot 1 (242 Contact Cartridge)"},
	{CONN_SLOT_2, "Slot 2 (SEC Cartridge)"},
	{CONN_SOCKET_423, "Socket423 (PGA423 Socket)"},
	{CONN_SOCKET_478, "Socket478 (PGA478 Socket)"},
	{CONN_SOCKET_603, "Socket603 (PGA603 Socket)"},
	{CONN_MMC, "Mobile Module Connector (BGA)"},
	{CONN_MMC2, "Mobile Module Connector (MMC-2)"},
	{CONN_MICROFCBGA, "Micro-FCBGA"},
	{CONN_LGA775, "LGA775"},
	/* Transmeta specific sockets */
	{CONN_BGA474, "474 Pin BGA"},
	/* natsemi specific (Geode) */
	{CONN_BGA, "BGA"},	/*  */
};

static const char * get_connector_name(enum connector id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(connector_strings); i++)
		if (id == connector_strings[i].id)
			return connector_strings[i].name;
	return NULL;
}

void decode_connector(enum connector type)
{
	const char *s;
	if (type == CONN_UNKNOWN)
		return;

	s = get_connector_name(type);
	if (!s)
		s = "unknown";
	printf("Connector type: %s\n\n", s);
}
