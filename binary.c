/*
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Binary output routines.
 */

#include <stdio.h>
#include <x86info.h>

/* Print the low n bits of value, most significant first; n is at most 64. */
static void print_bits(unsigned int n, unsigned long long value, int bytes)
{
	unsigned int i;

	for (i = n; i > 0; i--) {
		(void)putchar((value >> (i - 1)) & 1 ? '1' : '0');
		if (bytes && i > 1 && (i - 1) % 8 == 0)
			(void)putchar(' ');
	}
	(void)putchar('\n');
}

void binary(unsigned int n, unsigned long value)
{
	print_bits(n > 64 ? 64 : n, value, 0);
}

void binary32(unsigned long value)
{
	print_bits(32, value, 1);
}

void binary64(unsigned long long value)
{
	binary32(value>>32);
	printf("           ");
	binary32(value);
}
