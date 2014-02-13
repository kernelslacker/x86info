ifeq ($(CC),"")
CC = gcc
endif

SHELL = /bin/sh

CFLAGS = -DVERSION="$(VERSION)"

CFLAGS += -Wall -W -g -O2 -D_FORTIFY_SOURCE=2 -I. -I../include
ifneq ($(SYSROOT),)
CFLAGS += --sysroot=$(SYSROOT)
endif
CFLAGS += -Wdeclaration-after-statement
CFLAGS += -Wformat=2
CFLAGS += -Wimplicit
CFLAGS += -Winit-self
CFLAGS += -Wlogical-op
CFLAGS += -Wmissing-declarations -Wredundant-decls
CFLAGS += -Wnested-externs
CFLAGS += -Wpacked
CFLAGS += -Wshadow
CFLAGS += -Wstrict-aliasing=3
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes
CFLAGS += -Wswitch-enum
CFLAGS += -Wundef
CFLAGS += -Wwrite-strings

CFLAGS += -Werror

LDFLAGS = -Wl,-z,relro,-z,now

all: lsmsr

LSMSR_TMP_HEADERS=AMD_k8.h \
	AMD_fam10h.h \
	AMD_fam11h.h \
	AMD_fam12h.h \
	AMD_fam14h.h \
	AMD_fam15h.h \
	generic_msr.h

%.h: %.regs createheader.py
	@python createheader.py $< `basename $< .regs` >$@

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

