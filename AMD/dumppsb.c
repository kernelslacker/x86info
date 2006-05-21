#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/mman.h>

#include "powernow.h"
#include "../x86info.h"

#define START 0x000c0000L
#define END   0x000ffff0L
#define ROMSIZE (END-START)+1

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


void dump_PSB(struct cpudata *cpu, unsigned int maxfid, unsigned int startvid)
{
	int fd, i, j;
	char *p;
	struct psb_s *psb;
	struct pst_s *pst;
	int numpstates;
	unsigned int fid, vid;

	fd=open("/dev/mem", O_RDONLY);
	if (fd==-1) {
		perror("/dev/mem");
		return;
	}

	p = mmap(NULL, ROMSIZE, PROT_READ, MAP_SHARED, fd, START);
	if (p == (void *)-1) {
		perror("mmap() error");
		if (close(fd)==-1) {
			perror("close");
			exit(EXIT_FAILURE);
		}
		return;
	}

	for (i=0;i<ROMSIZE;i++) {
		if (memcmp(p, "AMDK7PNOW!",  10) == 0){
			printf ("Found PSB header at %p\n", p);
			psb = (struct psb_s *) p;
			printf ("Table version: 0x%x\n", (unsigned int)psb->tableversion);
			if (psb->tableversion != 0x12) {
				printf ("Sorry, only v1.2 tables supported right now\n");
				goto out;
			}

			printf ("Flags: 0x%x ", (unsigned int) psb->flags);
			if ((psb->flags & 1)==0) {
				printf ("(Mobile voltage regulator)\n");
			} else {
				printf ("(Desktop voltage regulator)\n");
			}

			printf ("Settling Time: %d microseconds.\n",
				(int)psb->settlingtime);
			printf ("Has %d PST tables. (Only dumping ones relevant to this CPU).\n",
				(int)psb->numpst);

			p += sizeof (struct psb_s);

			pst = (struct pst_s *) p;

			for (i = 0 ; i <psb->numpst; i++) {
				pst = (struct pst_s *) p;
				numpstates = pst->numpstates;

				if ((etuple(cpu) == pst->cpuid) && (maxfid==pst->maxfid) && (startvid==pst->startvid))
				{
					printf (" PST:%d (@%p)\n", i, pst);
					printf ("  cpuid: 0x%x\t", pst->cpuid);
					printf ("  fsb: %d\t", (int)pst->fsbspeed);
					printf ("  maxFID: 0x%x\t", (unsigned int)pst->maxfid);
					printf ("  startvid: 0x%x\n", (unsigned int)pst->startvid);
					printf ("  num of p states in this table: %d\n", numpstates);

					p = (char *) pst + sizeof (struct pst_s);
					for (j=0 ; j < numpstates; j++) {
						fid = *p++;
						printf ("    FID: 0x%x (%.1fx [%.0fMHz])\t", fid,
							fid_codes[fid], pst->fsbspeed * fid_codes[fid]);

						vid = *p++;
						printf ("VID: 0x%x (%0.3fV)\n", vid, mobile_vid_table[vid]);
					}
					printf ("\n");
				} else {
					p = (char *) pst + sizeof (struct pst_s);
					for (j=0 ; j < numpstates; j++)
						p+=2;
				}
			}
			goto out;
		}
		p++;
	}

out:
	if (munmap(p, ROMSIZE)==-1) {
		perror("munmap");
		exit(EXIT_FAILURE);
	}

	if (close(fd)==-1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
	return;
}

