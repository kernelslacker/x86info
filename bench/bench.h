void time_int80h(void);
void time_cpuid(void);

#define rdtsc() ({ unsigned long a,d; asm volatile("rdtsc":"=a" (a), "=d" (d)); a; })
