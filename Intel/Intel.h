#ifndef _INTEL_H
#define _INTEL_H
void decode_Intel_bluesmoke(int cpunum);
void decode_Intel_TLB (int i, int family);
void get_intel_cacheinfo (struct cpudata *c);
#endif /* _INTEL_H */
