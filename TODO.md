# x86info TODO

## 🔴 Correctness

A6. vendors/intel/cachesize.c:452-462 — the L2L3 vs L3L2 heuristic is
    broken: the TRACE pass already marks every non-trace descriptor as
    unknown, so `oldknown == found_unknown` is always true, L3L2_cache_table
    is dead, and descriptors 0x46/0x47/0x4a-0x4d (L3 per the SDM) are always
    reported and summed as L2. 0x48 is also in both L2_cache_table and
    L2L3_cache_table, so it is printed and counted twice. cachesize_L2
    feeds the naming switches in identify-family6.c. [opus]
    1. Move 0x46/0x47/0x4a-0x4d into L3_cache_table, 0x78 into L2.
    2. Special-case 0x49: L3 on family 0xf model 6, L2 otherwise.
    3. Delete L2L3/L3L2 tables and the oldknown logic.
    (Fold into G1 if doing the restructure.)

## 🟡 Performance

B2. x86info.c:197 — with --all-cpus --mhz, estimate_MHz() runs a 1 s busy
    loop serially per CPU (128 s on a 128-thread box). Measure all CPUs in
    parallel (one pthread per CPU, pinned) or measure for a shorter
    window. [opus]

B3. vendors/intel/cachesize.c:259-285 — decode_cache() re-runs CPUID 2 for
    each of ~11 tables, twice (once during identify, once for --cache).
    Read the descriptor bytes once into a 16*n array (see G1). [sonnet]

B4. vendors/amd/powernow.c:135-160,233-268 — two functions each do a full
    pci_alloc/init/scan_bus/cleanup. Share one helper that scans once and
    finds a device by vendor + list of device IDs. [sonnet]

## 🟡 Maintainability

C10. apic.c:21,151-212 / mtrr.c:63,182-204 — apic_msr_value() and
    mtrr_value() are pointless wrappers around `read_msr()==1`; the APIC
    register dump and fixed-range MTRR dump are long printf/dump pairs that
    want `{label, offset}` tables. APIC_BASE is also read 4 times; read it
    once. [sonnet]

C11. vendors/amd/identify.c:194-248 — set_fam11h/12h/14h/15h_revinfo are
    the same function with a different lookup and prefix; set_fam17h/19h/
    1ah_revinfo are one-line wrappers. Use a `{family, prefix, lookup}`
    table and one function each. [sonnet]

C12. vendors/amd/identify.c:102-177 — set_connector() is four switches
    mapping pkg_id to a constant. Use per-family arrays indexed by pkg_id;
    `c->connector = 0` should be CONN_UNKNOWN. [haiku]

C13. vendors/amd/identify.c:345-396 — do_l2assoc() switch is a 16-entry
    lookup table. [haiku]

C14. vendors/amd/identify.c:402-497 — 1G TLB leaf 0x80000019 is queried
    twice; the TLB line and cache block prints are repeated 8x/3x. Add
    `print_tlb(label, assoc_fn, assoc, entries)` and
    `print_cache(label, size, assoc, lpt, line)` helpers. [sonnet]

C15. vendors/amd/model-name.c:22-75 — 55-line switch of brand strings plus
    a second switch on model letter. Make it a
    `{index, letter, fmt}` table. [sonnet]

C16. mptable.c:251-385 — seven copies of "read region, scan 16-byte aligned
    for _MP_". Extract `scan_for_mp(addr, size, &paddr)`. enumerate_cpus()
    and display_mptable() (lines 388-458) are also duplicates differing only
    in `silent`; merge into one function taking `silent`. [sonnet]

C17. vendors/intel/identify.c:33-38 duplicates intel_num_cpu_cores() in
    vendors/intel/topology.c:36; identify.c:29-30 re-stores flags_ecx/edx
    already set by get_feature_flags(). Keep one. [haiku]

C18. vendors/intel/topology.c — phys_pkg_id() is a one-line wrapper; the
    `goto out; out: return;` is noise; `num_siblings == 1` and `<= 1` checks
    are redundant; cpuid_ebx(cpu, 1) re-issues a leaf already read. [haiku]

C19. vendors/intel/info.c:37-49 — six snprintf calls with manual offset
    arithmetic build the serial; one
    `snprintf(..., "%04X-%04X-%04X-%04X-%04X-%04X", ...)` does it. [haiku]

C20. vendors/centaur/longhaul.c:10-37 / include/powersaver.h — same MSR
    (0x110a), near-identical bitfield unions. Share one union. [haiku]

C21. vendors/amd/powernow.c:89-95 — k8_fid_codes[64] is
    `fid & 1 || fid > 42 ? -1 : 4 + fid / 2`. Compute it. [haiku]

