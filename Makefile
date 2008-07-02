CFLAGS = -Wall -W -Wshadow -g -O2
CC = gcc

SHELL = /bin/sh

all: x86info test

C_SRC =\
	AMD/identify.c\
	AMD/bluesmoke.c\
	AMD/MSR-Athlon.c\
	AMD/MSR-K6.c\
	AMD/powernow.c\
	AMD/dumppsb.c\
	AMD/bugs.c\
\
	Cyrix/identify.c\
\
	Intel/identify.c\
	Intel/info.c\
	Intel/bluesmoke.c\
	Intel/cachesize.c\
	Intel/eblcr.c\
	Intel/MSR-P4.c\
	Intel/MSR-P6.c\
	Intel/MSR-PM.c\
	Intel/microcode.c\
\
	IDT/identify.c\
	IDT/MSR-C3.c\
	IDT/longhaul.c\
	IDT/powersaver.c\
\
	NatSemi/identify.c\
\
	RiSE/identify.c\
\
	SiS/identify.c\
\
	x86info.c\
	havecpuid.c\
	cpuid.c\
	features.c\
	identify.c\
	rdmsr.c\
	binary.c\
	mptable.c\
	get_model_name.c\
	mtrr.c \
	connector.c\
\
	bench/benchmarks.c\
	bench/MHz.c

OBJS = $(C_SRC:%.c=%.o)

x86info: $(OBJS)
	$(CC) $(CFLAGS) -o x86info $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -MMD -o $@ -c $<
	@cp $*.d $*.P; \
	 sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	     -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(C_SRC:%.c=%.P)

.S.o:
	$(CC) $(CFLAGS) -o $@ -c $<

nodes:
	scripts/makenodes

test:
	scripts/testnodes

VERSION=1.21

release:
	git repack -a -d
	git-prune-packed
	git-archive --format=tar --prefix=x86info-$(VERSION)/ HEAD | gzip -9 > x86info-$(VERSION).tgz

clean:
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*~" -exec rm {} \;
	@find . -name "*.P" -exec rm {} \;
	@rm -f x86info x86info.exe

splint:
	splint +posixlib -badflag -fileextensions -type -nullassign -boolops -showcolumn -sysunrecog -fullinitblock -onlytrans -unrecog -usedef -statictrans -compdestroy -predboolint -predboolothers -D__`uname -m`__ $(C_SRC)

sparse:
	sparse $(C_SRC)
