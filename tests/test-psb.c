/*
 * Check that decode_psb() matches PSTs against the CPU's CPUID signature.
 */
#include <stdio.h>
#include <string.h>
#include <x86info.h>
#include <amd.h>
#include <powernow.h>

const double mobile_vid_table[32];
const double fid_codes[32];

static size_t add_pst(char *rom, size_t off, unsigned int sig,
		      unsigned char maxfid, unsigned char startvid)
{
	unsigned char hdr[8] = { sig & 0xff, (sig >> 8) & 0xff,
		(sig >> 16) & 0xff, (sig >> 24) & 0xff, 100, maxfid,
		startvid, 2 };
	unsigned char states[4] = { 4, 5, 6, 7 };

	memcpy(rom + off, hdr, sizeof(hdr));
	memcpy(rom + off + sizeof(hdr), states, sizeof(states));
	return off + sizeof(hdr) + sizeof(states);
}

int main(void)
{
	static char rom[512];
	struct cpudata cpu;
	size_t off = 64;
	int ret, fail = 0;

	memset(&cpu, 0, sizeof(cpu));
	cpu.family = 6;
	cpu.model = 8;
	cpu.stepping = 1;

	memcpy(rom + off, "AMDK7PNOW!", 10);
	rom[off + 10] = 0x12;
	rom[off + 15] = 3;
	off += 16;
	off = add_pst(rom, off, 0x6a0, 0x10, 0x08);
	off = add_pst(rom, off, 0x681, 0x10, 0x08);
	off = add_pst(rom, off, 0x681, 0x12, 0x08);

	ret = decode_psb(&cpu, 0x10, 0x08, rom, sizeof(rom), 0);
	if (ret != 1) {
		fprintf(stderr, "matching PST: got %d, want 1\n", ret);
		fail = 1;
	}

	ret = decode_psb(&cpu, 0x10, 0x08, rom, 100, 0);
	if (ret != 0) {
		fprintf(stderr, "truncated ROM: got %d, want 0\n", ret);
		fail = 1;
	}

	ret = decode_psb(&cpu, 0x10, 0x08, rom, 32, 0);
	if (ret != -1) {
		fprintf(stderr, "no PSB: got %d, want -1\n", ret);
		fail = 1;
	}
	return fail;
}
