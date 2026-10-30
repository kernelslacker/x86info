# QEMU CPUID captures

Captured on 2026-10-29 using QEMU 10.0.13 (Debian
`1:10.0.13+ds-0+deb13u1`) and the collector in `tests/qemu/`.
These 74 dumps cover all 154 entries from `-cpu help`: 153 entries under
TCG and `host` under KVM. Byte-identical captures share a fixture; versioned
model names are preferred over aliases when naming those fixtures.

Each capture configures one virtual CPU on the microvm machine. TCG filters
features it cannot emulate, so these dumps describe QEMU's exposed CPUID,
not the complete physical processor specification. Some model versions
therefore have identical dumps. The KVM host capture exposes the features
available to that virtual CPU on an AMD Ryzen AI 9 HX 470 host.

The `.dump` files use the canonical `x86info -r --all-cpus` format. Matching
`.out` and `.v.out` files were generated using the existing replay runner
and preserve the current decoder behavior, including unknown model names.
These goldens are regression baselines, not independently verified hardware
identifications. See `../qemu/README.md` for collection and sweep details.

| Dump (without suffix) | QEMU models with identical captures |
| --- | --- |
| `amd-qemu-athlon-v1` | `athlon`, `athlon-v1` |
| `amd-qemu-epyc-genoa-v1` | `EPYC-Genoa`, `EPYC-Genoa-v1` |
| `amd-qemu-epyc-milan-v1` | `EPYC-Milan-v1` |
| `amd-qemu-epyc-milan-v2` | `EPYC-Milan`, `EPYC-Milan-v2` |
| `amd-qemu-epyc-rome-v1` | `EPYC-Rome-v1`, `EPYC-Rome-v2` |
| `amd-qemu-epyc-rome-v3` | `EPYC-Rome-v3` |
| `amd-qemu-epyc-rome-v4` | `EPYC-Rome`, `EPYC-Rome-v4` |
| `amd-qemu-epyc-v1` | `EPYC-v1` |
| `amd-qemu-epyc-v2` | `EPYC-IBPB`, `EPYC-v2` |
| `amd-qemu-epyc-v3` | `EPYC-v3` |
| `amd-qemu-epyc-v4` | `EPYC`, `EPYC-v4` |
| `amd-qemu-host-ryzen-ai9-hx470` | `host (KVM)` |
| `amd-qemu-max` | `max` |
| `amd-qemu-opteron-g1-v1` | `Opteron_G1`, `Opteron_G1-v1` |
| `amd-qemu-opteron-g2-v1` | `Opteron_G2`, `Opteron_G2-v1` |
| `amd-qemu-opteron-g3-v1` | `Opteron_G3`, `Opteron_G3-v1` |
| `amd-qemu-opteron-g4-v1` | `Opteron_G4`, `Opteron_G4-v1` |
| `amd-qemu-opteron-g5-v1` | `Opteron_G5`, `Opteron_G5-v1` |
| `amd-qemu-phenom-v1` | `phenom`, `phenom-v1` |
| `amd-qemu-qemu64-v1` | `qemu64`, `qemu64-v1` |
| `centaur-qemu-yongfeng-v1` | `YongFeng-v1` |
| `centaur-qemu-yongfeng-v2` | `YongFeng`, `YongFeng-v2` |
| `hygon-qemu-dhyana-v1` | `Dhyana`, `Dhyana-v1`, `Dhyana-v2` |
| `intel-qemu-486-v1` | `486`, `486-v1` |
| `intel-qemu-broadwell-v1` | `Broadwell-v1` |
| `intel-qemu-broadwell-v2` | `Broadwell-noTSX`, `Broadwell-v2` |
| `intel-qemu-broadwell-v3` | `Broadwell-IBRS`, `Broadwell-v3` |
| `intel-qemu-broadwell-v4` | `Broadwell`, `Broadwell-noTSX-IBRS`, `Broadwell-v4` |
| `intel-qemu-cascadelake-server-v1` | `Cascadelake-Server`, `Cascadelake-Server-noTSX`, `Cascadelake-Server-v1`, `Cascadelake-Server-v2`, `Cascadelake-Server-v3`, `Cascadelake-Server-v4`, `Cascadelake-Server-v5` |
| `intel-qemu-clearwaterforest-v1` | `ClearwaterForest`, `ClearwaterForest-v1` |
| `intel-qemu-conroe-v1` | `Conroe`, `Conroe-v1` |
| `intel-qemu-cooperlake-v1` | `Cooperlake`, `Cooperlake-v1`, `Cooperlake-v2` |
| `intel-qemu-core2duo-v1` | `core2duo`, `core2duo-v1` |
| `intel-qemu-coreduo-v1` | `coreduo`, `coreduo-v1` |
| `intel-qemu-denverton-v1` | `Denverton-v1` |
| `intel-qemu-denverton-v2` | `Denverton`, `Denverton-v2`, `Denverton-v3` |
| `intel-qemu-graniterapids-v1` | `GraniteRapids-v1` |
| `intel-qemu-graniterapids-v2` | `GraniteRapids`, `GraniteRapids-v2` |
| `intel-qemu-haswell-v1` | `Haswell-v1` |
| `intel-qemu-haswell-v2` | `Haswell-noTSX`, `Haswell-v2` |
| `intel-qemu-haswell-v3` | `Haswell-IBRS`, `Haswell-v3` |
| `intel-qemu-haswell-v4` | `Haswell`, `Haswell-noTSX-IBRS`, `Haswell-v4` |
| `intel-qemu-icelake-server-v1` | `Icelake-Server-noTSX`, `Icelake-Server-v1`, `Icelake-Server-v2`, `Icelake-Server-v3` |
| `intel-qemu-icelake-server-v4` | `Icelake-Server`, `Icelake-Server-v4`, `Icelake-Server-v5`, `Icelake-Server-v6`, `Icelake-Server-v7` |
| `intel-qemu-ivybridge-v1` | `IvyBridge-v1` |
| `intel-qemu-ivybridge-v2` | `IvyBridge`, `IvyBridge-IBRS`, `IvyBridge-v2` |
| `intel-qemu-knightsmill-v1` | `KnightsMill`, `KnightsMill-v1` |
| `intel-qemu-kvm32-v1` | `kvm32`, `kvm32-v1` |
| `intel-qemu-kvm64-v1` | `kvm64`, `kvm64-v1` |
| `intel-qemu-n270-v1` | `n270`, `n270-v1` |
| `intel-qemu-nehalem-v1` | `Nehalem-v1` |
| `intel-qemu-nehalem-v2` | `Nehalem`, `Nehalem-IBRS`, `Nehalem-v2` |
| `intel-qemu-penryn-v1` | `Penryn`, `Penryn-v1` |
| `intel-qemu-pentium-v1` | `pentium`, `pentium-v1` |
| `intel-qemu-pentium2-v1` | `pentium2`, `pentium2-v1` |
| `intel-qemu-pentium3-v1` | `pentium3`, `pentium3-v1` |
| `intel-qemu-qemu32-v1` | `qemu32`, `qemu32-v1` |
| `intel-qemu-sandybridge-v1` | `SandyBridge-v1` |
| `intel-qemu-sandybridge-v2` | `SandyBridge`, `SandyBridge-IBRS`, `SandyBridge-v2` |
| `intel-qemu-sapphirerapids-v1` | `SapphireRapids-v1`, `SapphireRapids-v2` |
| `intel-qemu-sapphirerapids-v3` | `SapphireRapids`, `SapphireRapids-v3` |
| `intel-qemu-sierraforest-v1` | `SierraForest-v1` |
| `intel-qemu-sierraforest-v2` | `SierraForest`, `SierraForest-v2` |
| `intel-qemu-skylake-client-v1` | `Skylake-Client-v1` |
| `intel-qemu-skylake-client-v2` | `Skylake-Client-IBRS`, `Skylake-Client-v2` |
| `intel-qemu-skylake-client-v3` | `Skylake-Client`, `Skylake-Client-noTSX-IBRS`, `Skylake-Client-v3`, `Skylake-Client-v4` |
| `intel-qemu-skylake-server-v1` | `Skylake-Server-v1` |
| `intel-qemu-skylake-server-v2` | `Skylake-Server-IBRS`, `Skylake-Server-v2` |
| `intel-qemu-skylake-server-v3` | `Skylake-Server`, `Skylake-Server-noTSX-IBRS`, `Skylake-Server-v3`, `Skylake-Server-v4`, `Skylake-Server-v5` |
| `intel-qemu-snowridge-v1` | `Snowridge-v1` |
| `intel-qemu-snowridge-v2` | `Snowridge`, `Snowridge-v2`, `Snowridge-v3`, `Snowridge-v4` |
| `intel-qemu-westmere-v1` | `Westmere-v1` |
| `intel-qemu-westmere-v2` | `Westmere`, `Westmere-IBRS`, `Westmere-v2` |
| `unknown-qemu-base` | `base` |
