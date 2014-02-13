VERSION=1.30

CFLAGS = -g -O2 -Werror -Wall -Wshadow -Wextra -Wmissing-declarations -Wdeclaration-after-statement -Wredundant-decls -DVERSION="$(VERSION)"
LDFLAGS = -Wl,-z,relro,-z,now

ifeq ($(CC),"")
CC = gcc
endif

SHELL = /bin/sh

all: x86info test lsmsr


LSMSR_TMP_HEADERS=AMD/k8.h AMD/fam10h.h AMD/fam11h.h AMD/fam12h.h \
	AMD/fam14h.h AMD/fam15h.h generic_msr.h

%.h: %.regs scripts/createheader.py
	python scripts/createheader.py $< `basename $< .regs` >$@

LSMSR_SRC = \
	lsmsr.c \
	cpuid.c \
	havecpuid.c

LSMSR_OBJS = $(LSMSR_SRC:%.c=%.o)

lsmsr.c: $(LSMSR_TMP_HEADERS)

lsmsr.o: $(LSMSR_TMP_HEADERS)

lsmsr: $(LSMSR_TMP_HEADERS) $(LSMSR_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o lsmsr $(LSMSR_OBJS)




X86INFO_SRC = \
	AMD/identify.c \
	AMD/machine_check.c \
	AMD/MSR-Athlon.c \
	AMD/MSR-K6.c \
	AMD/powernow.c \
	AMD/dumppsb.c \
	AMD/bugs.c \
\
	Cyrix/identify.c \
\
	Intel/identify.c \
	Intel/identify-family6.c \
	Intel/identify-family6-extended.c \
	Intel/identify-family15.c \
	Intel/info.c \
	Intel/machine_check.c \
	Intel/cachesize.c \
	Intel/eblcr.c \
	Intel/MSR-P4.c \
	Intel/MSR-P6.c \
	Intel/MSR-performance.c \
	Intel/MSR-thermal.c \
	Intel/MSR-IDA.c \
	Intel/microcode.c \
	Intel/topology.c \
\
	Centaur/identify.c \
	Centaur/MSR-C3.c \
	Centaur/longhaul.c \
	Centaur/powersaver.c \
\
	NatSemi/identify.c \
\
	RiSE/identify.c \
\
	SiS/identify.c \
\
	x86info.c \
	commandline.c \
	havecpuid.c \
	cpuid.c \
	features.c \
	identify.c \
	rdmsr.c \
	binary.c \
	mptable.c \
	get_model_name.c \
	mtrr.c  \
	apic.c  \
	connector.c \
	topology.c \
\
	bench/benchmarks.c \
	bench/MHz.c

X86INFO_OBJS = $(X86INFO_SRC:%.c=%.o)

x86info: $(X86INFO_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o x86info $(X86INFO_OBJS) -lpci



nodes:
	scripts/makenodes

test:
	scripts/testnodes

release:
	git repack -a -d
	git prune-packed
	git archive --format=tar --prefix=x86info-$(VERSION)/ HEAD | gzip -9 > x86info-$(VERSION).tgz

clean:
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*~" -exec rm {} \;
	@rm -f x86info x86info.exe
	@rm -f lsmsr $(LSMSR_TMP_HEADERS)
	@rm -f core.*

splint:
	splint +posixlib -badflag -fileextensions -type -nullassign -boolops -showcolumn -sysunrecog -fullinitblock -onlytrans -unrecog -usedef -statictrans -compdestroy -predboolint -predboolothers -D__`uname -m`__ $(X86INFO_SRC)

sparse:
	sparse $(X86INFO_SRC)

cscope:
	cscope -Rb

mirror:
	@git push --mirror git@github.com:kernelslacker/x86info.git

scan:
	@scan-build --use-analyzer=/usr/bin/clang make

