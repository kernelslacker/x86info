CFLAGS = -Wall -W -g
all: x86info

OBJS =\
	x86info.o\
	cpuid.o\
	cpuid_UP.o\
	features.o\
	cpuid-amd.o\
	cpuid-intel.o\
	cpuid-cyrix.o\
	cpuid-idt.o\
	cpuid-rise.o\
	identify.o\
	dumpregs.o\
	rdmsr.o\
	binary.o

x86info: $(OBJS)
	gcc -o x86info $(OBJS)

clean:
	rm -f *.o x86info
