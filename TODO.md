# x86info TODO

## 🔴 Correctness

## 🟡 Performance

## 🟡 Maintainability

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
