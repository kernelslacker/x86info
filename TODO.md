# TODO

Whole-tree review. Baseline: `make check` passes, cppcheck is clean.

## 🔴 Correctness

## 🟡 Performance

## 🟡 Maintainability

## 🟡 Clean-ups / Style

## 🟡 Tests & docs

E3. tests/test-opts.c — check that it covers short options `-a -c -f -m -r -v`
    and every long option in the man page (x86info.1 matches
    `long_options` today; keep it that way with a check). Model: haiku.

## 🟢 Minor

F1. vendors/amd/dumppsb.c — `ROMSIZE` is `(END-START)+1` = 0x3fff1, an odd
    byte count for `mmap`. Fix: use `0x100000 - START` (0x40000).

F2. mtrr.c:decode_address — the 36-bit fallback and the MSR numbers
    (0xfe, 0x2ff) are magic numbers. Fix: name them.
