# x86info TODO

## 🔴 Correctness

A33. topology.c:597-609 — sockets[] is sized max phys_proc_id + 1, and
    phys_proc_id (id >> pkg_shift) can be near UINT_MAX with bogus CPUID from
    a hypervisor: calloc() fails and the topology line silently disappears,
    or max_proc_id + 1 wraps to 0 and sockets[] is written out of bounds.
    Count distinct packages by sorting, as count_cores() does; total_threads
    is just nrCPUs. [sonnet]

A34. identify.c:38-41 — extended family/model are only decoded in
    identify_intel() and identify_amd(). Centaur (VIA Nano is family 6 model
    0xf with emodel set) and the other vendors keep emodel 0, so model(cpu),
    the header and the grouping are wrong for them.
    1. Decode efamily (base family 0xf) and emodel (base family 6 or 0xf; AMD
       only 0xf) once in get_cpu_info_basics().
    2. Drop the vendor copies.
    3. Have lsmsr's set_family_model() (lsmsr.c:220) use the same rule.
    [sonnet]

A35. lsmsr/Makefile:20 — `python3 createheader.py ... >$@` leaves an empty or
    partial header behind when the script fails (it exits 1 on a bad .regs)
    and the next make treats it as up to date. Add `.DELETE_ON_ERROR:`.
    [haiku]

A36. lsmsr/createheader.py:91-99 — parse_registers() uses independent ifs and
    never resets reg after "}", so a "{name=addr;desc}" line whose
    description contains ':' also goes to add_field() (IndexError), and any
    ':' line after a "}" is added to the previous register. Use match on the
    line prefix and set reg = None after appending. [haiku]

A37. vendors/amd/bugs.c:185 — --bugs (also set by -a for non-root users)
    reads MSR_CLKCTL without checking user_is_root, so a non-root run on a K7
    prints "/dev/cpu/0/msr: Permission denied" and latches nodriver. Gate it
    like decode_microcode() does. [haiku]

A38. microcode.c:407-411 — the Intel fallback reads MSR 0x8b directly, but
    the SDM requires writing 0 to it and executing CPUID 1 first, which a
    read-only /dev/cpu/N/msr can't do, so the value may be 0 or stale. Only
    reached without sysfs (FreeBSD, old kernels). Drop the MSR fallback for
    Intel. [haiku]

