/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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

/*
 * TSC rate in MHz from CPUID 0x15 (crystal clock ratio), or failing that
 * the base frequency from CPUID 0x16 on Intel parts with an invariant TSC.
 * Returns 0 if CPUID doesn't say.
 */
static unsigned int cpuid_tsc_MHz(const struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx, edx = 0;

	if (cpu->cpuid_level >= 0x15) {
		cpuid(cpu->number, 0x15, &eax, &ebx, &ecx, NULL);
		if (eax && ebx && ecx)
			return (unsigned int)((unsigned long long)ecx * ebx /
					      eax / 1000000);
	}

	if (cpu->vendor != VENDOR_INTEL || cpu->cpuid_level < 0x16)
		return 0;
	if (cpu->maxei >= 0x80000007)
		cpuid(cpu->number, 0x80000007, NULL, NULL, NULL, &edx);
	if (!(edx & (1 << 8)))
		return 0;
	cpuid(cpu->number, 0x16, &eax, NULL, NULL, NULL);
	return eax & 0xffff;
}

static double elapsed_us(const struct timespec *t0, const struct timespec *t1)
{
	return (double)(t1->tv_sec - t0->tv_sec) * 1e6 +
	       (double)(t1->tv_nsec - t0->tv_nsec) / 1e3;
}

void estimate_MHz(struct cpudata *cpu)
{
	unsigned long long int cycles[2]; /* gotta be 64 bit */
	unsigned long long aperf[2], mperf[2];
	struct timespec ts[2];
	int have_perf;
	unsigned int eax, ebx, ecx, edx, known_MHz;
	double tsc_mhz;
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

	have_perf = user_is_root && read_perf_counters(cpu, &aperf[0], &mperf[0]);

	/* Nothing left to measure if CPUID gives the TSC rate */
	known_MHz = cpuid_tsc_MHz(cpu);
	if (known_MHz && !have_perf) {
		cpu->MHz = known_MHz;
		cpu->core_MHz = 0;
		return;
	}

	old = signal(SIGALRM, sighandler);
	if (old == SIG_ERR) {
		printf("Some kind of signal failure.\n");
		cpu->MHz = 0;
		return;
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts[0]);
	cycles[0] = rdtsc();

	alarm(1);
	while (!nosignal)
		// coverity[DC.WEAK_CRYPTO]
		r = r * rand();

	nosignal = 0;
	signal(SIGALRM, old);

	cycles[1] = rdtsc();
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts[1]);

	/* Unsigned subtraction copes with rdtsc wrapping */
	tsc_mhz = (double)(cycles[1] - cycles[0]) / elapsed_us(&ts[0], &ts[1]);
	if (known_MHz)
		tsc_mhz = known_MHz;

	cpu->core_MHz = 0;
	if (have_perf && read_perf_counters(cpu, &aperf[1], &mperf[1]) &&
	    mperf[1] > mperf[0] && aperf[1] > aperf[0])
		cpu->core_MHz = (unsigned int)(tsc_mhz *
			(double)(aperf[1] - aperf[0]) /
			(double)(mperf[1] - mperf[0]) + 0.5);

	cpu->MHz = (unsigned int)(tsc_mhz + 0.5);
}

/* Fill cpu->MHz, measuring the TSC rate only once per run. */
void ensure_MHz(struct cpudata *cpu)
{
	static int measured;
	static unsigned int tsc_MHz;

	if (cpu->MHz)
		return;
	if (!measured) {
		tsc_MHz = cpuid_tsc_MHz(cpu);
		if (!tsc_MHz) {
			estimate_MHz(cpu);
			tsc_MHz = cpu->MHz;
		}
		measured = 1;
	}
	cpu->MHz = tsc_MHz;
}

/* True if a measured MHz is within 1% of a nominal SKU frequency. */
int MHz_matches(unsigned int measured, unsigned int nominal)
{
	unsigned int diff = measured > nominal ? measured - nominal :
						 nominal - measured;

	return nominal != 0 && diff * 100 <= nominal;
}
