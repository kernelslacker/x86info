/*
 * $Id: cachesize.c,v 1.2 2001/12/10 21:48:37 davej Exp $
 */

#include "../x86info.h"

void get_intel_cacheinfo (struct cpudata *c)
{
	unsigned long eax, ebx, ecx, edx;

	c->cachesize_L1_I = 0;
	c->cachesize_L1_D = 0;
	c->cachesize_L2 = 0;
	c->cachesize_L3 = 0;

	if (c->maxi > 1) {
		/* supports eax=2  call */
		int i, j, n;
		unsigned long regs[4];
		unsigned char *dp = (unsigned char *)regs;

		/* Number of times to iterate */
		cpuid (c->number, 2, &eax, &ebx, &ecx, &edx);
		n = eax & 0xFF;

		for ( i = 0 ; i < n ; i++ ) {
			cpuid (c->number, 2, &regs[0], &regs[1], &regs[2], &regs[3]);
			
			/* If bit 31 is set, this is an unknown format */
			for ( j = 0 ; j < 3 ; j++ )
				if ((regs[j] & (1<<31)) != 0)
					regs[j] = 0;

			/* Byte 0 is level count, not a descriptor */
			for ( j = 1 ; j < 16 ; j++ ) {
				unsigned char des = dp[j];
				unsigned char dl, dh;
				unsigned int cs;

				dh = des >> 4;
				dl = des & 0x0F;

				/* Black magic... */
				switch ( dh ) {
				case 0:
					switch ( dl ) {
					case 6:
						/* L1 I cache */
						c->cachesize_L1_I += 8;
						break;
					case 8:
						/* L1 I cache */
						c->cachesize_L1_I += 16;
						break;
					case 10:
						/* L1 D cache */
						c->cachesize_L1_D += 8;
						break;
					case 12:
						/* L1 D cache */
						c->cachesize_L1_D += 16;
						break;
					default:;
						/* TLB, or unknown */
					}
					break;
				case 2:
					if ( dl ) {
						/* L3 cache */
						cs = (dl-1) << 9;
						c->cachesize_L3 += cs;
					}
					break;
				case 4:
					if (c->family > 6 && dl ) {
						/* P4 family */
						/* L3 cache */
						cs = 128 << (dl-1);
						c->cachesize_L3 += cs;
						break;
					}
					/* else same as 8 - fall through */
				case 8:
					if ( dl ) {
						/* L2 cache */
						cs = 128 << (dl-1);
						c->cachesize_L2 += cs;
					}
					break;
				case 6:
					if (dl > 5) {
						/* L1 D cache */
						cs = 8<<(dl-6);
						c->cachesize_L1_D += cs;
					}
					break;
				case 7:
					if ( dl >= 8 ) {
						/* L2 cache */
						cs = 64<<(dl-8);
						c->cachesize_L2 += cs;
					} else {
						/* L0 I cache, count as L1 */
						cs = dl ? (16 << (dl-1)) : 12;
						c->cachesize_L1_I += cs;
					}
					break;
				default:
					/* TLB, or something else we don't know about */
					break;
				}
			}
		}
	}
}
