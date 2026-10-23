/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>
#ifdef __linux__
#include <sys/syscall.h>
#include <linux/perf_event.h>
#endif

#include <x86info.h>
#include <rdtsc.h>

#define MSR_MPERF	0xe7
#define MSR_APERF	0xe8

static void print_MHz(unsigned int mhz)
{
	if (mhz < 1000) {
		printf("%uMHz", mhz);
	} else {
		unsigned int centi = (mhz + 5) / 10;

		printf("%u.%02uGHz", centi / 100, centi % 100);
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

#ifdef __linux__
#define MSR_PMU_TYPE	"/sys/bus/event_source/devices/msr/type"
#define PERF_MSR_APERF	1
#define PERF_MSR_MPERF	2

/* Per-thread APERF/MPERF counters from the kernel's msr PMU; no root needed. */
static int perf_msr_open(unsigned int config)
{
	struct perf_event_attr attr;
	unsigned int type;
	FILE *f;
	int ok;

	f = fopen(MSR_PMU_TYPE, "r");
	if (!f)
		return -1;
	ok = fscanf(f, "%u", &type) == 1;
	fclose(f);
	if (!ok)
		return -1;

	memset(&attr, 0, sizeof(attr));
	attr.type = type;
	attr.size = sizeof(attr);
	attr.config = config;
	return (int)syscall(SYS_perf_event_open, &attr, 0, -1, -1, 0);
}

static int read_perf_msr_pmu(unsigned long long *aperf, unsigned long long *mperf)
{
	static int fds[2] = { -1, -1 };
	static int tried;

	if (!tried) {
		tried = 1;
		fds[0] = perf_msr_open(PERF_MSR_APERF);
		fds[1] = perf_msr_open(PERF_MSR_MPERF);
		if (fds[0] < 0 || fds[1] < 0) {
			if (fds[0] >= 0)
				close(fds[0]);
			if (fds[1] >= 0)
				close(fds[1]);
			fds[0] = fds[1] = -1;
		}
	}
	if (fds[0] < 0)
		return 0;
	return read(fds[0], aperf, sizeof(*aperf)) == sizeof(*aperf) &&
	       read(fds[1], mperf, sizeof(*mperf)) == sizeof(*mperf);
}
#else
static int read_perf_msr_pmu(unsigned long long *aperf, unsigned long long *mperf)
{
	(void)aperf;
	(void)mperf;
	return 0;
}
#endif

/*
 * Read the APERF/MPERF counters if the CPU has them: from the MSRs as
 * root, otherwise from the kernel's msr PMU.
 */
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
	if (user_is_root &&
	    read_msr(cpu->number, base + MSR_APERF, aperf) == 1 &&
	    read_msr(cpu->number, base + MSR_MPERF, mperf) == 1)
		return 1;
	return read_perf_msr_pmu(aperf, mperf);
}

/* Cores take most of a second to ramp up to boost clocks, so spin for a
 * while before measuring the core clock. The TSC rate needs no warm-up. */
#ifndef WARMUP_US
#define WARMUP_US	1000000.0
#endif
#ifndef MEASURE_US
#define MEASURE_US	100000.0
#endif

static double elapsed_us(const struct timespec *t0, const struct timespec *t1);

static unsigned long spin(unsigned long r, double us)
{
	struct timespec t0, t1;

	clock_gettime(CLOCK_MONOTONIC_RAW, &t0);
	do {
		// coverity[DC.WEAK_CRYPTO]
		r = r * rand();
		clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
	} while (elapsed_us(&t0, &t1) < us);
	return r;
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
	// coverity[DC.WEAK_CRYPTO]
	unsigned long r = rand();

	/* Make sure we have a TSC (and hence RDTSC) */
	cpuid(cpu->number, 1, &eax, &ebx, &ecx, &edx);
	if ((edx & (1<<4)) == 0) {
		printf("No TSC, MHz calculation cannot be performed.\n");
		cpu->MHz = 0;
		return;
	}

	have_perf = read_perf_counters(cpu, &aperf[0], &mperf[0]);

	/* Nothing left to measure if CPUID gives the TSC rate */
	known_MHz = cpuid_tsc_MHz(cpu);
	if (known_MHz && !have_perf) {
		cpu->MHz = known_MHz;
		cpu->core_MHz = 0;
		return;
	}

	if (have_perf) {
		r = spin(r, WARMUP_US);
		read_perf_counters(cpu, &aperf[0], &mperf[0]);
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts[0]);
	cycles[0] = rdtsc();

	r = spin(r, MEASURE_US);

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
