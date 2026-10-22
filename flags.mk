# Compiler flags shared by x86info and lsmsr.

FLAGS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

COMMON_CFLAGS = -Wall -W -g -O2 -D_FORTIFY_SOURCE=2
ifneq ($(SYSROOT),)
COMMON_CFLAGS += --sysroot=$(SYSROOT)
endif
COMMON_CFLAGS += -Wdeclaration-after-statement
COMMON_CFLAGS += -Wformat=2
COMMON_CFLAGS += -Wimplicit
COMMON_CFLAGS += -Winit-self
COMMON_CFLAGS += -Wmissing-declarations -Wredundant-decls
COMMON_CFLAGS += -Wnested-externs
COMMON_CFLAGS += -Wpacked
COMMON_CFLAGS += -Wshadow
COMMON_CFLAGS += -Wstrict-prototypes -Wmissing-prototypes
COMMON_CFLAGS += -Wswitch-enum
COMMON_CFLAGS += -Wundef
COMMON_CFLAGS += -Wwrite-strings

# gcc specific
ifneq ($(shell $(CC) -v 2>&1 | grep -c "clang"), 1)
COMMON_CFLAGS += -Wstrict-aliasing=3
COMMON_CFLAGS += -Wlogical-op
endif

# clang specific
#COMMON_CFLAGS += -fsanitize=undefined

# Only enabled during development, and on gcc 6.3+
CPP_MAJOR := $(shell $(CPP) -dumpversion 2>&1 | cut -d'.' -f1)
CPP_MINOR := $(shell $(CPP) -dumpversion 2>&1 | cut -d'.' -f2)
DEVEL   := $(shell grep VERSION $(FLAGS_DIR)Makefile | head -n1 | grep pre | wc -l)
COMMON_CFLAGS += $(shell if [ $(CPP_MAJOR) -eq 6 -a $(CPP_MINOR) -ge 3 -a $(DEVEL) -eq 1 ] ; then echo "-Werror"; else echo ""; fi)
