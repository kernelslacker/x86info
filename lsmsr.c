/*
 * Author: Andreas Herrmann <andreas.herrmann3@amd.com>
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 *
 * Licensed under the terms of the GNU GENERAL PUBLIC LICENSE version 2.
 * See file COPYING for details.
 */

#define _LARGEFILE64_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>

#include "msr.h"
#include "cpuid.h"
#include "x86info.h"
#include "AMD/k8.h"
#include "AMD/fam10h.h"
#include "generic_msr.h"

/* Todos:
 * - add (list and eventually write) support for write-only MSR
 * - add decoding support for bit fields
 * - proper check for MSR support
 * - add support for generic MSRs for non-Intel/AMD CPUs
 */

#define VERSION "0.815"

struct {
	int fd;
	int show_all;
	int list;
	int verbosity;
	int cpu;
	enum vendor vendor;
	int family;
	int model;
	struct reg_spec *msr_table;
	const char *prog;
	const char *msr_name;
	const char *msr_search;
	uint32_t reg;
} g = {
	.reg = -1,
};

int used_UP = 0;

/* getopt_long stuff */
static struct option lopts[] = {
	{"all", no_argument, 0, 'a'},		/* show all MSRs */
	{"cpu", required_argument, 0, 'c'},	/* CPU number */
	{"family", required_argument, 0, 'f'},	/* CPU family */
	{"list", no_argument, 0, 'l'},		/* list known MSRs */
	{"register", required_argument, 0, 'r'},  /* register name or address */
	{"help", no_argument, 0, 'h'},		/* help */
	{"model", required_argument, 0, 'm'},	/* CPU model */
	{"version", no_argument, 0, 'v'},	/* version info */
	{"verbosity", required_argument, 0, 'V'}, /* verbosity */
	{0, 0, 0, 0}
};

#define OPTSTRING "ac:C:f:hlm:r:vV:"

#define _USAGE "[OPTION] <MSR>\n"

#define _HELP \
"  -a, --all                  show info for all known MSRs\n"\
"  -c, --cpu <CPU>            specify CPU for which MSRs are shown (default 0)\n"\
"  -f, --family <cpu family>  set cpu family to be used\n"\
"  -l, --list                 list info about MSR(s), its addresses and fields\n"\
"  -r, --register <name|addr> select register by name or address\n"\
"  -h, --help                 show help\n"\
"  -m, --model                set cpu model to be used\n"\
"  -v, --version              show version info\n"\
"  -V, --verbosity <n>        set verbosity\n"\
"                             V=1 show all fields of the register\n"\
"                             V=2 dito and show reserved fields\n"\
"                             V=3 like V=1 but each field on separate line\n"\
"                             V=4 dito and show reserved fields\n"\
"\n"\
" Display model specific registers on AMD64 processors.\n"\
" If <MSR> is given information for all MSRs for which <MSR> matches the\n"\
" beginning of its name are displayed. Precedence of MSR selection is:\n"\
" \" -r <name|addr>\" overrules \"<MSR>\" overrules \"-a\".\n"\
"\n"

static void usage(void)
{
	fprintf(stderr, "Usage: %s "_USAGE, g.prog);
	fprintf(stderr, "Try '%s --help' for more information\n", g.prog);
	exit(1);
}

static void help(void)
{
	fprintf(stdout, "Usage: %s "_USAGE, g.prog);
	fprintf(stdout, "Help:\n"_HELP);
}

static void version(void)
{
	fprintf(stdout, "%s version %s\n", g.prog, VERSION);
}

static int get_msr_val(unsigned int msr, unsigned long long *val)
{
	off64_t off;
	int err;

	*val = 0;
	off = lseek64(g.fd, (off64_t) msr, SEEK_SET);
	if (off == (off_t) -1) {
		perror("invalid MSR");
		return 1;
	}

	off = read(g.fd, val, 8);
	err = errno;
	if (off != 8) {
		fflush(stdout);
		fprintf(stderr,
			"could not read MSR 0x%8.8x (%s): %s\n",
			msr, get_reg_name(msr, g.msr_table), strerror(err));
		return 0;
	}

	return 0;
}

static int open_dev(int cpu)
{
	char s[20];

	snprintf(s, sizeof(s), "/dev/cpu/%d/msr", cpu);
	g.fd = open(s, O_RDONLY);
	if (g.fd < 0)
		fprintf(stderr, "could not open device %s: %s\n", s,
			strerror(errno));

	return g.fd;
}

#define PRINT_MSR(reg, val) \
do { \
	print_reg(reg, val, g.list, g.show_all, g.verbosity); \
} while(0)

static int _show_msr(struct reg_spec *reg)
{
	unsigned long long val;
	if (!g.list)
		if (get_msr_val(reg->address, &val))
			return 1;
	PRINT_MSR(reg, val);
	return 0;
}

static int show_matching(const char *name)
{
	int i, t;
	int n = MSR_MAX_LEN;
	int ret = 0;

	t = strlen(name);
	if (t<n)
		n = t;

	g.show_all = 1;
	for (i = 0; g.msr_table[i].name; i++) {
		t = strlen(g.msr_table[i].name);
		if (n<t)
			t = n;
		if (strncmp(name, g.msr_table[i].name, t) == 0)
			if (_show_msr(&(g.msr_table[i]))) {
				ret = 1;
				break;
			}
	}
	g.show_all = 0;
	return ret;
}