C22. Redundant `if (!user_is_root) return;` in dump_* / decode_* MSR
    functions (msr-athlon, msr-k6, msr-p4, msr-performance, msr-thermal,
    microcode, machine_check x2, powersaver, msr-c3, show_patch_level): all
    are behind need_root already. Remove. [haiku]

C23. vendors/intel/identify-family6-extended.c:14,228 — ONLYMHZ is never
    set in the table; remove the flag and its branch. strncpy at line 244
    should be snprintf. [haiku]

C24. get_model_name.c:12 — named get_ but prints; re-queries the vendor via
    CPUID instead of using cpu->vendor. Rename to display_model_name() and
    use cpu->vendor. [haiku]

C25. vendors/amd/identify.c:1025-1029 — AMD always prints "Address Size"
    (virtual/physical order) duplicating --addr (physical/virtual). Remove
    the AMD copy. [haiku]

## 🟡 Clean-ups/Refactoring/Style

D1. Remove commented-out code: powernow.c:70-73, identify-family6.c:225,
    amd/identify.c:859-861,871-873, intel/identify.c:69-72,107-110,
    centaur/identify.c:20-24 etc, intel/info.c:141, intel/topology.c:63-69,
    centaur/powersaver.c:75-76, cachesize.c:60-66. [haiku]

D2. cpuid.c:48-52 — cpuid4() is cpuid_count(cpu, 4, n, ...). Delete it and
    use cpuid_count. dump_raw_cpuid's `ecx = 0;` (line 80) is dead. [haiku]

D3. x86info.c:181 — check_cpu_similarity() open-codes the model() macro.
    x86info.c:20 nrSMPCPUs is a file global only used in main. [haiku]

D4. commandline.c:78-86 — usage() re-derives short options with a switch
    mirroring long_options. Print `-%c` when `val < 128`. [haiku]

D5. vendors/cyrix/identify.c:15 / include/cyrix.h — decode_cyrix_tlb() is
    only used locally; make it static. [haiku]

D6. lsmsr/lsmsr.c:408-411 — usage() exits, so `goto out` is dead;
    `optind > -1` (line 413) is always true; first cpuid(…, 0) in
    set_family_model() (line 257) is unused. [haiku]

D7. vendors/amd/dumppsb.c:132-134 — `for (j…) p += 2;` is
    `p += 2 * numpstates`. exit() on munmap/close failure is overkill. [haiku]

D8. mtrr.c:25 — static max_phy_addr is reset and set per call; pass it to
    decode_address(). <asm/mtrr.h> is included only for MTRR_NUM_TYPES;
    define it locally. [haiku]

## 🟡 Deprecated/Obsolete API Usage

E2. cpuid-solaris.c:57 — lseek64/off64_t/_LARGEFILE64_SOURCE (removed in
    musl, deprecated in glibc). Use pread() with `-D_FILE_OFFSET_BITS=64`.
    [haiku]

E3. cpuid-solaris.c — bind_cpu is a FIXME stub and there is no rdmsr
    backend, so MSR output and per-CPU cpuid don't work. Implement them
    (processor_bind(), /dev/cpu/N/msr) or drop the README claim. [sonnet]

E5. lsmsr/createheader.py — `#!/usr/bin/python`, no `with open`, regex
    `re.search("^{")` for startswith, the `"<msr.h>"` branch emits an
    invalid include, no type hints. Bring to PYTHON.md standards (pathlib,
    startswith, type hints, black, pyrefly strict). [sonnet]

## 🟡 Better API Alternatives

F1. vendors/intel/microcode.c / amd/identify.c:956 — reading MSR 0x8b needs
    root, and Intel requires a WRMSR+CPUID sequence first. Linux exposes
    /sys/devices/system/cpu/cpuN/microcode/version without root. Prefer it,
    fall back to the MSR. Unify the Intel/AMD functions. [sonnet]

F2. MHz.c:103-122 — elapsed time is assumed to be exactly 1 s from alarm().
    Bracket the rdtsc reads with clock_gettime(CLOCK_MONOTONIC_RAW) and
    divide by the measured interval. When CPUID 0x15/0x16 give the TSC or
    base frequency, report that instead of measuring. [sonnet]

## 🟡 Architectural improvements

G1. vendors/intel/cachesize.c — restructure decoding: [opus]
    1. Read all CPUID 2 descriptor bytes once.
    2. One combined descriptor table carrying a level/kind field (trace,
       L1I, L1D, L2, L3, ITLB, DTLB, prefetch) instead of table-pointer
       identity checks (lines 227-244) to pick the size field.
    3. A descriptor not found in the table is unknown; drop
       unknown_array/found_unknown/clean_unknowns and the static
       found_general/found_tlb_general state.
    Supersedes A6, B3.

