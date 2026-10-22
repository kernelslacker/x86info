/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <signal.h>

#include <x86info.h>
#include <rdtsc.h>

#define MSR_MPERF	0xe7
#define MSR_APERF	0xe8

static void print_MHz(unsigned int mhz)
{
	if (mhz < 1000)
		printf("%uMHz", mhz);
	else {
		unsigned int a = (mhz / 1000);
		unsigned int b = ((mhz % 1000)/100);
		unsigned int c = (a*1000)+(b*100);
		printf("%u.%u%uGHz", a, b, (mhz - c)/10);
	}
}

void display_MHz(const struct cpudata *cpu)
{
	print_MHz(cpu->MHz);
}

/*
 * With an invariant TSC the measured rate is the TSC rate, not the core
 * clock; APERF/MPERF, when readable, give the core clock over the run.
 */
void display_MHz_line(const struct cpudata *cpu)
{
	unsigned int edx = 0;

	if (cpu->maxei >= 0x80000007)
		cpuid(cpu->number, 0x80000007, NULL, NULL, NULL, &edx);

	if (cpu->core_MHz) {
		print_MHz(cpu->core_MHz);
		printf(" processor (measured), TSC ");
		print_MHz(cpu->MHz);
		printf(".\n\n");
		return;
	}

	display_MHz(cpu);
	if (edx & (1 << 8))
		printf(" TSC frequency.\n\n");
	else
		printf(" processor (estimate).\n\n");
}

/* Read the APERF/MPERF counters if the CPU has them and the MSRs are readable. */
static int read_perf_counters(const struct cpudata *cpu,
			      unsigned long long *aperf, unsigned long long *mperf)
{
	unsigned int ecx = 0;
	unsigned int base = cpu->vendor == VENDOR_AMD ? 0xc0000000 : 0;

	if (cpu->cpuid_level < 6)
		return 0;
	cpuid(cpu->number, 6, NULL, NULL, &ecx, NULL);
	if (!(ecx & 1))
		return 0;
	return read_msr(cpu->number, base + MSR_APERF, aperf) == 1 &&
	       read_msr(cpu->number, base + MSR_MPERF, mperf) == 1;
}

static volatile sig_atomic_t nosignal = 0;

static void sighandler(int sig __attribute__((unused)))
{
	nosignal = 1;
}

void estimate_MHz(struct cpudata *cpu)
{
	unsigned long long int cycles[2]; /* gotta be 64 bit */
	unsigned long long aperf[2], mperf[2];
	int have_perf;
	unsigned int eax, ebx, ecx, edx;
	void (*old)(int);
	// coverity[DC.WEAK_CRYPTO]
	unsigned long r = rand();

	/* Make sure we have a TSC (and hence RDTSC) */
	cpuid(cpu->number, 1, &eax, &ebx, &ecx, &edx);
	if ((edx & (1<<4)) == 0) {
		printf("No TSC, MHz calculation cannot be performed.\n");
		cpu->MHz = 0;
		return;
	}

	old = signal(SIGALRM, sighandler);
	if (old == SIG_ERR) {
		printf("Some kind of signal failure.\n");
		cpu->MHz = 0;
		return;
	}

	have_perf = user_is_root && read_perf_counters(cpu, &aperf[0], &mperf[0]);

	cycles[0] = rdtsc();

	alarm(1);
	while (!nosignal)
		// coverity[DC.WEAK_CRYPTO]
		r = r * rand();

	nosignal = 0;
	signal(SIGALRM, old);

	cycles[1] = rdtsc();

	cpu->core_MHz = 0;
	if (have_perf && read_perf_counters(cpu, &aperf[1], &mperf[1]) &&
	    mperf[1] > mperf[0] && aperf[1] > aperf[0]) {
		double tsc_mhz = (double)(cycles[1] - cycles[0]) / 1e6;

		cpu->core_MHz = (unsigned int)(tsc_mhz *
			(double)(aperf[1] - aperf[0]) /
			(double)(mperf[1] - mperf[0]) + 0.5);
	}

	/* Check to see if rdtsc wrapped */
	if (cycles[1] < cycles[0])
		/* yes we did. */
		cpu->MHz = ((-1ULL - cycles[0]) + cycles[1]) / 1000000;
	else
		cpu->MHz = (cycles[1] - cycles[0]) / 1000000;

	if ((cpu->MHz % 50) > 15)
		cpu->MHz = ((cpu->MHz / 50) * 50) + 50;
	else
		cpu->MHz = ((cpu->MHz / 50) * 50);
}
