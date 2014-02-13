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

ifeq ($(CC),"")
CC = gcc
endif

SHELL = /bin/sh

all: x86info test

X86INFO_SRC = \
	$(wildcard *.c)	\
	$(wildcard AMD/*.c) \
	$(wildcard Cyrix/*.c) \
	$(wildcard Intel/*.c) \
	$(wildcard Centaur/*.c) \
	$(wildcard NatSemi/*.c) \
	$(wildcard RiSE/*.c) \
	$(wildcard SiS/*.c)

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
	@rm -f x86info
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

tags:   $(SRCS)
	@ctags -R --exclude=tmp

coverity:
	@rm -rf cov-int x86info-coverity.tgz
	@cov-build --dir cov-int make
	@tar czvf x86info-coverity.tgz cov-int

