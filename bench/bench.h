void time_int80h(void);
void time_cpuid(void);

#define rdtsc() ({ unsigned long a,d; asm volatile("rdtsc":"=a" (a), "=d" (d)); a; })

#define TIME(x,y) \
	bmin = 100000;					\
	for (i = 0; i < 1000; i++) {	\
		unsigned long bstart,bend;	\
		bstart = rdtsc();			\
		x;							\
		bend = rdtsc();				\
		bend -= bstart;				\
		if (bend < bmin)			\
			bmin = bend;			\
	}								\
	printf(y ": %ld cycles\n", bmin);

