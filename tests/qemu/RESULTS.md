Sweep on 2026-10-29, using QEMU 10.0.13 (Debian
`1:10.0.13+ds-0+deb13u1`) and x86info revision `37b931c`:

- All 153 TCG-compatible entries from `-cpu help` passed, including aliases
  and versioned models. No crashes, sanitizer diagnostics, output mismatches,
  trailing whitespace or raw dump roundtrip failures were found.
- The remaining entry, `host`, passed separately with
  `qemu-system-x86_64-microvm` and KVM. Its brand string was
  `AMD Ryzen AI 9 HX 470 w/ Radeon 890M`; x86info identified
  `AMD Zen5/Zen6 Series Processor (Strix Point/Gorgon Point)`.
- The existing `make check` suite also passed.

Twenty-one TCG entries reported an unknown CPU/model/vendor. Grouping aliases
and versions gives the following cases:

| QEMU models | Entries | x86info result |
| --- | ---: | --- |
| Haswell and its aliases/versions | 8 | Unknown model |
| Penryn and Penryn-v1 | 2 | Unknown model |
| n270 and n270-v1 | 2 | Unknown model |
| Opteron_G1/G2 and their v1 versions | 4 | Unknown CPU |
| kvm32/kvm64 and their v1 versions | 4 | Unknown CPU |
| base | 1 | Unknown vendor (empty vendor string) |

These are observations for the one-vCPU emulated configuration, not proof
that the corresponding physical CPUs are unrecognized. In particular, the
Intel naming table sometimes requires an exact core count, stepping or
frequency match. QEMU's configurable topology and synthetic CPUID values
can miss those matches. `base` intentionally has no features enabled.

The retained `tests/build/qemu/summary.json` and `tests/build/qemu-kvm/summary.json`
contain individual results; their sibling per-model directories contain raw
and canonical dumps, decoded output and QEMU warnings. See [README.md](README.md)
for reproduction and coverage limits.
