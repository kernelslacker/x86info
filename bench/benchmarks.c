#include "bench.h"

void show_benchmarks (void)
{
	time_cpuid();
	time_int80h();
}