struct vendor_string {
	enum vendor id;
	const char *name;
} vendor_names[] = {
	{VENDOR_UNKNOWN, "(unknown)"},
	{VENDOR_AMD, "AMD"},
	{VENDOR_CENTAUR, "Centaur"},
	{VENDOR_CYRIX, "Cyrix"},
	{VENDOR_INTEL, "Intel"},
	{VENDOR_NATSEMI, "Natsemi"},
	{VENDOR_RISE, "Rise"},
	{VENDOR_TRANSMETA, "Transmeta"},
	{VENDOR_SIS, "SIS"},
};
get_name(vendor, enum vendor, vendor_names);

static void set_vendor(void)
{
	unsigned int b;
	cpuid(g.cpu, 0, NULL, &b, NULL, NULL);

	switch (b) {
	case 0x68747541:
		g.vendor = VENDOR_AMD;
		break;
	case 0x756e6547:
		g.vendor = VENDOR_INTEL;
		break;
	case 0x69727943:
		g.vendor = VENDOR_CYRIX;
		break;
	case 0x746e6543:
		g.vendor = VENDOR_CENTAUR;
		break;
	case 0x646f6547:
		g.vendor = VENDOR_NATSEMI;
		break;
	case 0x52697365:
	case 0x65736952:
		g.vendor = VENDOR_RISE;
		break;
	case 0x20536953:
		g.vendor = VENDOR_SIS;
		break;
	default:
		g.vendor = VENDOR_UNKNOWN;
		break;
	}
}

static void set_family_model(void)
{
	unsigned int a;
	cpuid(g.cpu, 0, &a, NULL, NULL, NULL);
	if (!g.family) {
		cpuid(g.cpu, 1, &a, NULL, NULL, NULL);
		g.family = (a >> 8) & 0xf;
		if (g.family == 0xf)
			g.family += (a >> 20) & 0xff;
	}

	if (!g.model) {
		cpuid(g.cpu, 1, &a, NULL, NULL, NULL);
		g.model = (a >> 4) & 0xf;
		if (g.model == 0xf)
			g.model |= (a >> 12) & 0xf0;
	}
}

static int set_msr_table(void)
{
	int supported = 0;

	set_vendor();
	set_family_model();

	if (g.vendor == VENDOR_AMD) {
		supported = 1;
		switch (g.family) {
		case 0x0f:
			g.msr_table = k8_spec;
			break;
		case 0x10:
			g.msr_table = fam10h_spec;
			break;
		default:
			g.msr_table = generic_msr_spec;
		}
	}

	if (g.vendor == VENDOR_INTEL) {
		supported = 1;
		g.msr_table = generic_msr_spec;
	}

	if (!supported) {
		fprintf(stdout, "CPU not (yet) supported "
			"(vendor=\"%s\", family=%d, model=%d)\n",
			get_vendor_name(g.vendor), g.family, g.model);
		return 1;
	}

	return 0;
}

#define OPT_MAX 32
int main(int argc, char *argv[])
{
	char c;
	int i, li, ret;
	struct reg_spec *reg;

	ret = 1;
	if((g.prog = rindex(argv[0], '/')))
		++g.prog;
	else
		g.prog = argv[0];

	if (!HaveCPUID()) {
		fprintf(stderr, "warning: no cpuid instruction available\n");
		fprintf(stdout, "no MSR information available for this CPU\n");
		return 0;
	}

	while((c = getopt_long(argc, argv, OPTSTRING, lopts, &li)) != -1) {
		switch (c) {
		case 'a':
			g.show_all = 1;
			break;
		case 'h':
			help();
			return 0;
		case 'v':
			version();
			return 0;
		case 'V':
			g.verbosity = strtol(optarg, NULL, 10);
			break;
		case 'c':
			g.cpu = strtol(optarg, NULL, 0);
			break;
		case 'm':
			g.model = strtol(optarg, NULL, 0);
			break;
		case 'f':
			g.family = strtol(optarg, NULL, 0);
			break;
		case 'l':
			g.list = 1;
			break;
		case 'r':
			if (isalpha(*optarg))
				g.msr_name = optarg;
			else
				g.reg = (unsigned int) strtol(optarg, NULL, 16);
			break;
		default:
			usage();
		}
	}

	if((optind > -1) && argv[optind]) {
		if ((argc - optind) != 1)
			usage();
		if (strlen(argv[optind]) >= OPT_MAX) {
			fprintf(stderr,"error: invlalid command line\n");
			goto out;
		}
		g.msr_search = argv[optind];
	}

	if (set_msr_table())
		goto out;

	if (g.msr_name) {
		g.reg = get_reg_addr(g.msr_name, g.msr_table);
		if (g.reg == (uint32_t) -1) {
			fflush(stdout);
			fprintf(stderr, "error: unknown register \"%s\"\n",
				g.msr_name);
			goto out;
		}
	}

	if (!g.list)
		if(open_dev(g.cpu) < 0)
			goto out;

	if (g.reg != (uint32_t) -1) {
		reg = get_reg_spec(g.reg, g.msr_table);
		if (!reg) {
			fflush(stdout);
			fprintf(stderr, "error: unknown MSR %x\n", g.reg);
			goto out;
		}
		if (_show_msr(reg))
			goto out;
	} else 	if (g.msr_search) {
		if (show_matching(g.msr_search))
			goto out;
	} else if (g.show_all) {
		for (i = 0; g.msr_table[i].name; i++)
			if (_show_msr(&(g.msr_table[i])))
				goto out;
	}
	ret = 0;

 out:
	if (g.fd >=0)
		close(g.fd);
	return ret;
}
