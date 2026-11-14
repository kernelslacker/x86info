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

## 🟢 Enhancements

## 🟢 Minor nits

E3. `vendors/intel/identify-family6-extended.c:125` — model 0x66 is
    Cannon Lake (Core), not Atom.

E4. `connector.c:17,62` — double space in "Socket 370  (370 Pin PGA)";
    empty `/*  */` comment.

E5. `vendors/amd/identify.c:479-490,542-545` — `goto out_660` inside a
    `case` (an `else if` does it), and four blank lines.
    `vendors/amd/powernow.c:227` trailing whitespace.
    `vendors/intel/identify-family6.c:336` space indentation.
