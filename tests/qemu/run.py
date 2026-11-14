#!/usr/bin/env python3
"""Collect real emulated CPUID, then exercise x86info's replay backend."""

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import re
import subprocess
import time
from typing import NotRequired, TypedDict


ROOT: Path = Path(__file__).resolve().parents[2]
DONE: bytes = b"QEMU CPUID DONE\n"
TRAILING_WS: re.Pattern[bytes] = re.compile(rb"[\t ]+\n")
UNKNOWN: re.Pattern[str] = re.compile(
    r"[^\n]*(?:Unknown CPU|Unknown model|Unknown vendor)[^\n]*", re.IGNORECASE
)
DECODED_OPTIONS: list[str] = [
    "-c",
    "-f",
    "--addr",
    "--bugs",
    "--connector",
    "--all-cpus",
]


class Result(TypedDict):
    model: str
    status: str
    reason: NotRequired[str]
    unknown: NotRequired[list[str]]


def replay(binary: Path, dump: Path, args: list[str], output: Path) -> bytes:
    env: dict[str, str] = dict(os.environ, X86INFO_CPUID_DUMP=str(dump))
    result = subprocess.run(
        [str(binary), *args], env=env, capture_output=True, timeout=30
    )
    output.write_bytes(result.stdout)
    output.with_suffix(output.suffix + ".err").write_bytes(result.stderr)
    if result.returncode or result.stderr:
        raise RuntimeError(
            f"{binary.parent.name} {args}: exit {result.returncode}; see {output}"
        )
    return result.stdout


def collect_cpuid(
    qemu: str, accel: str, model: str, path: Path, timeout: float
) -> bytes:
    """Boot the CPUID collector under QEMU and return its serial output."""
    serial: Path = path / "serial.log"
    serial.unlink(missing_ok=True)
    with (path / "qemu.err").open("wb") as err:
        proc = subprocess.Popen(
            [
                qemu,
                "-machine",
                "microvm",
                "-accel",
                accel,
                "-cpu",
                model,
                "-smp",
                "1",
                "-m",
                "16M",
                "-bios",
                str(ROOT / "tests/build/qemu-rom.bin"),
                "-display",
                "none",
                "-monitor",
                "none",
                "-serial",
                f"file:{serial}",
                "-no-reboot",
            ],
            stdout=subprocess.DEVNULL,
            stderr=err,
        )
        try:
            deadline: float = time.monotonic() + timeout
            while True:
                data: bytes = serial.read_bytes() if serial.exists() else b""
                if data.endswith(DONE):
                    return data.removesuffix(DONE)
                if proc.poll() is not None:
                    raise RuntimeError(
                        "QEMU exited before collecting CPUID; see qemu.err"
                    )
                if time.monotonic() >= deadline:
                    raise RuntimeError(
                        "CPUID collection timed out; see serial.log/qemu.err"
                    )
                time.sleep(0.02)
        finally:
            if proc.poll() is None:
                proc.terminate()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()


def check_replay(path: Path, dump: Path) -> list[bytes]:
    """Replay the dump with both binaries; return the outputs in order."""
    normal: Path = ROOT / "tests/build/x86info"
    sanitized: Path = ROOT / "tests/build-asan/x86info"
    runs: list[tuple[str, list[str]]] = [
        ("default", []),
        ("decoded", DECODED_OPTIONS),
        ("verbose", ["-v", *DECODED_OPTIONS]),
        ("raw", ["-r", "--all-cpus"]),
    ]
    outputs: list[bytes] = []
    for name, args in runs:
        output: bytes = replay(normal, dump, args, path / f"{name}.out")
        asan: bytes = replay(sanitized, dump, args, path / f"{name}.asan.out")
        # Version strings can differ between separately built binaries.
        if output.splitlines()[1:] != asan.splitlines()[1:]:
            raise RuntimeError(f"sanitized output differs for {name}")
        if name == "verbose" and TRAILING_WS.search(output):
            raise RuntimeError("verbose output contains trailing whitespace")
        outputs.append(output)
    return outputs


