#include <stdio.h>

void long2binstr(long value) {
	int i;

	for(i=0;i<32;i++) {
		if(1<<31 & value)
			printf("1");
		else
			printf("0");

		value = value<<1;
    
		if(i==23 || i==15 || i==7)
			printf(" ");
	}
	printf("\n");
}
