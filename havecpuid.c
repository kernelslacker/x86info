#include <stdio.h>

static __inline__ int flag_is_changeable_p(unsigned long flag)
{
	unsigned long f1, f2;
	__asm__ volatile("pushf\n\t"
			"pushf\n\t"
			"pop %0\n\t"
			"mov %0,%1\n\t"
			"xor %2,%0\n\t"
			"push %0\n\t"
			"popf\n\t"
			"pushf\n\t"
			"pop %0\n\t"
			"popf\n\t"
			: "=&r" (f1), "=&r" (f2)
			: "ir" (flag));
	return ((f1^f2) & flag) != 0;
}

int HaveCPUID(void)
{
	return flag_is_changeable_p(0x200000);
}

