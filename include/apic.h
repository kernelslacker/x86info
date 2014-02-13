#ifndef _APIC_H
#define _APIC_H

#define IA32_APIC_BASE_MSR 		0x1B

#define APIC_REGISTER_SPACE 		4096
#define APIC_REGISTER_ALIGNED_SIZE 	4

#define PAGE_OFFSET(addr) 		((unsigned long)(addr) & (getpagesize() - 1))

enum apic_register_offset
{
	APIC_ID		= 0x2,
	APIC_VER 	= 0x3,
	APIC_TPR 	= 0x8,
	APIC_APR 	= 0x9,
	APIC_PPR 	= 0xA,
	APIC_EOI 	= 0xB,
	APIC_RRD 	= 0xC,
	APIC_LDR 	= 0xD,
	APIC_DFR 	= 0xE,
	APIC_SIVR 	= 0xF,
	APIC_ISR	= 0x10,
	APIC_TMR	= 0x18,
	APIC_IRR 	= 0x20,
	APIC_ESR	= 0x28,
	APIC_LVT 	= 0x2F,
	APIC_ICR0 	= 0x30,
	APIC_ICR1 	= 0x31,
	APIC_LVTT 	= 0x32,
	APIC_LVTTS 	= 0x33,
	APIC_LVTPC 	= 0x34,
	APIC_LVT0 	= 0x35,
	APIC_LVT1 	= 0x36,
	APIC_LVTER 	= 0x37,
	APIC_TICR 	= 0x38,
	APIC_TCCR 	= 0x39,
	APIC_TDCR 	= 0x3E,
};

#include <fcntl.h>
#include <sys/mman.h>

#endif /* _APIC_H */
