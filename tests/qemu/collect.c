/* Freestanding i486 code, including for models unable to boot x86-64 Linux.
 * Only CPUID is executed; no advertised SIMD or other optional instructions
 * are needed. The serial stream uses the existing replay dump format.
 */
static void out(unsigned short port, unsigned char value)
{
	__asm__ volatile("outb %0, %1" : : "a" (value), "d" (port));
}

static void put(const char *s)
{
	while (*s)
		out(0x3f8, *s++);
}

static void hex(unsigned int n)
{
	int shift;

	for (shift = 28; shift >= 0; shift -= 4)
		out(0x3f8, "0123456789abcdef"[(n >> shift) & 15]);
}

static unsigned int dump(unsigned int leaf, unsigned int sub)
{
	unsigned int a, b, c, d;

	__asm__ volatile("cpuid" : "=a" (a), "=b" (b), "=c" (c), "=d" (d)
			 : "a" (leaf), "c" (sub));
	put("eax in: 0x"); hex(leaf);
	put(", ecx in: 0x"); hex(sub);
	put(", eax = "); hex(a);
	put(" ebx = "); hex(b);
	put(" ecx = "); hex(c);
	put(" edx = "); hex(d);
	put("\n");
	return a;
}

static void range(unsigned int base)
{
	unsigned int max, leaf, sub;

	max = dump(base, 0);
	if ((max & 0xffff0000) != base)
		return;
	if (max - base > 255)
		max = base + 255;
	for (leaf = base + 1; leaf <= max; leaf++) {
		dump(leaf, 0);
		switch (leaf) {
		case 4: case 7: case 0xb: case 0xd: case 0xf: case 0x10:
		case 0x12: case 0x14: case 0x17: case 0x18: case 0x1d:
		case 0x1f: case 0x20: case 0x23: case 0x24:
		case 0x8000001d: case 0x80000020: case 0x80000026:
			/* Include terminating and zero subleaves too. The replay
			 * backend and raw dumper decide which ones to display.
			 */
			for (sub = 1; sub < 64; sub++)
				dump(leaf, sub);
			break;
		default:
			break;
		}
	}
}

void collect(void);
void collect(void)
{
	out(0x3fb, 0x80);
	out(0x3f8, 1);
	out(0x3f9, 0);
	out(0x3fb, 3);
	put("CPU #0:\n");
	range(0);
	range(0x40000000);
	range(0x80000000);
	range(0xc0000000);
	put("QEMU CPUID DONE\n");
}