A39. rdmsr-linux.c:502-537, cpuid-linux.c:63,91 — the `nodriver` latches and
    lazily filled fd arrays are written from the --mhz worker threads
    (read_perf_counters -> read_msr). A data race; and as root without the
    msr module, several threads print the "modprobe msr" hint before one
    sets nodriver. Probe MSR access once in the main thread before
    estimate_MHz_cpus() starts threads (e.g. open the group leaders' fds).
    [sonnet]

## 🟡 Performance

B6. vendors/intel/identify-family6-extended.c:530-537 — get_namestring()
    checks MHz before stepping, so any CPU whose model has MHz-keyed entries
    (0xe, 0x17, 0x1a, 0x1c, 0x3c) spends ensure_MHz()'s 100ms spin even when
    no stepping matches. Test stepping and cores first. [haiku]

B7. cpuid.c:327, cpuid-linux.c:36-51 — without the cpuid driver (every
    non-root run) native_cpuid() calls bind_cpu() on each CPUID:
    CPU_ALLOC(), sched_setaffinity(), CPU_FREE(), hundreds of times per CPU.
    Remember the last CPU bound in a __thread variable in bind_cpu() and skip
    the syscall when it is unchanged. [haiku]

B8. x86info.c:497-511 — with --mptable, root runs open and scan /dev/mem
    twice (display_mptable() then enumerate_cpus()). Have read_mptable()
    record that it ran and let enumerate_cpus() reuse ncpu. [haiku]

## 🟡 Maintainability

C26. features.c:500-547 — the AMD and Intel cases are identical except that
    Intel omits the "\n" after the extended flags, so on Intel the CPUID
    6/7/0x80000007 flags run on at the end of the "Extended feature flags:"
    line. Merge the two cases and print show_extra_flags() output under its
    own heading; update the golden. [haiku]

C27. features.c:318-342 — show_extra_flags() also prints the CPUID 0x1F
    topology levels, which are topology, not flags. Move that block into
    topology.c (behind --debug or --verbose), and make show_extra_flags()
    static since only features.c calls it. [sonnet]

C28. Dead or needlessly exported functions: cpuid_ebx() (cpuid.c:358) has no
    callers; estimate_MHz() (MHz.c:288) is a one-line wrapper used only by
    measure_thread(); display_MHz() (MHz.c:37) is only used in MHz.c. Delete
    them or make them static, and drop their prototypes from x86info.h.
    [haiku]

C29. MHz.c:356-372 — ensure_MHz() calls cpuid_tsc_MHz() and then
    measure_MHz(cpu, 0), which calls cpuid_tsc_MHz() again and returns early
    with it. Just call measure_MHz(cpu, 0). [haiku]

C30. MHz.c:50,217-220, vendors/amd/identify.c:258 — CPUID 0x80000007 EDX is
    fetched in three places for the invariant TSC and mobile bits. Store it
    in cpudata from get_feature_flags() and test the bits there. [haiku]

C31. mtrr.c:446-454 — get_max_phy_addr() repeats the CPUID 0x80000008
    lookup that get_cpu_info_basics() already stores in cpu->phyaddr_bits.
    Use the field. [haiku]

C32. MHz.c:24,185 — the forward declarations of has_perf_counters() and
    elapsed_us() only exist because of definition order; reorder the
    functions. spin()'s LCG can go too: the clock_gettime() call in the loop
    already stops it from being optimised away. [haiku]

C33. cpudata nr_cores (vendors/intel/identify.c:64) and x86_max_cores
    (vendors/intel/topology.c:63) both hold intel_num_cpu_cores(), so leaf 4
    is decoded twice into two fields. Fill x86_max_cores once in
    identify_intel(), use it for the fam6e .cores match, and drop nr_cores.
    [sonnet]

C34. vendors/centaur/longhaul.c, vendors/centaur/powersaver.c — both decode
    MSR 0x110a with near-identical printf()/binary() blocks over the same
    union. Fold them into one decoder that takes the CPU generation.
    [sonnet]

C35. vendors/amd/machine_check.c:431-445 — when the family >= 0x10 read of
    MCG_CTL fails, the else-if reads it again and decodes it with the K7
    layout. Read MCG_CTL once, then branch on family. [haiku]

C36. connector.c:275-283 hand-rolls the lookup that the get_name() macro
    (include/x86info.h:128) generates for amd-revision.h and lsmsr. Use the
    macro. [haiku]

C37. vendors/intel/msr-p4.c:20,36,49 use raw 0x17, 0x8b and 0x1a0 although
    include/intel.h:28-36 defines MSR_IA32_PLATFORM_ID, MSR_IA32_UCODE_REV
    and MSR_IA32_MISC_ENABLE; microcode.c:384 defines its own
    MSR_UCODE_REV; MSR_IA32_PERF_CTL and MSR_IA32_ENERGY_PERF_BIAS are
    unused. Use the constants and drop the duplicates and unused ones.
    [haiku]

## 🟡 Clean-ups/Refactoring/Style

D9. features.c:299-301 — `ecx = 0;` before cpuid(..., 0x07, ...) does
    nothing, because the subleaf comes from idx. Use
    cpuid_count(cpu->number, 7, 0, ...). [haiku]

D10. identify.c:25-31 — the unknown-vendor message issues CPUID 0 a second
    time; fetch ECX in the first call. [haiku]

D11. vendors/amd/identify.c:313-324 — four identical case arms for families
    0x11, 0x12, 0x14 and 0x15; stack the labels. [haiku]

D12. vendors/centaur/identify.c:115,132 query CPUID 0x80000005 twice, and
    features.c:518 re-queries 0xc0000000 although cpu->maxei2 holds it.
    [haiku]

D13. x86info.c:379-386 — the `if (user_is_root)` around the MTRR/APIC dumps
    never fails: show_mtrr and show_apic set need_root, which main() already
    enforces. Drop it. [haiku]

## 🟡 Deprecated/Obsolete API Usage

E3. cpuid-solaris.c — bind_cpu is a FIXME stub and there is no rdmsr
    backend, so MSR output and per-CPU cpuid don't work. Implement them
    (processor_bind(), /dev/cpu/N/msr) or drop the README claim. [sonnet]

E6. include/apic.h:8 — PAGE_OFFSET() uses getpagesize(), which was dropped
    from POSIX.1-2001; use sysconf(_SC_PAGESIZE). [haiku]

E7. cpuid-linux.c:25-34 — the glibc < 2.4 #error guards a 2006 release that
    nothing can still build against. Remove it. [haiku]

## 🟡 Better API Alternatives

## 🟡 Architectural improvements

## 🟡 Comments

H4. mptable.c:209 — "convert physical address to virtual address" above a
    plain cast; nothing is converted. Remove it. [haiku]

H5. apic.c:119-142 — the `/** ... **/` blocks ("Verify if apic feature is
    supported", "Mapping the apic registers") restate the next line. Remove
    them. [haiku]

## 🟡 Tests & docs

I1. `make check` replays CPUID dumps from tests/dumps and diffs the output
    with the .out goldens, but only a Zen 5 dump is checked in. Add dumps
    (`x86info -r | grep '^eax in'`) from a K7, K8, Zen 2/3/4, Core 2,
    Nehalem, Alder Lake hybrid and VIA CPU, then `UPDATE=1
    tests/run-tests.sh tests/build/x86info` after checking the output by
    hand. [needs hardware]

I4. The replay backend describes exactly one CPU (x86info.c:297-299,
    cpuid-replay.c), so CPU grouping, the shared header, hybrid core types,
    the MHz leader copy and the topology summary are untested, although
    most recent commits changed them.
    1. Let a dump hold several "CPU N:" sections (`x86info -r --all-cpus`
       already prints one block per CPU) and key replay entries by CPU.
    2. Have get_online_cpus() return the replayed CPU ids under
       CPUID_REPLAY.
    3. Add a two-CPU P+E hybrid dump and a two-identical-CPU dump with
       goldens, and cover A27 with an SMT dump that lists one thread per
       core.
    [sonnet]

## 🟢 Enhancements

J5. cpuid.c:369-394 — Zhaoxin ("  Shanghai  ", EBX 0x68532020) is
    VENDOR_UNKNOWN. Map it to VENDOR_CENTAUR (same lineage, same 0xC000000x
    leaves) and add the family 7 names to identify_centaur(). Depends on
    A34. [sonnet]

J6. cpuid.c:413-444 — dump_subleaves() skips other leaves that have
    subleaves (0xf, 0x10, 0x17, 0x1d, 0x20, 0x23, 0x24, 0x80000026), so -r
    output, and the dumps made from it for tests, are missing them.
    [haiku]

## 🟢 Minor nits / one-liners

K6. vendors/intel/identify-family6-extended.c:445 — stray ")" in
    "Core i7 [Broadwell])". [haiku]

K7. vendors/intel/misc-enable.c:181 — "compatability". [haiku]

K8. vendors/intel/machine_check.c:136 — file-scope nr_msr_banks is only used
    inside decode_intel_machine_check(); make it a local. [haiku]

K9. vendors/amd/identify.c:534-544 — "Duron " + " (Thoroughbred)" gives a
    double space, and "[A0]"/"[B0]" are appended with no leading space.
    [haiku]

K10. include/amd.h:9-11 (<linux/types.h>), include/powernow.h and
    include/powersaver.h (<sys/types.h>) and include/intel.h (<string.h>)
    include headers nothing in them uses. [haiku]

K11. .gitignore — `.d` and `x86info-*.tgz` are stale: dependency files live
    in .deps/ and `make tarball` writes .tar.xz. [haiku]

K12. vendors/amd/powernow.c:15,22 (mobile_vid_table, fid_codes),
    vendors/amd/amd-revision.h and vendors/intel/identify-family6-extended.c
    (intel_fam6e_names) — read-only tables that aren't const. [haiku]

K13. lsmsr/lsmsr.c:169-194 — show_matching()'s length juggling is
    strncmp(table name, name, strlen(name)) == 0; and -c accepts negative
    CPU numbers. [haiku]

K14. x86info.c:533,540 — "CPU #%u" prints the array index + 1, not the CPU
    number, so the labels are wrong once some CPUs are offline. Print
    cpus[i].number. [haiku]
