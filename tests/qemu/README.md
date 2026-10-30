Run `make check-qemu` to test every CPU model and alias listed by
`qemu-system-x86_64 -cpu help`. This optional suite needs Python 3.9+, QEMU
with TCG and the microvm machine, GCC-compatible compilation with `-m32`,
and GNU ld/objcopy. It needs neither root nor a guest OS nor 32-bit libc.
Use `QEMU=/path/to/qemu-system-x86_64` to select another QEMU binary.

A 64 KiB boot ROM enters protected mode and collects CPUID through a serial
port. Its i486 instruction baseline allows testing models that cannot boot a
modern Linux guest. Collection includes basic, extended, hypervisor and
Centaur ranges, plus indexed subleaves (bounded to 64). Only the bootstrap
CPU is sampled, with one virtual CPU configured.

Each dump runs through normal and ASan/UBSan replay builds with default,
feature/cache/address/bug/connector, verbose and raw options. The suite checks
exit status, sanitizer diagnostics, matching output between builds, trailing
whitespace, raw dump roundtrips and preservation of decoded output when the
dump is canonicalized. Unknown model/vendor names are reported separately;
they do not fail the smoke test. This is not a golden correctness oracle.

Reports, dumps, QEMU warnings and output go into `tests/build/qemu/`, with a
machine-readable `summary.json`. These are ignored build artifacts. QEMU's
TCG accelerator filters unsupported features, so the collected leaves reflect
what the emulator exposes, not the full physical processor specification.
This tests CPUID decoding through replay, not guest Linux integration, MSRs,
timing or multi-CPU discovery. `host` requires KVM and is skipped under TCG.

After building with `make check-qemu`, selected models can be rerun with:

```sh
python3 tests/qemu/run.py --models Haswell-v1 EPYC-Genoa-v1
python3 tests/qemu/run.py --accel kvm --models host --output tests/build/qemu-kvm
```

`--jobs`, `--timeout` (seconds per collection), `--qemu` and `--output` are
also available. The microvm-only QEMU binary in some distributions supports
only KVM; select the full system binary for the default TCG sweep.
