#include <stdio.h>

static __inline__ int flag_is_changeable_p(unsigned int flag)
{
	unsigned int f1, f2;
	__asm__ volatile("pushfl\n\t"
			"pushfl\n\t"
			"popl %0\n\t"
			"movl %0,%1\n\t"
			"xorl %2,%0\n\t"
			"pushl %0\n\t"
			"popfl\n\t"
			"pushfl\n\t"
			"popl %0\n\t"
			"popfl\n\t"
			: "=&r" (f1), "=&r" (f2)
			: "ir" (flag));
	return ((f1^f2) & flag) != 0;
}

int HaveCPUID(void)
{
	return flag_is_changeable_p(0x200000);
}

