CFLAGS = -Wall -W -Wshadow -g -O2

SHELL = /bin/sh

all: x86info test

OBJS =\
	AMD/identify.o\
	AMD/bluesmoke.o\
	AMD/MSR-Athlon.o\
	AMD/MSR-K6.o\
	AMD/powernow.o\
	AMD/dumppsb.o\
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
	Intel/MSR-PM.o\
	Intel/microcode.o\
\
	IDT/identify.o\
	IDT/MSR-C3.o\
	IDT/longhaul.o\
	IDT/powersaver.o\
\
	NatSemi/identify.o\
\
	RiSE/identify.o\
\
	SiS/identify.o\
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
	bench/benchmarks.o\
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

VERSION=1.20
	
release:
	git repack -a -d
	git-prune-packed
	git-tar-tree HEAD x86info-$(VERSION) | gzip -9 > x86info-$(VERSION).tgz

clean:
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*~" -exec rm {} \;
	@rm -f x86info x86info.exe

splint:
	splint +posixlib -badflag -fileextensions -type -nullassign -boolops -showcolumn -sysunrecog -fullinitblock -onlytrans -unrecog -usedef -statictrans -compdestroy -predboolint -predboolothers -D__`uname -m`__ *.c  AMD/*.c  Cyrix/*.c Intel/*.c NatSemi/*.c RiSE/*.c SiS/*.c IDT/*.c

