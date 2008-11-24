#ifndef _X86INFO_H
#define _X86INFO_H

#include "cpuid.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

enum vendor {
	VENDOR_UNKNOWN = 0 ,
	VENDOR_AMD,
	VENDOR_CENTAUR,
	VENDOR_CYRIX,
	VENDOR_INTEL,
	VENDOR_NATSEMI,
	VENDOR_RISE,
	VENDOR_TRANSMETA,
	VENDOR_SIS,
};

enum connector {
	CONN_UNKNOWN = 0,
	CONN_SOCKET_3,
	CONN_SOCKET_4,
	CONN_SOCKET_5,
	CONN_SOCKET_7,
	CONN_SOCKET_370,
	CONN_SOCKET_370_FCPGA,
	CONN_SOCKET_5_7,
	CONN_SUPER_SOCKET_7,
	CONN_SLOT_A,
	CONN_SOCKET_A,
	CONN_SOCKET_A_SLOT_A,
	CONN_SOCKET_A_OR_SLOT_A,
	CONN_SOCKET_57B,
	CONN_MOBILE_7,
	CONN_SOCKET_8,
	CONN_SLOT_1,
	CONN_SLOT_2,
	CONN_SOCKET_423,
	CONN_MMC,
	CONN_MMC2,
	CONN_BGA474,
	CONN_BGA,
	CONN_SOCKET_754,
	CONN_SOCKET_478,
	CONN_SOCKET_603,
	CONN_MICROFCBGA,
	CONN_SOCKET_939,
	CONN_SOCKET_940,
	CONN_LGA775,
	CONN_SOCKET_F,
	CONN_SOCKET_AM2,
	CONN_SOCKET_S1G1,
	CONN_SOCKET_S1G2,
	CONN_SOCKET_F_R2,
	CONN_SOCKET_AM2_R2,
};

#define CPU_NAME_LEN 80
struct cpudata {
	struct cpudata *next;
	unsigned int number;
	enum vendor vendor;
	unsigned int efamily;
	unsigned int family;
	unsigned int model;
	unsigned int emodel;
	unsigned int stepping;
	unsigned int type;
	unsigned int cachesize_L1_I, cachesize_L1_D;
	unsigned int cachesize_L2;
	unsigned int cachesize_L3;
	unsigned int cachesize_trace;
	unsigned int maxi, maxei, maxei2;
	char name[CPU_NAME_LEN];
	enum connector connector;
	unsigned int flags_ecx;
	unsigned int flags_edx;
	unsigned int eflags_ecx;
	unsigned int eflags_edx;
	unsigned int MHz;
	unsigned int nr_cores;
	unsigned int nr_logical;
	char *datasheet_url;
	char *errata_url;
	/* Intel specific bits */
	unsigned int brand;
	unsigned int apicid;
	char serialno[30];
};

#define family(c) (c->family + c->efamily)
#define model(c) ((c->emodel << 4) + c->model)

#define tuple(c) ((c->family<<8)|(c->model<<4)|(c->stepping))
#define etuple(c) ((c->efamily<<8)|(c->model<<4)|(c->stepping))

#define ARRAY_SIZE(x) ((int)(sizeof(x)/sizeof(x[0])))

#define get_name(title,type,table) \
static const char *get_##title##_name(type id) \
{ \
	int i; \
        for (i = 0; i < ARRAY_SIZE(table); i++) \
                if (id == table[i].id) \
                        return table[i].name;   \
        return NULL; \
}

extern void Identify_AMD (struct cpudata *cpu);
extern void Identify_Cyrix (struct cpudata *cpu);
extern void identify_centaur(struct cpudata *cpu);
extern void Identify_Intel (struct cpudata *cpu);
extern void identify_RiSE(struct cpudata *cpu);
extern void identify_natsemi(struct cpudata *cpu);
extern void identify_sis(struct cpudata *cpu);

extern void display_AMD_info(struct cpudata *cpu);
extern void display_Cyrix_info(struct cpudata *cpu);
extern void display_centaur_info(struct cpudata *cpu);
extern void display_Intel_info(struct cpudata *cpu);

extern void get_feature_flags(struct cpudata *cpu);
extern void show_feature_flags(struct cpudata *cpu);
extern void get_cpu_info_basics(struct cpudata *cpu);
extern void identify(struct cpudata *cpu);
extern void show_info(struct cpudata *cpu);

extern int read_msr(int cpu, unsigned int idx, unsigned long long *val);
extern void binary(unsigned int n, unsigned long value);
extern void binary32(unsigned long value);
extern void binary64(unsigned long long value);
extern void dumpmsr (int cpunum, unsigned int msr, int size);
extern void dumpmsr_bin (int cpunum, unsigned int msr, int size);
extern void dumpregs(int cpunum, unsigned int begin, unsigned int end);

extern void dump_mtrrs (struct cpudata *cpu);

extern void estimate_MHz(struct cpudata *cpu);
extern int HaveCPUID(void);
extern void interpret_eblcr(u32 lo);
extern int issmp(int verb);
extern int enumerate_cpus(void);
extern void get_model_name(struct cpudata *cpu);
extern void decode_connector(enum connector type);
extern void show_benchmarks(struct cpudata *cpu);
extern void decode_serial_number(struct cpudata *cpu);

extern void show_intel_topology(struct cpudata *cpu);

void decode_AMD_cacheinfo(struct cpudata *cpu);
void decode_Intel_caches(struct cpudata *cpu, int output);

extern int show_bench;
extern int show_bios;
extern int show_bluesmoke;
extern int show_bugs;
extern int show_cacheinfo;
extern int show_connector;
extern int show_eblcr;
extern int verbose;
extern int show_microcode;
extern int show_msr;
extern int show_mtrr;
extern int show_pm;
extern int show_registers;
extern int show_urls;

extern unsigned int nrCPUs;

extern int used_UP;
extern int silent;
extern int user_is_root;

#define X86_FEATURE_MTRR	1<<12

#define _GNU_SOURCE
#define __USE_GNU
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
static inline void bind_cpu(struct cpudata *cpu)
{
	cpu_set_t set;

	if (sched_getaffinity(getpid(), sizeof(set), &set) == 0) {
		CPU_ZERO(&set);
		CPU_SET(cpu->number, &set);
		sched_setaffinity(getpid(), sizeof(set), &set);
	}
}

#endif /* _X86INFO_H */
