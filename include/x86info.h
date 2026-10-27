#pragma once

#include <stdbool.h>
#include <stdint.h>

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
	CONN_SOCKET_S1G3,
	CONN_SOCKET_F_R2,
	CONN_SOCKET_AM3,
	CONN_SOCKET_G34,
	CONN_SOCKET_ASB2,
	CONN_SOCKET_C32,
	CONN_SOCKET_FP1,
	CONN_SOCKET_FS1,
	CONN_SOCKET_FM1,
	CONN_SOCKET_FT1,
	CONN_SOCKET_FP2,
	CONN_SOCKET_FS1r2,
	CONN_SOCKET_FM2,
};

#define CPU_NAME_LEN 80
#define TOPOLOGY_NONE	0
#define TOPOLOGY_LEGACY	1	/* CPUID 1 / 4 derived */
#define TOPOLOGY_X2APIC	2	/* CPUID 0x1F / 0xB */

struct cpudata {
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
	unsigned int phyaddr_bits;
	unsigned int viraddr_bits;
	unsigned int cpuid_level, maxei, maxei2;
	char name[CPU_NAME_LEN];
	enum connector connector;
	unsigned int flags_ecx;
	unsigned int flags_edx;
	unsigned int eflags_ecx;
	unsigned int eflags_edx;
	unsigned int apm_edx;		/* CPUID 0x80000007 EDX */
	unsigned int MHz;		/* TSC rate */
	unsigned int core_MHz;		/* from APERF/MPERF, 0 if unknown */
	/* Intel specific bits */
	unsigned int brand;
	char serialno[30];

	unsigned int phys_proc_id;
	unsigned int topology;		/* TOPOLOGY_* */
	unsigned int core_type;		/* CORE_TYPE_*, 0 if not hybrid */
	unsigned int x86_max_cores;
	unsigned int cpu_core_id;
	unsigned int num_siblings;
};

/* Both return zero on success; on failure the outputs are zeroed. */
int cpuid(unsigned int cpu, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
int cpuid_count(unsigned int CPU_number, unsigned int op, int count,
	unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);


static inline unsigned int family(const struct cpudata *c)
{
	return c->family + c->efamily;
}

static inline unsigned int model(const struct cpudata *c)
{
	return (c->emodel << 4) + c->model;
}

static inline unsigned int tuple(const struct cpudata *c)
{
	return (c->family << 8) | (c->model << 4) | c->stepping;
}

#define ARRAY_SIZE(x) ((int)(sizeof(x)/sizeof(x[0])))

/* Append to cpu->name, at the end of whatever is already there. */
void cpu_name_append(struct cpudata *cpu, const char *str);
#define add_to_cpuname(x) cpu_name_append(cpu, x)

#define get_name(title,type,table) \
static const char *get_##title##_name(type id) \
{ \
	int i; \
        for (i = 0; i < ARRAY_SIZE(table); i++) \
                if (id == table[i].id) \
                        return table[i].name;   \
        return NULL; \
}

void identify_misc(struct cpudata *cpu);

extern const char *cpuid_80000007_edx_flags_desc[32];
void get_feature_flags(struct cpudata *cpu);
void display_feature_flags(const struct cpudata *cpu);

int bind_cpu(unsigned int cpunr);

/* Per-CPU descriptor cache slots: >= 0 is open. */
#define CPUDEV_CLOSED	-1
#define CPUDEV_FAILED	-2

/* Open or fetch /dev/cpu/<cpu>/<node> through the cache fds[MAX_CPUS]. */
int cpudev_fd(int *fds, unsigned int cpu, const char *node);

int native_cpuid(unsigned int cpunr, unsigned long long idx,
	unsigned int *eax, unsigned int *ebx,
	unsigned int *ecx, unsigned int *edx);

void parse_command_line(int argc, char **argv);

int get_cpu_info_basics(struct cpudata *cpu);
void identify(struct cpudata *cpu);
void show_header(struct cpudata *cpu);
void show_info(struct cpudata *cpu, int header);

int read_msr(int cpu, unsigned int idx, unsigned long long *val);
void binary(unsigned int n, unsigned long value);
void binary32(unsigned long value);
void binary64(unsigned long long value);
void dumpmsr(int cpunum, unsigned int msr, int size);
void dumpmsr_bin(int cpunum, unsigned int msr, int size);
void dump_mc_banks(int cpunum, unsigned int banks);
enum vendor vendor_from_cpuid(unsigned int ebx, unsigned int edx);
unsigned int sane_max_leaf(unsigned int max, unsigned int base);
void dump_raw_cpuid(int cpunum, unsigned int begin, unsigned int end);

void dump_mtrrs(struct cpudata *cpu);
void dump_apics(const struct cpudata *cpu);

void display_MHz_line(const struct cpudata *cpu);
void estimate_MHz_cpus(struct cpudata *cpus, unsigned int n,
		       const unsigned int *leader);
void ensure_MHz(struct cpudata *cpu);
int MHz_matches(unsigned int measured, unsigned int nominal);

int HaveCPUID(void);
#ifdef CPUID_REPLAY
unsigned int *replay_cpu_ids(unsigned int *count);
#endif
int enumerate_cpus(void);
void decode_microcode(struct cpudata *cpu);
void display_model_name(struct cpudata *cpu);
void decode_connector(enum connector type);

unsigned int get_count_order(unsigned int count);
void get_topology(struct cpudata *head);
void get_core_type(struct cpudata *cpu);
const char *core_type_name(const struct cpudata *cpu);
void clear_uniform_core_types(struct cpudata *cpus, unsigned int n);
void display_topology(struct cpudata *cpus, unsigned int n);

/* Upper bound for per-CPU descriptor tables. */
#define MAX_CPUS 8192

extern unsigned int nrCPUs;

bool is_root(void);

/* Probes of the running machine; a replay build answers without them. */
int host_longnop(void);			/* 1 yes, 0 no, -1 unknown */
int host_microcode_version(unsigned int cpu, unsigned long long *ver);
int host_can_measure(void);		/* may time or read counters */


/* command line args */
struct options {
	unsigned int show_apic;
	unsigned int show_bios;
	unsigned int show_machine_check;
	unsigned int show_bugs;
	unsigned int show_cacheinfo;
	unsigned int show_connector;
	unsigned int show_msr;
	unsigned int show_microcode;
	unsigned int show_mtrr;
	unsigned int show_pm;
	unsigned int show_cpuid;
	unsigned int show_mptable;
	unsigned int show_feature_flags;
	unsigned int show_MHz;
	unsigned int show_addr_sizes;
	unsigned int all_cpus;
	unsigned int debug;
	unsigned int verbose;
	unsigned int need_root;
};

extern struct options opts;

/* The BIOS PST tables are decoded by the PowerNow path. */
static inline int want_powernow(void)
{
	return opts.show_pm || opts.show_bios;
}

#define CORE_TYPE_PERFORMANCE	1
#define CORE_TYPE_EFFICIENCY	2
#define CORE_TYPE_UNKNOWN	3

#define X86_FEATURE_PSN		(1<<18)
#define X86_FEATURE_HT		(1<<28)
#define X86_FEATURE_TSC		(1<<4)
#define X86_APM_INVARIANT_TSC	(1<<8)	/* CPUID 0x80000007 EDX */
#define X86_FEATURE_MTRR	(1<<12)
#define X86_FEATURE_APIC	(1<<9)

#define __stringify_1(x...)     #x
#define __stringify(x...)       __stringify_1(x)
