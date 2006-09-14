#ifndef _X86INFO_H
#define _X86INFO_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define VENDOR_AMD	1
#define VENDOR_CENTAUR 2
#define VENDOR_CYRIX 3
#define VENDOR_INTEL 4
#define VENDOR_NATSEMI 5
#define VENDOR_RISE 6
#define VENDOR_TRANSMETA 7
#define VENDOR_SIS 8

struct cpudata {
	struct cpudata *next;
	unsigned int number;
	unsigned int vendor;
	unsigned int efamily;
	unsigned int family;
	unsigned int model;
	unsigned int emodel;
	unsigned int stepping;
	unsigned int type;
	unsigned int brand;
	unsigned int cachesize_L1_I, cachesize_L1_D;
	unsigned int cachesize_L2;
	unsigned int cachesize_L3;
	unsigned int cachesize_trace;
	unsigned int maxi, maxei, maxei2;
	char name[80];
	unsigned char connector;
	unsigned int flags_ecx;
	unsigned int flags_edx;
	unsigned int eflags_ecx;
	unsigned int eflags_edx;
	unsigned int MHz;
	char * datasheet_url;
	char * errata_url;
};

#define tuple(c) ((c->family<<8)|(c->model<<4)|(c->stepping))
#define etuple(c) ((c->efamily<<8)|(c->model<<4)|(c->stepping))

#define CONN_UNKNOWN		0
#define CONN_SOCKET_3		1
#define CONN_SOCKET_4		2
#define CONN_SOCKET_5		3
#define CONN_SOCKET_7		4
#define CONN_SOCKET_370		5
#define CONN_SOCKET_370_FCPGA	6
#define CONN_SOCKET_5_7		7
#define CONN_SUPER_SOCKET_7	8
#define CONN_SLOT_A		9
#define CONN_SOCKET_A		10
#define CONN_SOCKET_A_SLOT_A	11
#define CONN_SOCKET_A_OR_SLOT_A	12
#define CONN_SOCKET_57B		13
#define CONN_MOBILE_7		14
#define CONN_SOCKET_8		15
#define CONN_SLOT_1		16
#define CONN_SLOT_2		17
#define CONN_SOCKET_423		18
#define CONN_MMC		19
#define CONN_MMC2		20
#define CONN_BGA474		21
#define CONN_BGA		22
#define CONN_SOCKET_754		23
#define CONN_SOCKET_478		24
#define CONN_SOCKET_603		25
#define CONN_MICROFCBGA		26
#define CONN_SOCKET_939		27
#define CONN_SOCKET_940		28
#define CONN_LGA775		29

extern void cpuid (int, unsigned int, unsigned long *, unsigned long *, unsigned long *, unsigned long *);
extern void cpuid_UP (int, unsigned long *, unsigned long *, unsigned long *, unsigned long *);

extern void Identify_AMD (struct cpudata *cpu);
extern void Identify_Cyrix (struct cpudata *cpu);
extern void Identify_IDT (struct cpudata *cpu);
extern void Identify_Intel (struct cpudata *cpu);
extern void Identify_RiSE (struct cpudata *cpu);
extern void Identify_NatSemi (struct cpudata *cpu);
extern void Identify_SiS (struct cpudata *cpu);

extern void display_AMD_info(struct cpudata *cpu);
extern void display_Cyrix_info(struct cpudata *cpu);
extern void display_IDT_info(struct cpudata *cpu);
extern void display_Intel_info(struct cpudata *cpu);
extern void display_NatSemi_info(struct cpudata *cpu);
extern void display_RiSE_info(struct cpudata *cpu);
extern void display_SiS_info(struct cpudata *cpu);

extern void decode_feature_flags (struct cpudata *cpu);
extern void identify (struct cpudata *cpu);
extern void show_info (struct cpudata *cpu);

extern int read_msr(int cpu, unsigned int idx, unsigned long long *val);
extern void binary(unsigned int n, unsigned long value);
extern void binary32(unsigned long value);
extern void binary64(unsigned long long value);
extern void dumpmsr (int cpunum, unsigned int msr, int size);
extern void dumpmsr_bin (int cpunum, unsigned int msr, int size);

extern void dump_mtrrs (struct cpudata *cpu);

extern void estimate_MHz(struct cpudata *cpu);
extern int HaveCPUID(void);
extern void interpret_eblcr(u32 lo);
extern int issmp(int verb);
extern int enumerate_cpus(void);
extern void get_model_name (struct cpudata *cpu);
extern void decode_connector (unsigned int type);
extern void show_benchmarks (void);

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

#endif /* _X86INFO_H */
