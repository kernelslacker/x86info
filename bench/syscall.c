#include <time.h>
#include <sys/time.h>
#include <asm/unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#define rdtsc() ({ unsigned long a,d; asm volatile("rdtsc":"=a" (a), "=d" (d)); a; })

int main()
{
	int i, ret;
	unsigned long start, end;

	start = rdtsc();
	for (i = 0; i < 1000000; i++) {
		asm volatile("call 0xfffff000"
			:"=a" (ret)
			:"0" (__NR_getppid));
	}
	end = rdtsc();
	printf("%f cycles\n", (end - start) / 1000000.0);

	start = rdtsc();
	for (i = 0; i < 1000000; i++) {
		asm volatile("int $0x80"
			:"=a" (ret)
			:"0" (__NR_getppid));
	}
	end = rdtsc();
	printf("%f cycles\n", (end - start) / 1000000.0);
}


