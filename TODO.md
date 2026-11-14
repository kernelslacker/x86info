# TODO

Whole-tree review. `make check` passes. Items marked "unverified" come
from reading the code only; confirm them before fixing.

## 🔴 Correctness

A5. `lsmsr/lsmsr.c:134` — `get_reg_name()` returns NULL for an MSR that
    is not in the table, and the result goes to `%s`. Reachable with
    `lsmsr -r <unknown addr>` when the read fails. Print "unknown".
    (haiku)

A6. `rdmsr.c:15` — the 32-bit labelled form prints `(unsigned long)val`
    with `%08lx`, so on 64-bit the hex shows the high half while
    `binary32()` shows only the low half. Cast to `unsigned int`, use
    `%08x`. (haiku)

A7. `MHz.c:50` — `display_MHz_line()` prints "0MHz ..." when nothing was
    measured: no TSC, `bind_cpu()` failure in `measure_thread()`, or a
    replay dump without leaf 0x15/0x16 (seen with `--mhz` on
    `amd/zen3-rembrandt`). Print "unknown" for zero. (haiku)

A8. `features.c:620` — Centaur/Zhaoxin decode CPUID 1 ECX with a
    three-entry table, so modern Zhaoxin output is mostly `[1:ecx:N]`
    (see `centaur/centaur-qemu-yongfeng-v1.out` line 15). The same
    applies to 0x80000001 EDX. (sonnet)
    - Use the generic ECX and extended tables for `VENDOR_CENTAUR`.
    - Keep the 0xC0000001 table.
    - Regenerate the Centaur goldens.

A9. `vendors/centaur/msr-c3.c:15` — "Power management: " is printed with
    no newline and no text for models other than 6 to 9. Print the
    heading only inside the two branches. (haiku)

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

B1. `features.c:60-597` — each leaf has a name array and a parallel
    description array, kept in step by index comments, in two different
    initializer styles. Merge each pair into one table of
    `{ name, desc }` with designated indexes. (sonnet)

B2. `topology.c:175-180` and `vendors/amd/identify.c:644-651` — the same
    0x80000008 ECX decode (thread count, APIC ID width) appears twice,
    with the same comment. `identify.c:657` also reads 0x8000001E without
    the TopoExt check that `topology.c:188` makes. Extract one helper
    that returns threads, possible threads and threads per core. (sonnet)

B3. `cpuid.c:106` — `subleaf_state()` takes nine arguments. Pass two
    `const unsigned int regs[4]` (this subleaf, subleaf 0). (sonnet)

B4. `include/x86info.h:181` — `read_msr()` returns 1 on success and
    `cpuid()` returns 0. Callers test `== 1`, `!= 1` and `!read_msr()`.
    The CPU number is `int` in `read_msr`, `dumpmsr`, `dump_raw_cpuid`
    and the machine check functions, `unsigned int` elsewhere. Make
    `read_msr()` return `bool` and take `unsigned int`. (sonnet; wide but
    mechanical)

B5. `vendors/intel/cachesize.c:445` — `decode_intel_caches(cpu, output)`
    mixes sizing and printing. The `output == 0` pass from
    `identify_intel()` still walks leaf 4 and 0x18 and discards the
    result. Split into a sizing function that fills `cachesize_*` and a
    display function. (sonnet)

B6. `machine_check.c:10`, `vendors/amd/machine_check.c:13`,
    `vendors/intel/machine_check.c:13` — `MCG_*` and `MC_CTL` are defined
    three times. Move them to one header. (haiku)

B7. `vendors/amd/powernow.c:196` — `get_cof()` is a `goto` maze, returns
    0 for a bad divisor although the comment says -1, and
    `get_num_boost_states()` prints as a side effect of a getter. Use
    one `switch` on the family, and move the printing to the caller.
    (sonnet)

B8. `vendors/intel/identify-family6-extended.c:209`,
    `identify-family6.c:299,309` — identification calls `ensure_MHz()`,
    so naming a CPU can spin for 100 ms, and the static cache gives every
    CPU the first CPU's rate. Move the frequency-keyed SKU refinement to
    display time, or pass the rate in. See C2. (sonnet)

B9. `identify.c:38-46`, `lsmsr/lsmsr.c:211-229`, `mptable.c:163-169` —
    three copies of the family/model decode from CPUID 1 EAX, with
    different extended-model rules. Add one helper in `cpuid.c`. (sonnet)

B10. `topology.c:366-383` — an `offsetof` table to print three fields in
    debug mode; three plain loops are shorter. `mptable.c:303`
    `apic_probe()` returns codes 1 to 6 that no caller tells apart;
    return found / not found. (haiku)

B11. `identify.c:121` — Centaur and Cyrix print cache data from their
    `show_info` hook under `opts.show_cacheinfo`, while AMD and Intel use
    the `show_cache` hook. Move both to `show_cache`. (sonnet)

B12. `tests/qemu/run.py` — no type hints, results are bare dicts, and
    `test_model()` is about 70 lines that mix the QEMU lifecycle with the
    replay comparisons. Add hints, a `TypedDict` result, and split the
    function. (sonnet; lint with haiku)

B13. Dead or redundant code. (haiku)
    - `lsmsr/lsmsr.c:53,351` — `-m/--model` is parsed but only printed in
      an error message; `-DVERSION` is unused next to `LSMSR_VERSION`.
    - `vendors/amd/identify.c:180` — unused `id` parameter.
    - `include/apic.h` — `APIC_APR`, `APIC_EOI`, `APIC_RRD` unused.
    - `vendors/cyrix/identify.c:46` — `maxei < 0x80000000` is covered by
      the next test.
    - `vendors/amd/powernow.c:391` — `else if (family >= 0x10)` is always
      true there.
    - Unused includes: `MHz.c` `<sys/time.h>`, `vendors/amd/dumppsb.c`
      `<stdlib.h>` `<ctype.h>`, `vendors/intel/topology.c` `<strings.h>`.

B14. `vendors/centaur/identify.c:60-110` — process, voltage and
    transistor-count notes sit in comments. Move them to
    `docs/sspecs.md`. (haiku)

B15. `tests/run-tests.sh:23` — `shellcheck disable=SC2068` hides an
    unquoted `$@`. `"$@"` passes the same words; remove the directive.
    (haiku)

## 🟡 Tests & docs

C1. `tests/run-tests.sh:85` — the goldens never run `--mhz` or `--pm`.
    Both are deterministic in replay (CPUID 0x15/0x16, the PowerNOW
    feature list), so `cpuid_tsc_MHz()`, `display_MHz_line()` and
    `decode_powernow()` have no coverage. Add both options to the default
    golden after A7. (haiku, then regenerate)

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
