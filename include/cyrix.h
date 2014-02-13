#ifndef _CYRIX_H
#define _CYRIX_H
void decode_cyrix_tlb (int);
void identify_cyrix(struct cpudata *cpu);
void display_cyrix_info(struct cpudata *cpu);
#endif /* _CYRIX_H */
