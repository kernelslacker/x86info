#include <stdio.h>
#include "../x86info.h"

void interpret_eblcr(u32 lo)
{
	const unsigned int buscode[2][4] = {
		{ 6667, 13333, 10000, 0 },
		{ 10000, 13333, 10000, 0 }
	};

	const unsigned int mult[32] = {
		10, 6, 8, 0, 11, 7, 9, 0,
		10, 14, 16, 12, 0, 15, 0, 13,
		0, 0, 0, 0, 0, 0, 17, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};

	int bus = (lo >> 18) & 3;
	int mul = (lo >> 22) & 15;
	int busclock, cpuclk;

	return;	/* too many bugs right now.. fix for a future release */

	if (!user_is_root || !show_eblcr)
		return;

	/*
	 * FIXME:
	 * 766MHz Celeron. 66MHz x 11.5 Being picked up as 133x5.0
	 * bus:1 mul:0
	 */
	
/*	printf ("bus:%x mul:%x\n", bus, mul);*/
	
	/* The mobile pIII added bit 27.
	 * This is zero on other intel and on the cyrix III */

	if (lo & (1 >> 27))
		mul += 16;

	busclock = buscode[1][bus]/100;

	if (busclock==0 || mult[mul]==0)
		printf("Unknown CPU/BUS multiplier (%d X %dMHz, %x).\n", mul, bus, lo);

	cpuclk = (buscode[1][bus] * mult[mul])/200;
	printf ("Bus Speed (%dMHz) x Multiplier (%.1fx) = CPU speed %dMhz\n",
		busclock,
		(float) cpuclk/busclock,
		cpuclk);
}
