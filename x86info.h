/* x86info by Dave Jones
 * Based on 'cpuid' by Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#include <linux/types.h>

#define VENDOR_AMD	1
#define VENDOR_CENTAUR 2
#define VENDOR_CYRIX 3
#define VENDOR_INTEL 4
#define VENDOR_RISE 5

struct cpudata {
	int vendor;
	int family;
	int model;
	int stepping;
	int type;
	int brand;
	int cachesize_L1;
	int cachesize_L2;
};

extern int used_UP;

void cpuid (int, int, unsigned long *, unsigned long *, unsigned long *, unsigned long *);
void cpuid_UP (int, unsigned long *, unsigned long *, unsigned long *, unsigned long *);
void decode_intel_tlb (int);
void decode_cyrix_tlb (int);
void dointel (int, unsigned int, struct cpudata *cpu);
void doamd (int, unsigned int, struct cpudata *cpu);
void docyrix (int, unsigned int, struct cpudata *cpu);
void doIDT (int, unsigned int, struct cpudata *cpu);
void doRise (int, unsigned int, struct cpudata *cpu);

void decode_feature_flags (struct cpudata *cpu, int flags);
void identify (int cpunum);
void dumpregs (int cpunum);

int rdmsr(int cpu, unsigned int index, unsigned long long *val);
void long2binstr(long value);
void dumpmsr (int cpunum, unsigned int msr);
void dumpmsr_bin (int cpunum, unsigned int msr);
void estimate_MHz(int cpunum);
void decode_bluesmoke(int cpunum);
int HaveCPUID(void);
void interpret_eblcr(__u32 lo);
