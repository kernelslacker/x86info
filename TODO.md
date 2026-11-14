# TODO

Whole-tree review. `make check` passes. Items marked "unverified" come
from reading the code only; confirm them before fixing.

## 🔴 Correctness

A10. `cpuid-freebsd.c`, `cpuid-solaris.c`, `rdmsr-freebsd.c` — these do
    not follow the `cpuid()` contract: they `exit()` on error, do not use
    `cpuid_store()`, and reopen the device on every call. Solaris has no
    `read_msr()`, so it cannot link. FreeBSD `bind_cpu()` sets the
    affinity of the whole process, which breaks the per-thread MHz
    measurement. `MHz.c` uses `CLOCK_MONOTONIC_RAW`, which I believe the
    BSDs lack (unverified). Decide first: delete the non-Linux backends,
    or repair them. (decision, then sonnet)

A11. `vendors/amd/identify.c:597` — `(ebx >> 16) & 0xff` of 0x80000021
    is printed as the return address predictor size in entries. My
    recollection of the APM is that the field is in units of 8.
    Unverified; check 24594 before changing. (haiku after the check)

## 🟡 Maintainability

B8. `vendors/intel/identify-family6-extended.c:209`,
    `identify-family6.c:299,309` — identification calls `ensure_MHz()`,
    so naming a Core 2 CPU can spin for 100 ms per CPU. Move the
    frequency-keyed SKU refinement to display time, or pass the rate in.
    See C2. (sonnet)

## 🟡 Tests & docs

C2. `vendors/intel/identify-family6-extended.c:25-104` — rows keyed on
    `.mhz` can never match in replay, because `host_can_measure()` is 0.
    Let the replay backend supply a rate (for example a `MHz:` line in
    the dump) so these rows get a fixture. (sonnet)

C3. `Makefile:71-76,115-119` — `check` and `check-asan` list the unit
    test binaries by hand next to `UNIT_TESTS`. Loop over `UNIT_TESTS`
    so a new test needs one edit. (haiku)

## 🟢 Enhancements

D1. `features.c:402` — CPUID 7 subleaf 1 EBX and ECX are not decoded,
    and an unknown vendor gets only 1:EDX (`features.c:649`). Decode the
    generic ECX table in the default case.

D2. `vendors/centaur/identify.c:133` — Zhaoxin cache data comes only
    from 0x80000005/6. These parts have leaf 4; reuse the Intel decoder.

## 🟢 Minor nits

E1. `Makefile:1` — `VERSION =` runs `git describe` on every expansion;
    use `:=`.

E2. `features.c:256,269,529` — "hle" has the TSX description, "Instruct"
    is cut short, "XSTOR" should be "XRSTOR".

E3. `vendors/intel/identify-family6-extended.c:125` — model 0x66 is
    Cannon Lake (Core), not Atom.

E4. `connector.c:17,62` — double space in "Socket 370  (370 Pin PGA)";
    empty `/*  */` comment.

E5. `vendors/amd/identify.c:479-490,542-545` — `goto out_660` inside a
    `case` (an `else if` does it), and four blank lines.
    `vendors/amd/powernow.c:227` trailing whitespace.
    `vendors/intel/identify-family6.c:336` space indentation.
