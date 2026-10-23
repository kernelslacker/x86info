# x86info TODO

## 🔴 Correctness

## 🟡 Performance

## 🟡 Maintainability

## 🟡 Clean-ups/Refactoring/Style

## 🟡 Deprecated/Obsolete API Usage

## 🟡 Better API Alternatives

## 🟡 Architectural improvements

## 🟡 Comments

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

## 🟢 Minor nits / one-liners

