/* x86info by Dave Jones
 * Based on 'cpuid' by Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#define VENDOR_AMD	1
#define VENDOR_CENTAUR 2
#define VENDOR_CYRIX 3
#define VENDOR_INTEL 4

struct cpudata {
	int vendor;
	int family;
	int model;
	int stepping;
	int cachesize_L1;
	int cachesize_L2;
};

extern void cpuid(int,long *,long *,long *,long *);
extern void decode_intel_tlb(int);
extern void decode_cyrix_tlb(int);
extern void dointel(int, struct cpudata *cpu);
extern void doamd(int, struct cpudata *cpu);
extern void docyrix(int, struct cpudata *cpu);
extern void doIDT(int, struct cpudata *cpu);
extern void decode_feature_flags(struct cpudata *cpu, int flags);
