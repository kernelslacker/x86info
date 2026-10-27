VERSION=$(shell git describe --tags --always --dirty 2>/dev/null || date +%Y-%m-%d)

include flags.mk

CFLAGS = -DVERSION="$(VERSION)" -I. -Iinclude $(COMMON_CFLAGS)

LDFLAGS = -Wl,-z,relro,-z,now

# libpci is optional; "make LIBPCI=" builds without it.
ifdef STATIC_LIBPCI
LIBPCI = -Wl,-Bstatic -lpci -Wl,-Bdynamic -lz
else
LIBPCI ?= $(shell pkg-config --libs libpci 2>/dev/null)
endif

ifneq ($(strip $(LIBPCI)),)
CFLAGS += -DHAVE_LIBPCI $(shell pkg-config --cflags libpci 2>/dev/null)
endif

SHELL = /bin/sh

V	= @
Q	= $(V:1=)
QUIET_CC = $(Q:@=@echo    '  CC '$@;)

all: x86info lsmsr

.PHONY: lsmsr check record clean tarball sparse cscope scan tags coverity

X86INFO_HEADERS = \
	$(wildcard *.h) \
	$(wildcard include/*.h) \
	$(wildcard vendors/*/*.h)

X86INFO_SRC = \
	$(wildcard *.c)	\
	$(wildcard vendors/*/*.c) \
	$(wildcard lsmsr/*.c)

X86INFO_OBJS = $(sort $(patsubst %.c,%.o,$(wildcard *.c))) \
	$(sort $(patsubst %.c,%.o,$(wildcard vendors/*/*.c)))

x86info: $(X86INFO_OBJS) $(X86INFO_HEADERS)
	$(QUIET_CC)$(CC) $(CFLAGS) $(LDFLAGS) -o x86info $(X86INFO_OBJS) \
	    $(LIBPCI)

CHECK_BIN = tests/build/x86info
PSB_TEST = tests/build/test-psb
OPTS_TEST = tests/build/test-opts
MPTABLE_TEST = tests/build/test-mptable
CPULIST_TEST = tests/build/test-cpulist

# Build x86info against the CPUID replay backend and diff its output with
# the goldens in tests/dumps.
check: $(CHECK_BIN) $(PSB_TEST) $(OPTS_TEST) $(MPTABLE_TEST) $(CPULIST_TEST)
	@$(PSB_TEST) > /dev/null
	@$(OPTS_TEST)
	@$(MPTABLE_TEST)
	@$(CPULIST_TEST)
	@tests/test-runner.sh
	@tests/run-tests.sh $(CHECK_BIN)

$(PSB_TEST): tests/test-psb.c vendors/amd/dumppsb.c $(X86INFO_HEADERS)
	@mkdir -p tests/build
	$(QUIET_CC)$(CC) $(CFLAGS) -o $@ tests/test-psb.c vendors/amd/dumppsb.c

$(OPTS_TEST): tests/test-opts.c commandline.c $(X86INFO_HEADERS)
	@mkdir -p tests/build
	$(QUIET_CC)$(CC) $(CFLAGS) -o $@ tests/test-opts.c commandline.c

$(CPULIST_TEST): tests/test-cpulist.c cpulist.c $(X86INFO_HEADERS)
	@mkdir -p tests/build
	$(QUIET_CC)$(CC) $(CFLAGS) -o $@ tests/test-cpulist.c cpulist.c

$(MPTABLE_TEST): tests/test-mptable.c mptable.c $(X86INFO_HEADERS)
	@mkdir -p tests/build
	$(QUIET_CC)$(CC) $(CFLAGS) -o $@ tests/test-mptable.c mptable.c

# Record this machine's CPUID leaves, one section per CPU, as
# tests/dumps/$(NAME).dump. Check the output of the replay build, then
# create the golden with UPDATE=1 tests/run-tests.sh $(CHECK_BIN).
record: x86info
	@test -n "$(NAME)" || { echo "usage: make record NAME=<dump name>"; exit 1; }
	@test ! -e tests/dumps/$(NAME).dump || { echo "tests/dumps/$(NAME).dump exists"; exit 1; }
	@./x86info -r --all-cpus > tests/dumps/$(NAME).raw || { rm -f tests/dumps/$(NAME).raw; exit 1; }
	@grep -E '^(CPU #[0-9]+:|eax in:)' tests/dumps/$(NAME).raw > tests/dumps/$(NAME).dump; \
	 rm -f tests/dumps/$(NAME).raw; \
	 echo "recorded tests/dumps/$(NAME).dump: $$(grep -c '^CPU #' tests/dumps/$(NAME).dump) CPUs"

$(CHECK_BIN): $(X86INFO_HEADERS) $(wildcard *.c) $(wildcard vendors/*/*.c)
	@mkdir -p tests/build
	$(QUIET_CC)$(CC) $(CFLAGS) -DCPUID_REPLAY $(LDFLAGS) -o $@ \
	    $(wildcard *.c) $(wildcard vendors/*/*.c) $(LIBPCI)

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
	@rm -rf tests/build

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

