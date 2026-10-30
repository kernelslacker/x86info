# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and test

Plain C, plain Makefile (`flags.mk` holds the shared warning flags; the build is warning-heavy, keep it clean).

- `make` builds `x86info` and `lsmsr` (separate Makefile in `lsmsr/`).
- libpci is optional (found via pkg-config, used for AMD `--pm`); `make LIBPCI=` builds without it.
- `make check` runs everything: unit tests (`tests/test-{psb,opts,mptable,cpulist}.c`), `tests/test-runner.sh` (tests the golden runner itself), then the golden tests.
- `make check-asan` repeats the unit tests and goldens under ASan/UBSan.
- No single-test target. For one unit test: `make tests/build/test-opts && tests/build/test-opts`. For one golden: copy the loop body in `tests/run-tests.sh`, i.e. `X86INFO_CPUID_DUMP=tests/dumps/<vendor>/<name>.dump tests/build/x86info -c -f --addr --bugs --connector | tail -n +2 | diff tests/dumps/<vendor>/<name>.out -` (`-v` and `<name>.v.out` for the verbose golden).
- `make clean` removes `tests/build*`, `.deps`, objects.

## Golden tests (the main regression suite)

`make check` builds a second binary, `tests/build/x86info`, with `-DCPUID_REPLAY`. That swaps the live CPUID/MSR backends (`cpuid-linux.c`, `cpudev-linux.c`, `rdmsr-linux.c`, `host.c`) for `cpuid-replay.c`, which reads the dump named by `$X86INFO_CPUID_DUMP`.

Each `tests/dumps/<vendor>/<name>.dump` has `<name>.out` (default options) and `<name>.v.out` (`-v`) goldens; the runner also checks that `-r --all-cpus` output replays to an identical dump, and that verbose output has no trailing whitespace. The first line (version) is stripped before diffing.

- Regenerate goldens: `UPDATE=1 tests/run-tests.sh tests/build/x86info`, then check `git status` shows only the intended files changed, and review the diff.
- Add a machine: on the target host run `x86info -r --all-cpus` as root (needs the `cpuid` module, `modprobe cpuid`) and keep only the `CPU #n:` / `eax in:` lines (that is what `make record VENDOR=<vendor> NAME=<name>` does), save as `tests/dumps/<vendor>/<name>.dump`, then generate goldens as above. Sanity-check the output against the real CPU before committing; the live `-v` output should match the replay golden.
- Dumps are grouped under `tests/dumps/<vendor>/` (`amd`, `intel`, etc.); AMD Zen fixtures also live under `amd/`. Keep each dump and both goldens together.
- Dump names are `<vendor>-<uarch>-<part>` (e.g. `intel-silvermont-j1800`) or `zen5-...` for AMD synthetic cases.

## Architecture

`x86info.c` drives a per-CPU pass: for each CPU it fills a `struct cpudata` (`include/x86info.h`) from CPUID, then dispatches by vendor.

- **Vendor dispatch**: `identify.c` reads the vendor string and calls into `vendors/<vendor>/` (`amd`, `intel`, `centaur`, `cyrix`, `natsemi`, `rise`, `sis`, `misc`). Each vendor directory holds its own model identification, cache decoding, MSR dumping (`msr-*.c`), bugs and machine-check decoding. Intel family 6 identification is split across several `identify-family6*.c` files.
- **Backends**: CPUID and MSR access are per-OS files (`cpuid-linux.c`, `cpuid-freebsd.c`, `rdmsr-*.c`, ...) behind `native_cpuid` / the helpers in `cpuid.c`. Linux reads `/dev/cpu/N/{cpuid,msr}` (root only) and falls back to executing CPUID on the current CPU. The BSD/Solaris backends are lightly tested.
- **Topology**: `topology.c`, `apic.c`, `cpulist.c` and `mptable.c` build the socket/core/thread summary (leaf 0xB/0x1F where available, legacy leaves otherwise); AMD TopoExt handling is in `vendors/amd/identify.c`.
- **Output**: default output is the identification + cache/TLB + feature flag listing per CPU, collapsed to "N identical CPUs" in `-v` mode; flag naming lives in `features.c`.
- `docs/sspecs.md` holds S-spec/stepping notes moved out of the identification code comments.

## Conventions

- Kernel-style C (tabs, `/* */` comments), tight `-W` warning set including `-Wdeclaration-after-statement`, `-Wshadow`, `-Wswitch-enum`.
- Version string comes from `git describe`; it appears in the first output line (hence stripped from goldens).
