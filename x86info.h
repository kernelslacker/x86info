#ifndef _X86INFO_H
#define _X86INFO_H
/* x86info by Dave Jones
 * Based on 'cpuid' by Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#ifdef linux
#include <linux/types.h>
#else
#include <machine/types.h>
#define __u32 int
#endif /* linux */

typedef __u32 u32;

#define VENDOR_AMD	1
#define VENDOR_CENTAUR 2
#define VENDOR_CYRIX 3
#define VENDOR_INTEL 4
#define VENDOR_RISE 5
#define VENDOR_TRANSMETA 6

struct cpudata {
	unsigned int number;
	unsigned int vendor;
	unsigned int family;
	unsigned int model;
	unsigned int stepping;
	unsigned int type;
	unsigned int brand;
	unsigned int cachesize_L1_I, cachesize_L1_D;
	unsigned int cachesize_L2;
	unsigned int cachesize_L3;
	unsigned int maxi, maxei;
	unsigned char name[80];
};

void cpuid (int, int, unsigned long *, unsigned long *, unsigned long *, unsigned long *);
void cpuid_UP (int, unsigned long *, unsigned long *, unsigned long *, unsigned long *);

void Identify_AMD (unsigned int, struct cpudata *cpu);
void Identify_Cyrix (unsigned int, unsigned int, struct cpudata *cpu);
void Identify_IDT (unsigned int, struct cpudata *cpu);
void Identify_Intel (unsigned int, struct cpudata *cpu);
void Identify_RiSE (unsigned int, struct cpudata *cpu);

void display_AMD_info(unsigned int maxei, struct cpudata *cpu);
void display_Cyrix_info(unsigned int maxi, unsigned int maxei, struct cpudata *cpu);
void display_IDT_info(unsigned int maxei, struct cpudata *cpu);
void display_Intel_info(unsigned int maxi, struct cpudata *cpu);
void display_RiSE_info(unsigned int maxi, unsigned int maxei, struct cpudata *cpu);

void decode_feature_flags (struct cpudata *cpu, int flags, int eflags);
void identify (struct cpudata *cpu);
void dumpregs (int cpunum);

int read_msr(int cpu, unsigned int index, unsigned long long *val);
void binary32(unsigned long value);
void binary64(unsigned long long value);
void dumpmsr (int cpunum, unsigned int msr, int size);
void dumpmsr_bin (int cpunum, unsigned int msr, int size);

void estimate_MHz(int cpunum);
int HaveCPUID(void);
void interpret_eblcr(u32 lo);
int issmp(int *numcpu, int verb);
void get_model_name (unsigned long maxei, struct cpudata *cpu);

extern int show_bluesmoke;
extern int show_cacheinfo;
extern int show_eblcr;
extern int show_flags;
extern int show_msr;
extern int show_registers;

extern int used_UP;
extern int silent;
extern int user_is_root;

#endif /* _X86INFO_H */
