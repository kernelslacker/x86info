VERSION=1.31pre

CFLAGS = -DVERSION="$(VERSION)"

CFLAGS += -Wall -W -g -O2 -D_FORTIFY_SOURCE=2 -I. -Iinclude
ifneq ($(SYSROOT),)
CFLAGS += --sysroot=$(SYSROOT)
endif
CFLAGS += -Wdeclaration-after-statement
CFLAGS += -Wformat=2
CFLAGS += -Wimplicit
CFLAGS += -Winit-self
CFLAGS += -Wmissing-declarations -Wredundant-decls
CFLAGS += -Wnested-externs
CFLAGS += -Wpacked
CFLAGS += -Wshadow
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes
CFLAGS += -Wswitch-enum
CFLAGS += -Wundef
CFLAGS += -Wwrite-strings
CFLAGS += $(shell pkg-config --cflags libpci)

# gcc specific
ifneq ($(shell $(CC) -v 2>&1 | grep -c "clang"), 1)
CFLAGS += -Wstrict-aliasing=3
CFLAGS += -Wlogical-op
endif

# clang specific
#CFLAGS += -fsanitize=undefined

# Only enabled during development, and on gcc 4.8+
CPP_MAJOR := $(shell $(CPP) -dumpversion 2>&1 | cut -d'.' -f1)
CPP_MINOR := $(shell $(CPP) -dumpversion 2>&1 | cut -d'.' -f2)
DEVEL   := $(shell grep VERSION Makefile | head -n1 | grep pre | wc -l)
CFLAGS  += $(shell if [ $(CPP_MAJOR) -eq 4 -a $(CPP_MINOR) -ge 8 -a $(DEVEL) -eq 1 ] ; then echo "-Werror"; else echo ""; fi)

LDFLAGS = -Wl,-z,relro,-z,now
LDFLAGS += $(shell pkg-config --libs libpci)

ifeq ($(CC),"")
CC = gcc
endif

ifdef STATIC_LIBPCI
LIBPCI = -Wl,-Bstatic -lpci -Wl,-Bdynamic -lz
else
LIBPCI = -lpci
endif

SHELL = /bin/sh

V	= @
Q	= $(V:1=)
QUIET_CC = $(Q:@=@echo    '  CC '$@;)

all: x86info

X86INFO_HEADERS = \
	$(patsubst %.h,%.h,$(wildcard *.h)) \
	$(patsubst %.h,%.h,$(wildcard include/*.h))

X86INFO_SRC = \
	$(wildcard *.c)	\
	$(wildcard vendors/*/*.c)

X86INFO_OBJS = $(sort $(patsubst %.c,%.o,$(wildcard *.c))) \
	$(sort $(patsubst %.c,%.o,$(wildcard vendors/*/*.c)))

x86info: $(X86INFO_OBJS) $(X86INFO_HEADERS)
	$(QUIET_CC)$(CC) $(CFLAGS) $(LDFLAGS) -o x86info $(X86INFO_OBJS) \
	    $(LIBPCI)

DEPDIR= .deps
-include $(X86INFO_SRC:%.c=$(DEPDIR)/%.d)
df = $(DEPDIR)/$(*D)/$(*F)

%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -o $@ -c $<
	@mkdir -p $(DEPDIR)/$(*D)
	@$(CC) -MM $(CFLAGS) $*.c > $(df).d
	@mv -f $(df).d $(df).d.tmp
	@sed -e 's|.*:|$*.o:|' <$(df).d.tmp > $(df).d
	@sed -e 's/.*://' -e 's/\\$$//' < $(df).d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(df).d
	@rm -f $(df).d.tmp

release:
	@git repack -a -d
	@git prune-packed
	@git archive --format=tar --prefix=x86info-$(VERSION)/ HEAD | gzip -9 > x86info-$(VERSION).tgz

clean:
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*~" -exec rm {} \;
	@rm -f x86info
	@rm -f core.*
	@rm -rf $(DEPDIR)/*
	@rm -f x86info-coverity.tar.xz
	@rm -rf cov-int

tag:
	@git tag -a v$(VERSION) -m "$(VERSION) release."

sparse:
	@sparse $(X86INFO_SRC)

cscope:
	@cscope -Rb

scan:
	@scan-build --use-analyzer=/usr/bin/clang make

tags:   $(X86INFO_SRC)
	@ctags -R --exclude=tmp

coverity:
	@rm -rf cov-int x86info-coverity.tar.xz
	@cov-build --dir cov-int make
	@tar cJvf x86info-coverity.tar.xz cov-int

