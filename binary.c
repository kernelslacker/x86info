#include <stdio.h>

void long2binstr(long value) {
	int i;

	for(i=0;i<32;i++,value<<=1) {
		putchar( (1<<31 & value) ? '1' : '0' );

		if(i==23 || i==15 || i==7)
			putchar(' ');
	}
	putchar('\n');
}
