#define rdtsc() ({ unsigned long a,d; asm volatile("rdtsc":"=a" (a), "=d" (d)); a; })

#define NREPS 1000

#define TIME(x,y) \
{ \
	int i,j;						\
	unsigned long bmin,bstart,bend;	\
	for (j=0; j<100; j++) {			\
		bmin = 100000;				\
		bstart = rdtsc();			\
		for (i=0; i<NREPS; i++)		\
			x;						\
		bend = rdtsc();				\
		bend -= bstart;				\
		if (bend < bmin)			\
			bmin = bend;			\
	}								\
	printf(y ": %ld cycles\n", bmin/NREPS); \
}

