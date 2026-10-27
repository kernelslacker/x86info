/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  PSB decoding routines.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/mman.h>

#include <x86info.h>
#include "amd.h"
#include "powernow.h"

#define START 0x000c0000L
#define END   0x000ffff0L
#define ROMSIZE ((END-START)+1)

#define PSB_SIGLEN 10

struct psb_s {
	uint8_t signature[10];
	uint8_t tableversion;
	uint8_t flags;
	uint16_t settlingtime;
	uint8_t reserved1;
	uint8_t numpst;
};

struct pst_s {
	uint32_t cpuid;
	uint8_t fsbspeed;
	uint8_t maxfid;
	uint8_t startvid;
	uint8_t numpstates;
};

/* These are read straight from /dev/mem: the layout must have no padding. */
_Static_assert(sizeof(struct psb_s) == 16, "psb_s layout");
_Static_assert(sizeof(struct pst_s) == 8, "pst_s layout");


/*
 * Print the PSB and the PSTs matching this CPU in the ROM image rom, which
 * was read from physical address base. Returns the number of PSTs printed,
 * or -1 if the image holds no usable PSB.
 */
int decode_psb(struct cpudata *cpu, unsigned int maxfid, unsigned int startvid,
	       const char *rom, size_t len, unsigned long base)
{
	const char *p = rom, *end = rom + len;
	const struct psb_s *psb;
	const struct pst_s *pst;
	unsigned int fid, vid;
	int numpstates, j, t, matched = 0;

	for (; (size_t)(end - p) >= PSB_SIGLEN; p++)
		if (memcmp(p, "AMDK7PNOW!", PSB_SIGLEN) == 0)
			break;
	if ((size_t)(end - p) < PSB_SIGLEN)
		return -1;

	printf("Found PSB header at 0x%lx\n", base + (unsigned long)(p - rom));
	if (end - p < (long) sizeof(struct psb_s)) {
		printf("PSB header runs past the end of the ROM\n");
		return -1;
	}
	psb = (const struct psb_s *) p;
	printf("Table version: 0x%x\n", (unsigned int) psb->tableversion);
	if (psb->tableversion != 0x12) {
		printf("Sorry, only v1.2 tables supported right now\n");
		return -1;
	}

	printf("Flags: 0x%x ", (unsigned int) psb->flags);
	if ((psb->flags & 1) == 0)
		printf("(Mobile voltage regulator)\n");
	else
		printf("(Desktop voltage regulator)\n");

	printf("Settling Time: %d microseconds.\n", (int) psb->settlingtime);
	printf("Has %d PST tables. (Only dumping ones relevant to this CPU).\n",
		(int) psb->numpst);

	p += sizeof (struct psb_s);

	for (t = 0 ; t < psb->numpst; t++) {
		if (end - p < (long) sizeof(struct pst_s)) {
			printf("PST table runs past the end of the ROM\n");
			return matched;
		}
		pst = (const struct pst_s *) p;
		numpstates = pst->numpstates;
		if (end - p - (long) sizeof(struct pst_s) < 2L * numpstates) {
			printf("PST table runs past the end of the ROM\n");
			return matched;
		}

		p += sizeof (struct pst_s);
		if (tuple(cpu) == pst->cpuid && maxfid == pst->maxfid &&
		    startvid == pst->startvid) {
			matched++;
			printf(" PST:%d (@0x%lx)\n", t, base +
			       (unsigned long)((const char *)pst - rom));
			printf("  cpuid: 0x%x\t", pst->cpuid);
			printf("  fsb: %d\t", (int) pst->fsbspeed);
			printf("  maxFID: 0x%x\t", (unsigned int) pst->maxfid);
			printf("  startvid: 0x%x\n", (unsigned int) pst->startvid);
			printf("  num of p states in this table: %d\n", numpstates);

			for (j = 0 ; j < numpstates; j++) {
				fid = (unsigned char)p[2 * j] & 0x1f;
				printf("    FID: 0x%x (%.1fx [%.0fMHz])\t", fid,
					fid_codes[fid], pst->fsbspeed * fid_codes[fid]);

				vid = (unsigned char)p[2 * j + 1] & 0x1f;
				printf("VID: 0x%x (%0.3fV)\n", vid, mobile_vid_table[vid]);
			}
			printf("\n");
		}
		p += 2 * numpstates;
	}
	return matched;
}

void dump_PSB(struct cpudata *cpu, unsigned int maxfid, unsigned int startvid)
{
	int fd;
	char *map;

	fd = open("/dev/mem", O_RDONLY);
	if (fd == -1) {
		perror("/dev/mem");
		return;
	}

	map = mmap(NULL, ROMSIZE, PROT_READ, MAP_SHARED, fd, START);
	if (map == (void *) MAP_FAILED) {
		perror("mmap() error");
		if (close(fd) == -1)
			perror("close");
		return;
	}

	decode_psb(cpu, maxfid, startvid, map, ROMSIZE, START);

	if (munmap(map, ROMSIZE) == -1)
		perror("munmap");

	if (close(fd) == -1)
		perror("close");
}
