#ifndef _CPUID_H
#define _CPUID_H

extern void cpuid_UP (unsigned int idx, unsigned long *eax, unsigned long *ebx,
	unsigned long *ecx, unsigned long *edx);
extern void cpuid(unsigned int cpu, unsigned long long idx, unsigned int *eax,
	unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
extern void cpuid4(unsigned int cpu, unsigned long long idx, unsigned int *eax,
	unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
extern void cpuid_count(struct cpudata *cpu, unsigned int op, int count,
	unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
extern unsigned int cpuid_ebx(struct cpudata *cpu, unsigned int op);


#endif /* _CPUID_H */
