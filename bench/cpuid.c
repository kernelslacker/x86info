#include <time.h>
#include <sys/time.h>
#include <asm/unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "../x86info.h"
#include "bench.h"

static inline void __cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
	__asm__("cpuid"
	: "=a" (*eax),"=b" (*ebx),"=c" (*ecx),"=d" (*edx)
	: "0" (op));
}

void time_cpuid()
{
	int i;
	unsigned long start, end;
	int eax, ebx, ecx, edx;

	if (show_bench != 1)
		return;

	start = rdtsc();
	for (i = 0; i < 1000000; i++) {
		__cpuid (0, &eax, &ebx, &ecx, &edx);
	}
	end = rdtsc();
	printf("cpuid took %d cycles\n", (end - start) / 1000000);
}

