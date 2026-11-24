# TODO

Whole-tree review. Baseline: `make check` passes, cppcheck is clean.

## 🔴 Correctness

## 🟡 Performance

## 🟡 Maintainability

## 🟡 Clean-ups / Style

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
