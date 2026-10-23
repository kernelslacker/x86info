# x86info TODO

## 🟡 Deprecated/Obsolete API Usage

E2. cpuid-solaris.c:57 — lseek64/off64_t/_LARGEFILE64_SOURCE (removed in
    musl, deprecated in glibc). Use pread() with `-D_FILE_OFFSET_BITS=64`.
    [haiku]

E3. cpuid-solaris.c — bind_cpu is a FIXME stub and there is no rdmsr
    backend, so MSR output and per-CPU cpuid don't work. Implement them
    (processor_bind(), /dev/cpu/N/msr) or drop the README claim. [sonnet]

## 🟡 Architectural improvements

G5. 18 `show_*` globals in commandline.c / x86info.h. Group into a
    `struct options` (one extern) to make the option set discoverable and
    testable. Low priority. [sonnet]

## 🟡 Tests & docs

I1. No tests. Identification is a pure function of CPUID values. [opus]
    1. Add a compile-time-selectable cpuid backend that replays a recorded
       dump (the `x86info -r` format, or kernel /dev/cpu dumps).
    2. Check in dumps for a handful of CPUs (K7, K8, Zen 2/3/4/5, Core 2,
       Nehalem, Alder Lake hybrid, VIA) with expected name/cache/topology
       output.
    3. `make check` diffs output against goldens.
    This would have caught A1, A2, A6, A8, A9.

## 🟢 Enhancements

J2. Intel hybrid parts: report per-CPU core type (CPUID 0x1A EAX[31:24],
    P-core/E-core) in the per-CPU and topology output. [sonnet]

J4. cpuid.c:76 — dump_raw_cpuid() only dumps subleaf 0. Also walk
    subleaves for 4, 7, 0xB, 0xD, 0x12, 0x14, 0x18, 0x1F, 0x8000001D,
    0x80000020. [sonnet]
