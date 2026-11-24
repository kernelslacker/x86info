# TODO

Whole-tree review. Baseline: `make check` passes, cppcheck is clean.

## 🔴 Correctness

## 🟡 Performance

## 🟡 Maintainability

C6. vendors/amd/powernow.c:get_cof (case 0x12) — five near-identical
    `f`/`d` return statements. Fix: a small table of {mul, div} per
    divisor ID. Model: haiku.

C7. vendors/intel/cachesize.c:decode_general_tlb — builds `assoc` (including
    a `strcpy`) before the `type == 0` early `continue`, so the work is
    wasted. Fix: move the `type == 0` test first and use `snprintf` for both
    branches. Model: haiku.

C8. host.c:host_longnop — uses `signal()` plus `siglongjmp`, which has
    unspecified handler semantics. The comment names `%eax`, but the
    code runs in 64-bit mode. Fix: use `sigaction` with `SA_NODEFER`,
    and fix the comment. Model: haiku.

C9. x86info.c:get_online_cpus — the `n > MAX_CPUS` message goes to stdout and
    is mixed into the normal output. `main()` also prints "Need to be root"
    to stdout. Fix: write both to stderr. Model: haiku.

## 🟡 Clean-ups / Style

D1. commandline.c — uses `//` comments for each case. The project rule
    is `/* */`. Fix: convert them. `usage()` prints only option names, with
    no descriptions, so it cannot replace the man page. Fix: add a help
    string per option in the table and print it. Model: haiku.

D2. cpuid-linux.c:CPUID_CHUNK_SIZE comment — points to
    `linux/arch/i386/kernel/cpuid.c`, which no longer exists
    (`arch/x86/kernel/cpuid.c`). Fix: correct the path. Model: haiku.

D3. rdmsr.c / binary.c — `binary()`, `binary32(unsigned long)` take
    `unsigned long`, but the values are 32 or 64 bit. `binary()` has no
    callers outside the MSR dumps. Fix: check callers, remove `binary()` if
    unused, and use `uint32_t` / `uint64_t`. Model: haiku.

D4. lsmsr/lsmsr.c:open_dev — `char s[20]` truncates "/dev/cpu/%d/msr" for CPU
    numbers of eight or more digits. Fix: use `char s[32]`. Model: haiku.

D5. cpuid.c:dump_raw_cpuid — `cpu->cpuid_level` is only masked with 0xffff,
    so a bogus CPU can make `-r` print 65536 leaves. The extended ranges
    use `sane_max_leaf`. Fix: cap the basic range the same way. Model:
    haiku.

D6. vendors/intel/identify.c — the family 0x12 and 0x13 branches are nested
    switches inside the `case 0xF` switch. Fix: move them to a table keyed
    by `efamily`/model like the family 6 tables. Model: sonnet.

## 🟡 Tests & docs

E1. CLAUDE.md (Architecture) — lists `natsemi`, `rise` and `sis` as vendor
    directories. They have no directory; `vendors/misc/identify.c` handles
    them. Fix: edit the vendor list. Model: haiku.

E2. tests — no unit test covers `cpudev_pread` descriptor fallback
    (EMFILE, ENODEV), `parse_command_line` for `--all` as non-root, or the
    lsmsr option parser. Fix: add `tests/test-cpudev.c` using a
    temporary directory, and an lsmsr option test. Model: sonnet.

E3. tests/test-opts.c — check that it covers short options `-a -c -f -m -r -v`
    and every long option in the man page (x86info.1 matches
    `long_options` today; keep it that way with a check). Model: haiku.

## 🟢 Minor

F1. vendors/amd/dumppsb.c — `ROMSIZE` is `(END-START)+1` = 0x3fff1, an odd
    byte count for `mmap`. Fix: use `0x100000 - START` (0x40000).

F2. mtrr.c:decode_address — the 36-bit fallback and the MSR numbers
    (0xfe, 0x2ff) are magic numbers. Fix: name them.
