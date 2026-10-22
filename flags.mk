# Compiler flags shared by x86info and lsmsr.

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

# make WERROR=1 turns warnings into errors
ifdef WERROR
COMMON_CFLAGS += -Werror
endif
