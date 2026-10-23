/*
 * Check that the MP table scanner rejects malformed tables, using a
 * file-backed image of low physical memory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x86info.h>
#include "mptable.h"

#define IMAGE_SIZE	0x100000
#define FPS_ADDR	0xf0100
#define TABLE_ADDR	0x9000

struct options opts;

static unsigned char image[IMAGE_SIZE];

static void fix_checksum(unsigned char *p, size_t len, size_t at)
{
	unsigned int sum = 0;
	size_t i;

	p[at] = 0;
	for (i = 0; i < len; i++)
		sum += p[i];
	p[at] = -sum;
}

/* Build a floating pointer and a table with the given entries. */
static void build(const char *sig, unsigned int declared_len,
		  unsigned int entry_count, const unsigned char *entries,
		  size_t entries_len)
{
	unsigned char *fps = image + FPS_ADDR, *t = image + TABLE_ADDR;
	size_t len = 44 + entries_len;

	memset(image, 0, sizeof(image));
	memcpy(fps, "_MP_", 4);
	fps[4] = TABLE_ADDR & 0xff;
	fps[5] = TABLE_ADDR >> 8;
	fps[8] = 1;
	fps[9] = 4;
	fix_checksum(fps, 16, 10);

	memcpy(t, sig, 4);
	t[4] = declared_len & 0xff;
	t[5] = declared_len >> 8;
	t[6] = 4;
	t[34] = entry_count & 0xff;
	memcpy(t + 44, entries, entries_len);
	fix_checksum(t, declared_len < len ? declared_len : len, 7);
}

static int scan(int *ncpu)
{
	FILE *f = tmpfile();
	int ret;

	if (!f || fwrite(image, 1, sizeof(image), f) != sizeof(image)) {
		perror("tmpfile");
		exit(2);
	}
	fflush(f);
	ret = mptable_scan_fd(fileno(f), 1, ncpu);
	fclose(f);
	return ret;
}

static int fail;

static void check(const char *what, int ret, int ncpu, int want_ret,
		  int want_ncpu)
{
	if (ret != want_ret || ncpu != want_ncpu) {
		fprintf(stderr, "%s: got ret %d ncpu %d, want %d and %d\n",
			what, ret, ncpu, want_ret, want_ncpu);
		fail = 1;
	}
}

int main(void)
{
	unsigned char entries[20 * 2 + 8] = { 0 };
	unsigned char bad_type[20] = { 0 };
	int ret, ncpu;

	entries[20] = 0;	/* second processor */
	entries[40] = 1;	/* bus */

	build("PCMP", 44 + sizeof(entries), 3, entries, sizeof(entries));
	ret = scan(&ncpu);
	check("valid table", ret, ncpu, 1, 2);

	build("NOPE", 44, 1, entries, 20);
	ret = scan(&ncpu);
	check("bad signature", ret, ncpu, 0, 0);

	build("PCMP", 44, 1, entries, 20);
	ret = scan(&ncpu);
	check("entry beyond declared length", ret, ncpu, 0, 0);

	build("PCMP", 44 + sizeof(entries), 3, entries, sizeof(entries));
	image[TABLE_ADDR + 44 + 3] ^= 1;
	ret = scan(&ncpu);
	check("bad table checksum", ret, ncpu, 0, 0);

	build("PCMP", 44 + sizeof(entries), 3, entries, sizeof(entries));
	image[FPS_ADDR + 11] ^= 1;
	ret = scan(&ncpu);
	check("bad floating pointer checksum", ret, ncpu, 0, 0);

	bad_type[0] = 9;
	build("PCMP", 44 + 20, 1, bad_type, 20);
	ret = scan(&ncpu);
	check("unknown entry type", ret, ncpu, 0, 0);

	build("PCMP", 44 + sizeof(entries), 4, entries, sizeof(entries));
	ret = scan(&ncpu);
	check("entry count past table", ret, ncpu, 0, 0);

	return fail;
}
