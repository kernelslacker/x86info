/*
 *  (C) 2011 Andre Nogueira
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  APIC register dumping.
 */

#include <stdio.h>
#include <unistd.h>
#include <apic.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <x86info.h>

/* The mapped local APIC register page. */
struct apic_map {
	void *base;
	size_t size;
	volatile unsigned int *regs;
};

/* Map the APIC registers at physical address addr; 0 on success. */
static int map_apic_registers(struct apic_map *map, unsigned long long addr)
{
	long pagesize = sysconf(_SC_PAGESIZE);
	unsigned long long offset;
	int fd;

	if (pagesize <= 0)
		return -1;
	offset = addr & ((unsigned long long)pagesize - 1);

	fd = open("/dev/mem", O_RDONLY);
	if (fd == -1)
		return -1;

	map->size = APIC_REGISTER_SPACE + (size_t)offset;
	map->base = mmap(NULL, map->size, PROT_READ, MAP_SHARED, fd,
			 (off_t)(addr - offset));
	close(fd);
	if (map->base == MAP_FAILED)
		return -1;

	map->regs = (volatile unsigned int *)((char *)map->base + offset);
	return 0;
}


/* Registers sit on 16-byte boundaries, so offset * 4 indexes the unsigned
 * int array; the highest (APIC_TDCR) is at byte 992 of the 4096-byte page. */
static void dump_apic(const volatile unsigned int *apic_reg_buffer,unsigned int offset)
{
	printf("0x%08x\n", apic_reg_buffer[ (offset * APIC_REGISTER_ALIGNED_SIZE) ]);
}


struct apic_reg {
	const char *label;
	unsigned int offset;
};

static const struct apic_reg apic_regs_head[] = {
	{ "APIC Local ID", APIC_ID },
	{ "APIC Local Version", APIC_VER },
	{ "APIC Task Priority", APIC_TPR },
	{ "APIC Processor Priority", APIC_PPR },
	{ "APIC Logical Destination", APIC_LDR },
	{ "APIC Destination Format", APIC_DFR },
	{ "APIC Spurious Interrupt Vector", APIC_SIVR },
};

/* Eight consecutive 32-bit registers each; the label is a prefix. */
static const struct apic_reg apic_regs_bitmaps[] = {
	{ "APIC In-Service (ISR", APIC_ISR },
	{ "APIC Trigger Mode (TMR", APIC_TMR },
	{ "APIC Interrupt Request (IRR", APIC_IRR },
};

static const struct apic_reg apic_regs_tail[] = {
	{ "APIC Error Status", APIC_ESR },
	{ "APIC LVT CMCI", APIC_LVT },
	{ "APIC Interrupt Command (ICR0)", APIC_ICR0 },
	{ "APIC Interrupt Command (ICR1)", APIC_ICR1 },
	{ "APIC LVT Timer", APIC_LVTT },
	{ "APIC Thermal Sensor", APIC_LVTTS },
	{ "APIC LVT Performance Monitoring Counters", APIC_LVTPC },
	{ "APIC LVT LINT0", APIC_LVT0 },
	{ "APIC LVT LINT1", APIC_LVT1 },
	{ "APIC LVT Error", APIC_LVTER },
	{ "APIC Initial Count (for Timer)", APIC_TICR },
	{ "APIC Current Count (for Timer)", APIC_TCCR },
	{ "APIC Divide Configuration (for Timer)", APIC_TDCR },
};

#define APIC_LABEL_WIDTH 42

static void dump_apic_regs(const volatile unsigned int *apic_reg_buffer,
			   const struct apic_reg *regs, unsigned int nr)
{
	unsigned int i;

	for (i = 0; i < nr; i++) {
		printf("%-*s: ", APIC_LABEL_WIDTH, regs[i].label);
		dump_apic(apic_reg_buffer, regs[i].offset);
	}
}

void dump_apics(const struct cpudata *cpu)
{
	struct apic_map map;
	unsigned long long base;
	unsigned int i, j;
	char label[64];

	if (!(cpu->flags_edx & (X86_FEATURE_APIC)))
		return;

	if (bind_cpu(cpu->number))
		return;

	if (read_msr(cpu->number, IA32_APIC_BASE_MSR, &base) != 1)
		return;

	/* APIC_BASE bit 11: enabled */
	if (!(base & (1ULL << 11)))
		return;

	/* APIC_BASE bit 10: x2APIC mode, where the MMIO register page is disabled. */
	if (base & (1ULL << 10)) {
		printf("APIC registers: x2APIC mode, MMIO dump not available\n");
		return;
	}

	if (map_apic_registers(&map, (base >> 12) << 12))
		return;

	printf("APIC registers:\n");

	snprintf(label, sizeof(label), "APIC MSR Base(0x%02x)", IA32_APIC_BASE_MSR);
	printf("%-*s: 0x%016llx\n", APIC_LABEL_WIDTH, label, base);

	dump_apic_regs(map.regs, apic_regs_head, ARRAY_SIZE(apic_regs_head));

	for (j = 0; j < ARRAY_SIZE(apic_regs_bitmaps); j++) {
		for (i = 0; i < 8; i++) {
			snprintf(label, sizeof(label), "%s%u)", apic_regs_bitmaps[j].label, i);
			printf("%-*s: ", APIC_LABEL_WIDTH, label);
			dump_apic(map.regs, apic_regs_bitmaps[j].offset + i);
		}
	}

	dump_apic_regs(map.regs, apic_regs_tail, ARRAY_SIZE(apic_regs_tail));

	munmap(map.base, map.size);

	printf("\n");
}