def check_roundtrip(path: Path, outputs: list[bytes]) -> None:
    """The raw output must replay to itself and keep every decoded result."""
    normal: Path = ROOT / "tests/build/x86info"
    raw: bytes = outputs[-1]
    canonical: Path = path / "canonical.dump"
    canonical.write_bytes(
        b"".join(
            line
            for line in raw.splitlines(keepends=True)
            if line.startswith((b"CPU #", b"eax in:"))
        )
    )
    if not canonical.stat().st_size:
        raise RuntimeError("empty raw dump")
    again: bytes = replay(
        normal, canonical, ["-r", "--all-cpus"], path / "roundtrip.out"
    )
    if again != raw:
        raise RuntimeError("raw CPUID roundtrip differs")
    decoded: bytes = replay(normal, canonical, DECODED_OPTIONS, path / "canonical.out")
    if decoded != outputs[1]:
        raise RuntimeError("canonical dump loses decoded data")


def test_model(
    qemu: str, accel: str, model: str, directory: Path, timeout: float
) -> Result:
    path: Path = directory / model
    path.mkdir(parents=True, exist_ok=True)
    try:
        dump: Path = path / "cpuid.dump"
        dump.write_bytes(collect_cpuid(qemu, accel, model, path, timeout))
        outputs: list[bytes] = check_replay(path, dump)
        check_roundtrip(path, outputs)
        unknown: list[str] = sorted(
            set(UNKNOWN.findall(outputs[1].decode(errors="replace")))
        )
        return {"model": model, "status": "PASS", "unknown": unknown}
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as exc:
        return {"model": model, "status": "FAIL", "reason": str(exc)}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--qemu", default="qemu-system-x86_64")
    parser.add_argument("--accel", choices=("tcg", "kvm"), default="tcg")
    parser.add_argument(
        "--models", nargs="+", help="default: every model from -cpu help"
    )
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--timeout", type=float, default=20)
    parser.add_argument("--output", type=Path, default=ROOT / "tests/build/qemu")
    args = parser.parse_args()
    if args.jobs < 1 or args.timeout <= 0:
        parser.error("jobs and timeout must be positive")
    help_text: str = subprocess.check_output([args.qemu, "-cpu", "help"], text=True)
    accelerators: str = subprocess.check_output(
        [args.qemu, "-accel", "help"], text=True
    )
    if not re.search(rf"^{args.accel}$", accelerators, re.MULTILINE):
        parser.error(
            f"this QEMU lacks {args.accel}; use a binary with that accelerator"
        )
    available: list[str] = re.findall(
        r"^  (\S+)[^\n]*$",
        help_text.split("Recognized CPUID flags:")[0],
        re.MULTILINE,
    )
    models: list[str] = args.models or available
    if any(model not in available for model in models):
        parser.error("requested model is absent from QEMU's -cpu help")
    directory: Path = args.output.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "cpu-help.txt").write_text(help_text)
    version: str = subprocess.check_output([args.qemu, "--version"], text=True)
    (directory / "qemu-version.txt").write_text(version)
    results: list[Result] = []
    # 'host' requires KVM and cannot run under TCG.
    if args.accel == "tcg" and "host" in models:
        models = [model for model in models if model != "host"]
        results.append({"model": "host", "status": "SKIP", "reason": "requires KVM"})
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for result in pool.map(
            lambda model: test_model(
                args.qemu, args.accel, model, directory, args.timeout
            ),
            models,
        ):
            results.append(result)
            detail: str = result.get("reason") or "; ".join(result.get("unknown", []))
            print(
                f"{result['status']} {result['model']}"
                + (f": {detail}" if detail else ""),
                flush=True,
            )
    (directory / "summary.json").write_text(json.dumps(results, indent=2) + "\n")
    counts: dict[str, int] = {
        status: sum(r["status"] == status for r in results)
        for status in ("PASS", "FAIL", "SKIP")
    }
    print(f"{counts}; reports: {directory}")
    return int(bool(counts["FAIL"]))


if __name__ == "__main__":
    raise SystemExit(main())
