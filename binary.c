#include <stdio.h>

void binary (unsigned int n, unsigned long value)
{
	unsigned int i;

	for(i=0; i<n; i++, value<<=1)
		(void)putchar( (1<<(n-1) & value) ? '1' : '0' );
	(void)putchar('\n');
}

void binary32 (unsigned long value)
{
	int i;

	for(i=0;i<32;i++,value<<=1) {
		(void)putchar( (1<<31 & value) ? '1' : '0' );

		if(i==23 || i==15 || i==7)
			(void)putchar(' ');
	}
	(void)putchar('\n');
}

void binary64(unsigned long long value)
{
	binary32 (value>>32);
	printf ("           ");
	binary32 (value);
}
