# x86info TODO

## 🟡 Deprecated/Obsolete API Usage

E3. cpuid-solaris.c — bind_cpu is a FIXME stub and there is no rdmsr
    backend, so MSR output and per-CPU cpuid don't work. Implement them
    (processor_bind(), /dev/cpu/N/msr) or drop the README claim. [sonnet]

## 🟡 Tests & docs

I1. No tests. Identification is a pure function of CPUID values. [opus]
    1. Add a compile-time-selectable cpuid backend that replays a recorded
       dump (the `x86info -r` format, or kernel /dev/cpu dumps).
    2. Check in dumps for a handful of CPUs (K7, K8, Zen 2/3/4/5, Core 2,
       Nehalem, Alder Lake hybrid, VIA) with expected name/cache/topology
       output.
    3. `make check` diffs output against goldens.
    This would have caught A1, A2, A6, A8, A9.
