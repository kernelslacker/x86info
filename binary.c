#include <stdio.h>

void binary32 (unsigned long value)
{
	int i;

	for(i=0;i<32;i++,value<<=1) {
		putchar( (1<<31 & value) ? '1' : '0' );

		if(i==23 || i==15 || i==7)
			putchar(' ');
	}
	putchar('\n');
}

void binary64(unsigned long long value)
{
	binary32 (value>>32);
	binary32 (value);
}
