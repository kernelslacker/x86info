static unsigned long long rdtsc(void)
{
	unsigned long low, high;

	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
	return (high << 32) | low;
}

#define NREPS 1000

#define TIME(x,y) \
{ \
	int i;				\
	unsigned long bstart, bend;	\
	bstart = rdtsc();		\
	for (i=0; i<NREPS; i++)		\
		x;			\
	bend = rdtsc();			\
	printf(y ": %ld cycles\n", ((bend-bstart)/NREPS)); \
}
