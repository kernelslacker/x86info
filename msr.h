/*
 * Author: Andreas Herrmann <andreas.herrmann3@amd.com>
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 *
 * Licensed under the terms of the GNU GENERAL PUBLIC LICENSE version 2.
 * See file COPYING for details.
 */

#ifndef _msr_h
#define _msr_h


#define _RANGE(name, args...) \
unsigned char name##_range[] = { args }

#define _NAMES(name, args...) \
const char *name##_spec[] = { args }

#define _SPEC(addr, name, desc, prefix) \
{addr, #name, desc, prefix##name##_range, prefix##name##_spec}

#define MSR_MAX_LEN 32
struct reg_spec {
	unsigned int address;
	const char *name;
	const char *desc;
	unsigned char *bits;
	const char **spec;
};

static struct reg_spec *get_reg_spec(uint32_t msr, struct reg_spec *table)
{
	int i;
	for (i = 0; table[i].name; i++)
		if (msr == table[i].address)
			return &(table[i]);
	return NULL;
}

static uint32_t get_reg_addr(const char *name, struct reg_spec *table)
{
	int i;

	for (i = 0; table[i].name; i++)
		if (strcmp(name, table[i].name) == 0)
			return table[i].address;

	return -1;
}

static const char *get_reg_name(uint32_t reg, struct reg_spec *table)
{
	struct reg_spec *d;
	const char *s = NULL;

	d = get_reg_spec(reg, table);
	if (d)
		s = d->name;

	return s;
}

static void print_reg_bits(struct reg_spec *reg, uint64_t val, uint8_t list,
			   uint8_t verb)
{
	unsigned char *r;
	const char **d;
	int i, j, k;
	int first, any;
	uint64_t t;
	const char *s;

	if (!reg || !reg->bits || !reg->spec)
		return;

	any = 0;
	first = 1;
	r = reg->bits;
	d = reg->spec;
	for (i = 0, j = 0; r[i]; i++, j = k + 1){
		k = j + r[i] - 1;
		if (d[i] == 0) {
			if (verb == 2 || verb == 4)
				s = "res";
			else /* hide reserved fields */
				continue; 
		} else
			s = d[i];

		if (verb > 2)
			fprintf(stdout, "\n  ");
		else
			fprintf(stdout, "%s", first ? " (" : ",  ");

		if (list)
			printf("%d-%d:%s", j, k, s);
		else {
			if (r[i] == 64)
				t = val;
			else
				t = (val >> j) & ((1ULL<<r[i]) - 1);

			fprintf(stdout, "%s=%#llx", s, (unsigned long long) t);
		}
		first = 0;
		any = 1;
	}
	if (any && verb <= 2)
		fprintf(stdout, ")");
}

static void print_reg(struct reg_spec *reg, uint64_t val, uint8_t list,
		      uint8_t all, uint8_t verb)
{
	if (list) {
		if ((verb == 0 || verb == 3 || verb ==4) && reg->desc)
			fprintf(stdout, "%-*s: 0x%8.8x; %s",  all ? 20 : 0,
				reg->name, reg->address, reg->desc);
		else
			fprintf(stdout, "%-*s: 0x%8.8x",  all ? 20 : 0,
				reg->name, reg->address);

	} else
		fprintf(stdout, "%-*s = 0x%16.16llx", all ? 20 : 0,
			reg->name, val);

	if (verb)
		print_reg_bits(reg, val, list, verb);

	fprintf(stdout, "\n");
}

#endif /* _msr_h */
