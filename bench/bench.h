static inline unsigned long long int rdtsc(void)
{
	unsigned int low, high;
	unsigned long tsc;

	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
	tsc = ((unsigned long long) high << 32) | low;
	return tsc;
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
