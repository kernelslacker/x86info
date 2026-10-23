VERSION=$(shell git describe --tags --always --dirty 2>/dev/null || date +%Y-%m-%d)

include flags.mk

CFLAGS = -DVERSION="$(VERSION)" -I. -Iinclude $(COMMON_CFLAGS)
CFLAGS += $(shell pkg-config --cflags libpci)

LDFLAGS = -Wl,-z,relro,-z,now

ifdef STATIC_LIBPCI
LIBPCI = -Wl,-Bstatic -lpci -Wl,-Bdynamic -lz
else
LIBPCI ?= $(shell pkg-config --libs libpci)
endif

SHELL = /bin/sh

V	= @
Q	= $(V:1=)
QUIET_CC = $(Q:@=@echo    '  CC '$@;)

all: x86info lsmsr

.PHONY: lsmsr clean tarball sparse cscope scan tags coverity

X86INFO_HEADERS = \
	$(wildcard *.h) \
	$(wildcard include/*.h)

X86INFO_SRC = \
	$(wildcard *.c)	\
	$(wildcard vendors/*/*.c) \
	$(wildcard lsmsr/*.c)

X86INFO_OBJS = $(sort $(patsubst %.c,%.o,$(wildcard *.c))) \
	$(sort $(patsubst %.c,%.o,$(wildcard vendors/*/*.c)))

x86info: $(X86INFO_OBJS) $(X86INFO_HEADERS)
	$(QUIET_CC)$(CC) $(CFLAGS) $(LDFLAGS) -o x86info $(X86INFO_OBJS) \
	    $(LIBPCI)

lsmsr: x86info
	$(Q)$(MAKE) --no-print-directory -C lsmsr VERSION=$(VERSION)

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

tarball:
	@git archive --format=tar --prefix=x86info-$(VERSION)/ HEAD > x86info-$(VERSION).tar
	@xz -9 x86info-$(VERSION).tar


clean:
	@find . -name "*.o" -exec rm {} \;
	@find . -name "*~" -exec rm {} \;
	@rm -f x86info
	@$(MAKE) --no-print-directory -C lsmsr clean
	@rm -f core.*
	@rm -rf $(DEPDIR)/*
	@rm -f x86info-coverity.tar.xz
	@rm -rf cov-int

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

