/*
 *  (C) 2011 Andre Nogueira
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  APIC register dumping.
 *  TODO : decode the registers too.
 */

#include <stdio.h>
#include <unistd.h>
#include <apic.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <x86info.h>

static int fd;
static void *apic_mapped_base;
static unsigned int apic_mapped_size;

static int apic_msr_value(int cpu, int msr, unsigned long long *val)
{
	if (read_msr(cpu, msr, val) == 1)
		return 1;
	else
		return 0;
}

static void dump_apic_base_msr(int cpu, int msr)
{
	unsigned long long val;
	int ret;

	ret = apic_msr_value(cpu, msr, &val);
	if (ret)
		printf("0x%016llx\n", val);
}

static int apic_is_enable(int cpu, int msr)
{
	unsigned long long val;
	int ret;

	ret = apic_msr_value(cpu, msr, &val);
	if (ret)
		ret = ((int)(val >> 11) & 0x1);

	return ret;
}

/* APIC_BASE bit 10: x2APIC mode, where the MMIO register page is disabled. */
static int apic_is_x2apic(int cpu, int msr)
{
	unsigned long long val;

	if (!apic_msr_value(cpu, msr, &val))
		return 0;
	return (val >> 10) & 0x1;
}

static unsigned long long apic_physical_address(int cpu, int msr)
{
	unsigned long long val;
	unsigned long long apic_phys_addr;
	int ret;

	apic_phys_addr = 0;

	ret = apic_msr_value(cpu, msr, &val);
	if (ret)
		apic_phys_addr = ((val >> 12) << 12);

	return apic_phys_addr;
}

static volatile unsigned int * mapping_apic_registers(unsigned long long addr)
{
	unsigned long long offset;
	void *mapped;

	if ((fd = open("/dev/mem", O_RDONLY)) == -1)
		return NULL;

	offset = PAGE_OFFSET(addr);
	apic_mapped_size = APIC_REGISTER_SPACE + (unsigned int)offset;

	mapped = mmap(NULL, apic_mapped_size, PROT_READ, MAP_SHARED, fd, (off_t)(addr - offset));
	if (mapped == MAP_FAILED) {
		close(fd);
		return NULL;
	}

	apic_mapped_base = mapped;
	return (volatile unsigned int *)((char *)mapped + offset);
}

static void unmapping_apic_registers(void)
{
	if (munmap(apic_mapped_base, apic_mapped_size) == -1)
		return;
	if (close(fd) == -1)
		return;
}


/* Registers sit on 16-byte boundaries, so offset * 4 indexes the unsigned
 * int array; the highest (APIC_TDCR) is at byte 992 of the 4096-byte page. */
static void dump_apic(const volatile unsigned int *apic_reg_buffer,unsigned int offset)
{
	printf("0x%08x\n", apic_reg_buffer[ (offset * APIC_REGISTER_ALIGNED_SIZE) ]);
}


void dump_apics(const struct cpudata *cpu)
{
	const volatile unsigned int *apic_reg_buffer;
	unsigned long long phys_addr;
	int i;

	/**
	 * Verify if apic feature is supported
	 **/
	if (!(cpu->flags_edx & (X86_FEATURE_APIC)))
		return;

	/**
	* Verify if apic is enable
	**/
	if (!(apic_is_enable(cpu->number, IA32_APIC_BASE_MSR)))
		return;

	if (apic_is_x2apic(cpu->number, IA32_APIC_BASE_MSR)) {
		printf("APIC registers: x2APIC mode, MMIO dump not available\n");
		return;
	}

	phys_addr = apic_physical_address(cpu->number, IA32_APIC_BASE_MSR);

	/**
	 * Mapping the apic registers
	 **/
	apic_reg_buffer = mapping_apic_registers(phys_addr);
	if (apic_reg_buffer == NULL)
		return;

	printf("APIC registers:\n");

	printf("APIC MSR Base(0x%02x): 			: ", IA32_APIC_BASE_MSR);
	dump_apic_base_msr(cpu->number, IA32_APIC_BASE_MSR);

	printf("APIC Local ID				: ");
	dump_apic(apic_reg_buffer, APIC_ID);
	printf("APIC Local Version			: ");
	dump_apic(apic_reg_buffer, APIC_VER);
	printf("APIC Task Priority			: ");
	dump_apic(apic_reg_buffer, APIC_TPR);
	printf("APIC Arbitration Priority		: ");
	dump_apic(apic_reg_buffer, APIC_APR);
	printf("APIC Processor Priority 		: ");
	dump_apic(apic_reg_buffer, APIC_PPR);
	printf("APIC EOI 				: ");
	dump_apic(apic_reg_buffer, APIC_EOI);
	printf("APIC Remote Read 			: ");
	dump_apic(apic_reg_buffer, APIC_RRD);
	printf("APIC Logical Destination 		: ");
	dump_apic(apic_reg_buffer, APIC_LDR);
	printf("APIC Destination Format 		: ");
	dump_apic(apic_reg_buffer, APIC_DFR);
	printf("APIC Spurious Interrupt Vector 		: ");
	dump_apic(apic_reg_buffer, APIC_SIVR);

	for (i = 0 ; i < 8 ; i++) {
		printf("APIC In-Service (ISR%d)	 		: ", i);
		dump_apic(apic_reg_buffer, APIC_ISR+i);
	}

	for (i = 0 ; i < 8 ; i++) {
		printf("APIC Trigger Mode (TMR%d)	 	: ", i);
		dump_apic(apic_reg_buffer, APIC_TMR+i);
	}

	for (i = 0 ; i < 8 ; i++) {
		printf("APIC Interrupt Request (IRR0%d)	 	: ", i);
		dump_apic(apic_reg_buffer, APIC_IRR+i);
	}

	printf("APIC Error Status 			: ");
	dump_apic(apic_reg_buffer, APIC_ESR);
	printf("APIC LVT CMCI 				: ");
	dump_apic(apic_reg_buffer, APIC_LVT);
	printf("APIC Interrupt Command (ICR0)		: ");
	dump_apic(apic_reg_buffer, APIC_ICR0);
	printf("APIC Interrupt Command (ICR1) 		: ");
	dump_apic(apic_reg_buffer, APIC_ICR1);
	printf("APIC LVT Timer 				: ");
	dump_apic(apic_reg_buffer, APIC_LVTT);
	printf("APIC Thermal Sensor 			: ");
	dump_apic(apic_reg_buffer, APIC_LVTTS);
	printf("APIC LVT Performance Monitoring Counters: ");
	dump_apic(apic_reg_buffer, APIC_LVTPC);
	printf("APIC LVT LINT0 				: ");
	dump_apic(apic_reg_buffer, APIC_LVT0);
	printf("APIC LVT LINT1 				: ");
	dump_apic(apic_reg_buffer, APIC_LVT1);
	printf("APIC LVT Error 				: ");
	dump_apic(apic_reg_buffer, APIC_LVTER);
	printf("APIC Initial Count (for Timer)		: ");
	dump_apic(apic_reg_buffer, APIC_TICR);
	printf("APIC Current Count (for Timer)		: ");
	dump_apic(apic_reg_buffer, APIC_TCCR);
	printf("APIC Divide Configuration (for Timer)	: ");
	dump_apic(apic_reg_buffer, APIC_TDCR);

	unmapping_apic_registers();

	printf("\n");
}
