#pragma once

static inline unsigned long long int rdtsc(void)
{
	unsigned int low, high;

	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
	return ((unsigned long long int)high << 32) | low;
}
