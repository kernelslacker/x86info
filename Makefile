CFLAGS = -Wall -W -Wshadow -g -O2
#CFLAGS += $(shell if $(CC) -m32 -S -o /dev/null -xc /dev/null >/dev/null 2>&1; then echo "-m32"; fi)

#CFLAGS += -mwin32 -DWIN32_LEAN_AND_MEAN
SHELL = /bin/sh

all: x86info test

OBJS =\
	AMD/identify.o\
	AMD/bluesmoke.o\
	AMD/MSR-Athlon.o\
	AMD/MSR-K6.o\
	AMD/powernow.o\
	AMD/bugs.o\
\
	Cyrix/identify.o\
\
	Intel/identify.o\
	Intel/info.o\
	Intel/bluesmoke.o\
	Intel/cachesize.o\
	Intel/eblcr.o\
	Intel/MSR-P4.o\
	Intel/MSR-P6.o\
	Intel/microcode.o\
\
	IDT/identify.o\
	IDT/MSR-C3.o\
\
	NatSemi/identify.o\
\
	RiSE/identify.o\
\
	x86info.o\
	havecpuid.o\
	cpuid.o\
	cpuid_UP.o\
	features.o\
	identify.o\
	rdmsr.o\
	binary.o\
	mptable.o\
	get_model_name.o\
	mtrr.o \
	connector.o\
\
	bench/MHz.o

x86info: $(OBJS)
	$(CC) $(CFLAGS) -o x86info $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.S.o:
	$(CC) $(CFLAGS) -o $@ -c $<

nodes:
	scripts/makenodes

test:
	scripts/testnodes

clean:
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*~" -exec rm {} \;
	@rm -f x86info x86info.exe

