#include <time.h>
#include <sys/time.h>
#include <asm/unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "../x86info.h"
#include "bench.h"

void show_benchmarks(void)
{
	int ret, tmp=0;

	if (show_bench != 1)
		return;

	TIME(asm volatile("int $0x80" :"=a" (ret) :"0" (__NR_getppid)), "int 0x80");
	TIME(asm volatile("cpuid": : :"ax", "dx", "cx", "bx"), "cpuid");

	TIME(asm volatile("addl $1,0(%esp)"), "addl");
	TIME(asm volatile("lock ; addl $1,0(%esp)"), "locked add");

	TIME(asm volatile("bswap %0" : "=r" (tmp) : "0" (tmp)), "bswap");
}
