/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Generic routines for retrieving cpuid registers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <x86info.h>

/* returns zero on success */
int native_cpuid(unsigned int cpunr, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx)
{
	unsigned int a = 0, b = 0, c = 0, d = 0;

	c = idx >> 32;

	if (bind_cpu(cpunr))
		return -1;

	asm("cpuid"
		: "=a" (a),
		  "=b" (b),
		  "+c" (c),
		  "=d" (d)
		: "0" ((unsigned int)idx));

	if (eax!=NULL)
		*eax = a;
	if (ebx!=NULL)
		*ebx = b;
	if (ecx!=NULL)
		*ecx = c;
	if (edx!=NULL)
		*edx = d;

	return 0;
}

/* Some CPUID calls want 'count' to be placed in ecx */
void cpuid_count(unsigned int CPU_number, unsigned int op, int count,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx)
{
	cpuid(CPU_number, op | ((unsigned long long)(unsigned int)count << 32),
	      eax, ebx, ecx, edx);
}

unsigned int cpuid_ebx(unsigned int CPU_number, unsigned int op)
{
	unsigned int eax, ebx, ecx, edx;

	cpuid(CPU_number, op, &eax, &ebx, &ecx, &edx);

	return ebx;
}


/* Map the CPUID leaf 0 vendor string registers to a vendor. */
enum vendor vendor_from_cpuid(unsigned int ebx, unsigned int edx)
{
	switch (ebx) {
	case 0x756e6547:
		/* "GenuineTMx86" shares "Genu" with "GenuineIntel"; EDX is "ineT" */
		if (edx == 0x54656e69)
			return VENDOR_TRANSMETA;
		return VENDOR_INTEL;
	case 0x6f677948:	/* "HygonGenuine" */
	case 0x68747541:
		return VENDOR_AMD;
	case 0x69727943:
		return VENDOR_CYRIX;
	case 0x746e6543:
		return VENDOR_CENTAUR;
	case 0x646f6547:
		return VENDOR_NATSEMI;
	case 0x52697365:
	case 0x65736952:
		return VENDOR_RISE;
	case 0x20536953:
		return VENDOR_SIS;
	default:
		return VENDOR_UNKNOWN;
	}
}

/* Largest plausible leaf in the range starting at base, or 0 if bogus. */
unsigned int sane_max_leaf(unsigned int max, unsigned int base)
{
	if ((max & 0xffff0000) != base)
		return 0;
	if (max - base > 0xff)
		return base + 0xff;
	return max;
}

void dump_raw_cpuid(int cpunum, unsigned int begin, unsigned int end)
{
	unsigned int i;
	unsigned int eax, ebx, ecx, edx;

	/* Dump all the CPUID results in raw hex */
	if (end < begin)
		return;

	for (i = begin; ; i++) {
		cpuid(cpunum, i, &eax, &ebx, &ecx, &edx);
		printf("eax in: 0x%08x, eax = %08x ebx = %08x ecx = %08x edx = %08x\n", i, eax, ebx, ecx, edx);
		if (i == end)
			break;
	}
	printf("\n");
}
