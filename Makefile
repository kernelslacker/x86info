CFLAGS = -Wall -W
all: x86info

OBJS =\
	x86info.o\
	cpuid.o\
	features.o\
	cpuid-amd.o\
	cpuid-intel.o\
	cpuid-cyrix.o\
	cpuid-idt.o

x86info: $(OBJS)
	gcc -o x86info $(OBJS)

clean:
	rm -f *.o x86info
