/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>
#include <pthread.h>
#ifdef __linux__
#include <sys/syscall.h>
#include <linux/perf_event.h>
#endif

#include <x86info.h>
#include <rdtsc.h>

#define MSR_MPERF	0xe7
#define MSR_APERF	0xe8

/* CPUID 6 ECX[0]: the CPU has the APERF/MPERF counters. */
static int has_perf_counters(const struct cpudata *cpu)
{
	unsigned int ecx = 0;

	if (cpu->cpuid_level < 6)
		return 0;
	cpuid(cpu->number, 6, NULL, NULL, &ecx, NULL);
	return ecx & 1;
}

static void print_MHz(unsigned int mhz)
{
	if (mhz < 1000) {
		printf("%uMHz", mhz);
	} else {
		unsigned int centi = (mhz + 5) / 10;

		printf("%u.%02uGHz", centi / 100, centi % 100);
	}
}

/*
 * With an invariant TSC the measured rate is the TSC rate, not the core
 * clock; APERF/MPERF, when readable, give the core clock over the run.
 */
void display_MHz_line(const struct cpudata *cpu)
{
	if (cpu->core_MHz) {
		print_MHz(cpu->core_MHz);
		printf(" processor (measured), TSC ");
		print_MHz(cpu->MHz);
		printf(".\n\n");
		return;
	}

	print_MHz(cpu->MHz);
	if (cpu->apm_edx & X86_APM_INVARIANT_TSC)
		printf(" TSC frequency.");
	else
		printf(" processor (estimate).");
	if (!is_root() && has_perf_counters(cpu))
		printf(" Run as root for the core clock.");
	printf("\n\n");
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

static __thread int perf_fds[2] = { -1, -1 };
static __thread int perf_tried;

static void perf_msr_close(void)
{
	if (perf_fds[0] >= 0)
		close(perf_fds[0]);
	if (perf_fds[1] >= 0)
		close(perf_fds[1]);
	perf_fds[0] = perf_fds[1] = -1;
	perf_tried = 0;
}

static int read_perf_msr_pmu(unsigned long long *aperf, unsigned long long *mperf)
{
	int *fds = perf_fds;
	int tried = perf_tried;

	if (!tried) {
		perf_tried = 1;
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
static void perf_msr_close(void)
{
}

static int read_perf_msr_pmu(unsigned long long *aperf, unsigned long long *mperf)
{
	(void)aperf;
	(void)mperf;
	return 0;
}
#endif

static unsigned int perf_msr_base(const struct cpudata *cpu)
{
	return cpu->vendor == VENDOR_AMD ? 0xc0000000 : 0;
}

/*
 * Read the APERF/MPERF counters if the CPU has them: from the MSRs as
 * root, otherwise from the kernel's msr PMU.
 */
static int read_perf_counters(const struct cpudata *cpu,
			      unsigned long long *aperf, unsigned long long *mperf)
{
	unsigned int base = perf_msr_base(cpu);

	if (!has_perf_counters(cpu))
		return 0;
	if (is_root() &&
	    read_msr(cpu->number, base + MSR_APERF, aperf) == 1 &&
	    read_msr(cpu->number, base + MSR_MPERF, mperf) == 1)
		return 1;
	return read_perf_msr_pmu(aperf, mperf);
}

/* Open the MSR driver for this CPU, so the measuring threads find it ready. */
static void probe_msr_access(const struct cpudata *cpu)
{
	unsigned long long val;

	if (is_root() && has_perf_counters(cpu))
		read_msr(cpu->number, perf_msr_base(cpu) + MSR_APERF, &val);
}

/* Cores take most of a second to ramp up to boost clocks, so spin for a
 * while before measuring the core clock. The TSC rate needs no warm-up. */
#ifndef WARMUP_US
#define WARMUP_US	6000000.0
#endif
#ifndef MEASURE_US
#define MEASURE_US	100000.0
#endif

static double elapsed_us(const struct timespec *t0, const struct timespec *t1)
{
	return (double)(t1->tv_sec - t0->tv_sec) * 1e6 +
	       (double)(t1->tv_nsec - t0->tv_nsec) / 1e3;
}

static void spin(double us)
{
	struct timespec t0, t1;

	clock_gettime(CLOCK_MONOTONIC_RAW, &t0);
	do {
		clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
	} while (elapsed_us(&t0, &t1) < us);
}

/*
 * TSC rate in MHz from CPUID 0x15 (crystal clock ratio), or failing that
 * the base frequency from CPUID 0x16 on Intel parts with an invariant TSC.
 * Returns 0 if CPUID doesn't say.
 */
static unsigned int cpuid_tsc_MHz(const struct cpudata *cpu)
{
	unsigned int eax, ebx, ecx;

	if (cpu->cpuid_level >= 0x15) {
		cpuid(cpu->number, 0x15, &eax, &ebx, &ecx, NULL);
		if (eax && ebx && ecx)
			return (unsigned int)((unsigned long long)ecx * ebx /
					      eax / 1000000);
	}

	if (cpu->vendor != VENDOR_INTEL || cpu->cpuid_level < 0x16)
		return 0;
	if (!(cpu->apm_edx & X86_APM_INVARIANT_TSC))
		return 0;
	cpuid(cpu->number, 0x16, &eax, NULL, NULL, NULL);
	return eax & 0xffff;
}

/* Measure the TSC rate and, if want_core, the core clock as well. */
static void measure_MHz(struct cpudata *cpu, int want_core)
{
	unsigned long long int cycles[2]; /* gotta be 64 bit */
	unsigned long long aperf[2], mperf[2];
	struct timespec ts[2];
	int have_perf;
	unsigned int eax, ebx, ecx, edx, known_MHz;
	double tsc_mhz;

	/* Make sure we have a TSC (and hence RDTSC) */
	cpuid(cpu->number, 1, &eax, &ebx, &ecx, &edx);
	if (!(edx & X86_FEATURE_TSC)) {
		printf("No TSC, MHz calculation cannot be performed.\n");
		cpu->MHz = 0;
		return;
	}

	have_perf = want_core && host_can_measure() &&
		    read_perf_counters(cpu, &aperf[0], &mperf[0]);

	/* Nothing left to measure if CPUID gives the TSC rate */
	known_MHz = cpuid_tsc_MHz(cpu);
	if ((known_MHz && !have_perf) || !host_can_measure()) {
		cpu->MHz = known_MHz;
		cpu->core_MHz = 0;
		return;
	}

	if (have_perf) {
		spin(WARMUP_US);
		read_perf_counters(cpu, &aperf[0], &mperf[0]);
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts[0]);
	cycles[0] = rdtsc();

	spin(MEASURE_US);

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

static void *measure_thread(void *arg)
{
	struct cpudata *cpu = arg;

	if (bind_cpu(cpu->number))
		return NULL;
	measure_MHz(cpu, 1);
	perf_msr_close();
	return NULL;
}

/*
 * Measure the clocks of every CPU with --all-cpus, otherwise of the first
 * CPU of each group, all at once. The rest take their group leader's
 * figures. leader[i] is the index of the first CPU like cpus[i].
 */
void estimate_MHz_cpus(struct cpudata *cpus, unsigned int n,
		       const unsigned int *leader)
{
	struct worker {
		pthread_t tid;
		unsigned char started;
	} *workers = calloc(n, sizeof(*workers));
	unsigned int i;

	if (!workers) {
		perror("calloc");
		return;
	}

	for (i = 0; i < n; i++)
		if (opts.all_cpus || leader[i] == i)
			probe_msr_access(&cpus[i]);

	for (i = 0; i < n; i++) {
		if (!opts.all_cpus && leader[i] != i)
			continue;
		if (pthread_create(&workers[i].tid, NULL, measure_thread, &cpus[i]) == 0)
			workers[i].started = 1;
		else
			fprintf(stderr, "CPU #%u: unable to start MHz measurement.\n",
				cpus[i].number);
	}

	for (i = 0; i < n; i++) {
		if (workers[i].started)
			pthread_join(workers[i].tid, NULL);
	}

	if (!opts.all_cpus) {
		for (i = 0; i < n; i++) {
			unsigned int lead = leader[i];

			if (lead == i)
				continue;
			cpus[i].MHz = cpus[lead].MHz;
			cpus[i].core_MHz = cpus[lead].core_MHz;
		}
	}

	free(workers);
}

/* Fill cpu->MHz, measuring the TSC rate only once per run. */
void ensure_MHz(struct cpudata *cpu)
{
	static int measured;
	static unsigned int tsc_MHz;

	if (cpu->MHz)
		return;
	if (!measured) {
		measure_MHz(cpu, 0);
		tsc_MHz = cpu->MHz;
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
