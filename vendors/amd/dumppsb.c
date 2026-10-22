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
	u8 signature[10];
	u8 tableversion;
	u8 flags;
	u16 settlingtime;
	u8 reserved1;
	u8 numpst;
};

struct pst_s {
	u32 cpuid;
	u8 fsbspeed;
	u8 maxfid;
	u8 startvid;
	u8 numpstates;
};

/* These are read straight from /dev/mem: the layout must have no padding. */
_Static_assert(sizeof(struct psb_s) == 16, "psb_s layout");
_Static_assert(sizeof(struct pst_s) == 8, "pst_s layout");


void dump_PSB(struct cpudata *cpu, unsigned int maxfid, unsigned int startvid)
{
	int fd, i;
	char *map, *p, *end;

	fd = open("/dev/mem", O_RDONLY);
	if (fd == -1) {
		perror("/dev/mem");
		return;
	}

	map = p = mmap(NULL, ROMSIZE, PROT_READ, MAP_SHARED, fd, START);
	if (map == (void *) MAP_FAILED) {
		perror("mmap() error");
		if (close(fd) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}
		return;
	}

	end = map + ROMSIZE;

	for (i = 0; i <= ROMSIZE - PSB_SIGLEN; i++) {
		struct psb_s *psb;
		struct pst_s *pst;
		int numpstates, j, t;
		unsigned int fid, vid;

		if (memcmp(p, "AMDK7PNOW!", PSB_SIGLEN) == 0) {
			printf("Found PSB header at %p\n", p);
			if (end - p < (long) sizeof(struct psb_s)) {
				printf("PSB header runs past the end of the ROM\n");
				goto out;
			}
			psb = (struct psb_s *) p;
			printf("Table version: 0x%x\n", (unsigned int) psb->tableversion);
			if (psb->tableversion != 0x12) {
				printf("Sorry, only v1.2 tables supported right now\n");
				goto out;
			}

			printf("Flags: 0x%x ", (unsigned int) psb->flags);
			if ((psb->flags & 1) == 0)
				printf("(Mobile voltage regulator)\n");
			else
				printf("(Desktop voltage regulator)\n");

			printf("Settling Time: %d microseconds.\n",
				(int) psb->settlingtime);
			printf("Has %d PST tables. (Only dumping ones relevant to this CPU).\n",
				(int) psb->numpst);

			p += sizeof (struct psb_s);

			for (t = 0 ; t < psb->numpst; t++) {
				if (end - p < (long) sizeof(struct pst_s)) {
					printf("PST table runs past the end of the ROM\n");
					goto out;
				}
				pst = (struct pst_s *) p;
				numpstates = pst->numpstates;
				if (end - p - (long) sizeof(struct pst_s) < 2L * numpstates) {
					printf("PST table runs past the end of the ROM\n");
					goto out;
				}

				if ((etuple(cpu) == pst->cpuid) && (maxfid == pst->maxfid) && (startvid == pst->startvid)) {
					printf(" PST:%d (@%p)\n", t, pst);
					printf("  cpuid: 0x%x\t", pst->cpuid);
					printf("  fsb: %d\t", (int) pst->fsbspeed);
					printf("  maxFID: 0x%x\t", (unsigned int) pst->maxfid);
					printf("  startvid: 0x%x\n", (unsigned int) pst->startvid);
					printf("  num of p states in this table: %d\n", numpstates);

					p = (char *) pst + sizeof (struct pst_s);
					for (j = 0 ; j < numpstates; j++) {
						fid = (unsigned char)*p++ & 0x1f;
						printf("    FID: 0x%x (%.1fx [%.0fMHz])\t", fid,
							fid_codes[fid], pst->fsbspeed * fid_codes[fid]);

						vid = (unsigned char)*p++ & 0x1f;
						printf("VID: 0x%x (%0.3fV)\n", vid, mobile_vid_table[vid]);
					}
					printf("\n");
				} else {
					p = (char *) pst + sizeof (struct pst_s);
					for (j = 0 ; j < numpstates; j++)
						p += 2;
				}
			}
			goto out;
		}
		p++;
	}

out:
	if (munmap(map, ROMSIZE) == -1) {
		perror("munmap");
		exit(EXIT_FAILURE);
	}

	if (close(fd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	return;
}
