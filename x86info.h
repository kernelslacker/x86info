#ifndef _X86INFO_H
#define _X86INFO_H

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
	unsigned int phyaddr_bits;
	unsigned int viraddr_bits;
	unsigned int cpuid_level, maxei, maxei2;
	char name[CPU_NAME_LEN];
	enum connector connector;
	unsigned int flags_ecx;
	unsigned int flags_edx;
	unsigned int eflags_ecx;
	unsigned int eflags_edx;
	unsigned int MHz;
	unsigned int nr_cores;
	unsigned int nr_logical;
	char *info_url;
	char *datasheet_url;
	char *errata_url;
	/* Intel specific bits */
	unsigned int brand;
	unsigned int apicid;
	char serialno[30];

	unsigned int phys_proc_id;
	unsigned int initial_apicid;
	unsigned int x86_max_cores;
	unsigned int cpu_core_id;
	unsigned int num_siblings;
};

void cpuid_UP(unsigned int idx,
	unsigned long *eax, unsigned long *ebx, unsigned long *ecx, unsigned long *edx);
void cpuid(unsigned int cpu, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
void cpuid4(unsigned int CPU_number, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
void cpuid_count(unsigned int CPU_number, unsigned int op, int count,
	unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
unsigned int cpuid_ebx(unsigned int CPU_number, unsigned int op);


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

void Identify_AMD(struct cpudata *cpu);
void Identify_Cyrix(struct cpudata *cpu);
void identify_centaur(struct cpudata *cpu);
void Identify_Intel(struct cpudata *cpu);
void identify_RiSE(struct cpudata *cpu);
void identify_natsemi(struct cpudata *cpu);
void identify_sis(struct cpudata *cpu);

void display_AMD_info(struct cpudata *cpu);
void display_Cyrix_info(struct cpudata *cpu);
void display_centaur_info(struct cpudata *cpu);
void display_basic_Intel_info(struct cpudata *cpu);
void display_extended_Intel_info(struct cpudata *cpu);

void get_feature_flags(struct cpudata *cpu);
void display_feature_flags(struct cpudata *cpu);
void show_extra_intel_flags(struct cpudata *cpu);

void parse_command_line(int argc, char **argv);

void get_cpu_info_basics(struct cpudata *cpu);
void identify(struct cpudata *cpu);
void show_info(struct cpudata *cpu);

int read_msr(int cpu, unsigned int idx, unsigned long long *val);
void binary(unsigned int n, unsigned long value);
void binary32(unsigned long value);
void binary64(unsigned long long value);
void dumpmsr(int cpunum, unsigned int msr, int size);
void dumpmsr_bin(int cpunum, unsigned int msr, int size);
void dump_raw_cpuid(int cpunum, unsigned int begin, unsigned int end);

void dump_mtrrs(struct cpudata *cpu);
void dump_apics(struct cpudata *cpu);

void display_MHz(struct cpudata *cpu);
void estimate_MHz(struct cpudata *cpu);

int HaveCPUID(void);
void interpret_eblcr(u32 lo);
int enumerate_cpus(void);
void get_model_name(struct cpudata *cpu);
void decode_connector(enum connector type);
void show_benchmarks(struct cpudata *cpu);
void decode_serial_number(struct cpudata *cpu);

void get_topology(struct cpudata *head);
void display_topology(struct cpudata *head);
void get_intel_topology(struct cpudata *cpu);

void decode_AMD_cacheinfo(struct cpudata *cpu);

extern unsigned int nrCPUs;

extern struct cpudata *firstcpu;

extern unsigned int user_is_root;
extern unsigned int need_root;


/* command line args */
extern unsigned int show_apic;
extern unsigned int show_bench;
extern unsigned int show_bios;
extern unsigned int show_machine_check;
extern unsigned int show_bugs;
extern unsigned int show_cacheinfo;
extern unsigned int show_connector;
extern unsigned int show_eblcr;
extern unsigned int show_msr;
extern unsigned int show_microcode;
extern unsigned int show_mtrr;
extern unsigned int show_pm;
extern unsigned int show_cpuid;
extern unsigned int show_urls;
extern unsigned int show_mptable;
extern unsigned int show_feature_flags;
extern unsigned int show_MHz;
extern unsigned int show_addr_sizes;
extern unsigned int all_cpus;
extern unsigned int debug;
extern unsigned int verbose;

#define X86_FEATURE_HT		(1<<28)
#define X86_FEATURE_MTRR	(1<<12)
#define X86_FEATURE_APIC	(1<<9)

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
