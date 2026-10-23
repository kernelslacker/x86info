SHELL = /bin/sh

include ../flags.mk

VERSION ?= $(shell date +%Y-%m-%d)

CFLAGS = -DVERSION="$(VERSION)" -I. -I../include $(COMMON_CFLAGS)

LDFLAGS = -Wl,-z,relro,-z,now

.DELETE_ON_ERROR:

all: lsmsr

LSMSR_TMP_HEADERS=AMD_k8.h \
	AMD_fam10h.h \
	AMD_fam11h.h \
	AMD_fam12h.h \
	AMD_fam14h.h \
	AMD_fam15h.h \
	generic_msr.h

%.h: %.regs createheader.py
	@python3 createheader.py $< `basename $< .regs` >$@

LSMSR_SRC = \
	lsmsr.c \
	../cpuid.c \
	../cpuid-linux.c \
	../cpuid-freebsd.c \
	../cpuid-solaris.c \
	../havecpuid.c

LSMSR_OBJS = $(LSMSR_SRC:%.c=%.o)

lsmsr.c: $(LSMSR_TMP_HEADERS)

lsmsr.o: $(LSMSR_TMP_HEADERS)

lsmsr: $(LSMSR_TMP_HEADERS) $(LSMSR_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o lsmsr $(LSMSR_OBJS)


clean:
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*~" -exec rm {} \;
	@rm -f lsmsr $(LSMSR_TMP_HEADERS)
	@rm -f core.*

