# x86info modernization / improvement

## 3. Encapsulate global state into a context struct

`nrCPUs`, `user_is_root`, `need_root`, `firstcpu`, `debug`, `verbose`, and
every `show_*` flag are translation-unit globals scattered across
`commandline.c` and declared `extern` in `x86info.h`. Bundle them into a
`struct x86info_ctx` allocated in `main()` and threaded through the call tree.
This eliminates the brittle extern web and makes the data flow readable.

### Steps

1. In `include/x86info.h`, define:
   ```c
   struct x86info_ctx {
       unsigned int nrCPUs;
       unsigned int user_is_root;
       unsigned int need_root;
       unsigned int debug;
       unsigned int verbose;
       unsigned int all_cpus;
       unsigned int show_apic;
       unsigned int show_bios;
       unsigned int show_machine_check;
       unsigned int show_bugs;
       unsigned int show_cacheinfo;
       unsigned int show_connector;
       unsigned int show_msr;
       unsigned int show_microcode;
       unsigned int show_mtrr;
       unsigned int show_pm;
       unsigned int show_cpuid;
       unsigned int show_mptable;
       unsigned int show_feature_flags;
       unsigned int show_MHz;
       unsigned int show_addr_sizes;
       struct cpudata *firstcpu;
   };
   ```

2. Remove all the `extern unsigned int …` declarations from `x86info.h` for
   the fields above.

3. In `x86info.c` `main()`, declare `struct x86info_ctx ctx = {0};` and set
   `ctx.nrCPUs = 1;`. Pass `&ctx` to `parse_command_line()` and all other
   functions that currently reference the globals.

4. Update every function signature that accesses the old globals to accept
   `struct x86info_ctx *ctx` and replace bare references (`debug`, `verbose`,
   etc.) with `ctx->debug`, `ctx->verbose`, etc.

5. In `commandline.c`, remove the global variable definitions; the new
   `parse_command_line(int argc, char **argv, struct x86info_ctx *ctx)`
   writes through `ctx->…` instead.

6. Run `make`. Fix every "undeclared" or "implicit" error by plumbing `ctx`
   through to the affected call sites.

7. Commit: `ctx: bundle global state into struct x86info_ctx`

---

## 4. Separate data gathering from display

`show_info()` in `identify.c` mixes printing with control flow; the vendor
`identify_*` functions also print directly. The gather pass
(`fill_in_cpu_info`) already exists as a concept — finish the split so that
all vendor code only writes to `struct cpudata` and a separate display layer
does all `printf` calls. This is a prerequisite for any future JSON/machine-
readable output mode, and makes the `bind_cpu` FIXME in `x86info.c:37`
easier to resolve.

### Steps

1. Read `identify.c` and `show_info()`. List every `printf`/`puts` call
   inside `identify_*` functions and note what data they print (i.e. which
   field of `struct cpudata` or which local variable holds that value).

2. For each piece of data currently printed directly by vendor code, add a
   field to `struct cpudata` in `include/x86info.h` if one does not already
   exist (e.g. `char brand_string[48]`, `unsigned int apic_id`).

3. Refactor each vendor `identify_*` function to write into the new
   `struct cpudata` fields instead of calling `printf`. Remove the `printf`
   calls from the identify functions.

4. In `show_info()` (or a new `display_cpu_info()` function), add the
   corresponding `printf` calls that read from `struct cpudata` and emit
   the same output as before.

5. Resolve the `bind_cpu` FIXME in `x86info.c:37`: `display_detailed_info()`
   calls `bind_cpu(cpu->number)` before `show_info()`. Once all data is
   gathered in the gather pass (where `bind_cpu` is called per-CPU), the
   display pass does not need a separate `bind_cpu`; remove it.

6. Run `make`. Verify output of `./x86info -a` matches the pre-refactor
   output by diffing against a saved golden output captured beforehand:
   ```
   ./x86info -a > before.txt   # run before refactor
   ./x86info -a > after.txt    # run after refactor
   diff before.txt after.txt
   ```

7. Commit: `identify: separate data-gather from display`

---

## 5. Consolidate per-OS cpuid/rdmsr shims behind a clean interface

`cpuid-linux.c`, `cpuid-freebsd.c`, `cpuid-netbsd.c`, `cpuid-solaris.c` and
the corresponding `rdmsr-*` files all duplicate the same `bind_cpu` +
`native_cpuid` pattern with per-OS ifdefs or file selection. Define a single
internal header (`internal/os.h`) with the OS-abstraction contract, and make
each platform file implement exactly that interface; the Makefile/Meson build
selects the right one. This removes the `cpuid.c` forwarding shim and the
confusing split between `cpuid_UP` and `cpuid`.

### Steps

1. Read `cpuid-linux.c`, `cpuid-freebsd.c`, `cpuid-netbsd.c`,
   `cpuid-solaris.c`, and `cpuid.c`. Note the functions each file exports:
   `bind_cpu(unsigned int)`, `native_cpuid(…)`, and any helpers. Do the
   same for the four `rdmsr-*.c` files: note `read_msr(…)`.

2. Create `internal/os.h` (create the `internal/` directory first) with:
   ```c
   #pragma once
   #include <stdint.h>

   /* Bind the calling thread to CPU cpunr. No-op on UP builds. */
   void bind_cpu(unsigned int cpunr);

   /* Execute CPUID instruction on cpunr with leaf idx.
    * Returns 0 on success. Output pointers may be NULL. */
   int native_cpuid(unsigned int cpunr, unsigned long long idx,
                    unsigned int *eax, unsigned int *ebx,
                    unsigned int *ecx, unsigned int *edx);

   /* Read MSR idx on cpu. Returns 0 on success. */
   int read_msr(int cpu, unsigned int idx, unsigned long long *val);
   ```

3. Add `#include "internal/os.h"` to each `cpuid-*.c` and `rdmsr-*.c` file
   and verify each file implements exactly the functions declared above (add
   or rename as needed).

4. In the `Makefile`, replace the glob `$(wildcard *.c)` (which picks up
   all platform files) with an explicit variable. Add logic to select the
   right platform file based on `$(OS)` or `$(shell uname)`:
   ```make
   UNAME := $(shell uname -s)
   ifeq ($(UNAME),Linux)
       OS_CPUID = cpuid-linux.c
       OS_RDMSR = rdmsr-linux.c
   else ifeq ($(UNAME),FreeBSD)
       OS_CPUID = cpuid-freebsd.c
       OS_RDMSR = rdmsr-freebsd.c
   # … etc …
   endif
   ```
   Add `$(OS_CPUID)` and `$(OS_RDMSR)` to `X86INFO_SRC` and remove the
   other platform files from the build.

5. Remove `cpuid.c` (the forwarding shim) now that each platform file
   implements `native_cpuid` directly.

6. Update `include/x86info.h` to include `internal/os.h` (or include it
   in each `.c` that calls these functions) so the declarations are visible
   at call sites without duplicated `extern` declarations.

7. Run `make` on Linux. Fix any build errors. Confirm `./x86info -a` still
   works.

8. Commit: `os-shim: introduce internal/os.h and consolidate platform cpuid/rdmsr files`
