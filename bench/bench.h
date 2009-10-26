static inline unsigned long long int rdtsc(void)
{
	unsigned int low, high;

	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
	return ((unsigned long long int)high << 32) | low;
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
