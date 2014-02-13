#include <time.h>
#include <sys/time.h>
#include <asm/unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <x86info.h>

void time_int80h()
{
	int i, ret;
	unsigned long start, end;

	if (show_bench != 1)
		return;

	start = rdtsc();
	for (i = 0; i < 1000000; i++) {
		asm volatile("call 0xfffff000"
			:"=a" (ret)
			:"0" (__NR_getppid));
	}
	end = rdtsc();
	printf("%f cycles\n", (end - start) / 1000000.0);
}

