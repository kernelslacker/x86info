/*
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <x86info.h>

#ifndef __x86_64__
static int flag_is_changeable_p(unsigned long flag)
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
#endif

int HaveCPUID(void)
{
#ifdef __x86_64__
	/*
	 * Long mode requires CPUID. The pushf/push sequence in
	 * flag_is_changeable_p writes under %rsp, which can clobber
	 * red zone data in a leaf function.
	 */
	return 1;
#else
	return flag_is_changeable_p(0x200000);
#endif
}