G2. vendors/amd/identify.c (1056 lines) — move decode_amd_cacheinfo(),
    do_assoc(), do_l2assoc() and the 0x8000001D decoder to
    vendors/amd/cacheinfo.c, mirroring intel/cachesize.c. [haiku]

G3. vendors/natsemi, rise, sis — three ~20-line files, each a one-case
    switch. Merge into one vendors/misc/identify.c with a
    `{vendor, tuple, mask, name}` table. [haiku]

G4. include/msr.h, include/amd-revision.h — headers defining static
    functions/tables, each included by exactly one .c. Move msr.h into
    lsmsr/ (it isn't used by x86info) and amd-revision.h next to
    vendors/amd/identify.c. [haiku]

G5. 18 `show_*` globals in commandline.c / x86info.h. Group into a
    `struct options` (one extern) to make the option set discoverable and
    testable. Low priority. [sonnet]

## 🟡 Comments

H1. Remove the S-spec / release-date history blocks: amd/identify.c:782-794,
    833-852,874-949, identify-family6.c:116-131,163-186,248-264,319-389,
    408-476, identify-family6-extended.c:26-74, identify-family15.c
    throughout. If worth keeping, move to docs/sspecs.md. [haiku]

H2. Stale/wrong comments: sis/identify.c:4 says "Rise specific parts";
    centaur/identify.c:154 "check on-chip L2 cache size" (nothing follows);
    intel/info.c:144 "FIXME: Bit test for MCA here!"; cachesize.c:200,222
    and identify-family6-extended.c:243 TODOs; apic.c:7 / mtrr.c:5
    "TODO: decode the registers too" (mtrr already decodes). [haiku]

H3. features.c:285,291,307,312 — "// Intel CPUID 0x06" etc on leaves used
    for all vendors. Drop "Intel". [haiku]

## 🟡 Tests & docs

I1. No tests. Identification is a pure function of CPUID values. [opus]
    1. Add a compile-time-selectable cpuid backend that replays a recorded
       dump (the `x86info -r` format, or kernel /dev/cpu dumps).
    2. Check in dumps for a handful of CPUs (K7, K8, Zen 2/3/4/5, Core 2,
       Nehalem, Alder Lake hybrid, VIA) with expected name/cache/topology
       output.
    3. `make check` diffs output against goldens.
    This would have caught A1, A2, A6, A8, A9.

I2. README — mknod/modprobe.conf instructions, "glibc 2.4", "Solaris should
    work" are all obsolete. Rewrite: needs the cpuid/msr modules,
    /dev/cpu/* from devtmpfs, libpci for --pm on AMD. [haiku]

I3. lsmsr/createheader.py — validate that each register's field widths sum
    to 64; msr.h print_reg_bits() shifts by the running offset and is UB
    past 64. [haiku]

## 🟢 Enhancements

J1. vendors/amd/identify.c:624 — Hygon (family 0x18) maps to VENDOR_AMD
    but prints "Unknown CPU family: 0x18". Add a Dhyana entry. [haiku]

J2. Intel hybrid parts: report per-CPU core type (CPUID 0x1A EAX[31:24],
    P-core/E-core) in the per-CPU and topology output. [sonnet]

J3. rdmsr-linux.c:28 — on ENOENT for /dev/cpu/N/msr, hint
    "modprobe msr" once instead of a bare perror. Same for cpuid. [haiku]

J4. cpuid.c:76 — dump_raw_cpuid() only dumps subleaf 0. Also walk
    subleaves for 4, 7, 0xB, 0xD, 0x12, 0x14, 0x18, 0x1F, 0x8000001D,
    0x80000020. [sonnet]

## 🟢 Minor nits / one-liners

K1. Makefile — VERSION from `date` makes builds irreproducible; use
    `git describe --always --dirty` with date fallback. Add .PHONY for
    clean/tarball/sparse/cscope/scan/tags/coverity. `$(patsubst %.h,%.h,…)`
    is a no-op. [haiku]

K2. vendors/amd/identify.c:1008-1052 — `%d` with `(int)` casts on unsigned
    CPUID fields; use `%u`. [haiku]

K3. vendors/amd/identify.c:62-68 — abort() on snprintf < 0 is overkill.
    set_k8_revinfo() sets connector twice (lines 76, 86). [haiku]

K4. apic.c:183 — "IRR0%d" label typo (prints IRR00..IRR07). [haiku]

K5. lsmsr/lsmsr.c:47,207,350 — `g`, `vendor_names`, `unknown_msr`,
    `lopts` should be static. Same for cachesize.c:355,364
    tlbnames/pagenames. [haiku]
