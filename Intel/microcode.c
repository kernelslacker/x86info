/*
 *  $Id: microcode.c,v 1.3 2003/04/11 00:17:20 davej Exp $
 *  This file is part of x86info.
 *  (C) 2002 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Intel P6 microcode information.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../x86info.h"
#include "Intel.h"

void decode_microcode(struct cpudata *cpu)
{
	unsigned long long val=0;
	int ver;

	if (!user_is_root)
		return;

	if (cpu->family < 6)
		return;

	if (read_msr (cpu->number, MSR_IA32_UCODE_REV, &val)==1) {
		ver = val >>32;
		if (ver>0)
			printf ("Microcode version: 0x%016llx\n", val >>32);
		printf ("\n");
	}
}
