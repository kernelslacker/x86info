CFLAGS = -Wall -W -g -O2

all: x86info

OBJS =\
	AMD/identify.o\
	AMD/bluesmoke.o\
\
	Cyrix/identify.o\
\
	Intel/identify.o\
	Intel/bluesmoke.o\
\
	IDT/identify.o\
\
	RiSE/identify.o\
\
	x86info.o\
	havecpuid.o\
	cpuid.o\
	cpuid_UP.o\
	features.o\
	identify.o\
	dumpregs.o\
	rdmsr.o\
	binary.o\
	MHz.o\
	eblcr.o

x86info: $(OBJS)
	gcc -o x86info $(OBJS)

clean:
	find . -name "*.o" -exec rm {} \;
	rm -f x86info

