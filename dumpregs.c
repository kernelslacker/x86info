/*
 *  (C) 2001,2008 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include "x86info.h"

void dumpregs(int cpunum, unsigned int begin, unsigned int end)
{
        unsigned int i;
        unsigned int eax, ebx, ecx, edx;

        /* Dump all the CPUID results in raw hex */
        for (i=begin; i<=end; i++) {
                cpuid(cpunum, i, &eax, &ebx, &ecx, &edx);
                printf("eax in: 0x%08x, eax = %08x ebx = %08x ecx = %08x edx = %08x\n", i, eax, ebx, ecx, edx);
        }
        printf("\n");
}
