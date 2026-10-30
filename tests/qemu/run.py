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


ROOT = Path(__file__).resolve().parents[2]
DONE = b"QEMU CPUID DONE\n"


def replay(binary, dump, args, output):
    env = dict(os.environ, X86INFO_CPUID_DUMP=str(dump))
    result = subprocess.run(
        [str(binary), *args], env=env, capture_output=True, timeout=30
    )
    output.write_bytes(result.stdout)
    output.with_suffix(output.suffix + ".err").write_bytes(result.stderr)
    if result.returncode or result.stderr:
        raise RuntimeError(f"{binary.parent.name} {args}: exit {result.returncode}; "
                           f"see {output}")
    return result.stdout


def test_model(qemu, accel, model, directory, timeout):
    path = directory / model
    path.mkdir(parents=True, exist_ok=True)
    serial = path / "serial.log"
    try:
        serial.unlink(missing_ok=True)
        with (path / "qemu.err").open("wb") as err:
            proc = subprocess.Popen(
                [qemu, "-machine", "microvm", "-accel", accel, "-cpu", model,
                 "-smp", "1", "-m", "16M", "-bios",
                 str(ROOT / "tests/build/qemu-rom.bin"), "-display", "none",
                 "-monitor", "none", "-serial", f"file:{serial}", "-no-reboot"],
                stdout=subprocess.DEVNULL, stderr=err,
            )
            try:
                deadline = time.monotonic() + timeout
                while True:
                    data = serial.read_bytes() if serial.exists() else b""
                    if data.endswith(DONE):
                        break
                    if proc.poll() is not None:
                        raise RuntimeError("QEMU exited before collecting CPUID; see qemu.err")
                    if time.monotonic() >= deadline:
                        raise RuntimeError("CPUID collection timed out; see serial.log/qemu.err")
                    time.sleep(0.02)
            finally:
                if proc.poll() is None:
                    proc.terminate()
                try:
                    proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    proc.kill()
                    proc.wait()

        dump = path / "cpuid.dump"
        dump.write_bytes(data.removesuffix(DONE))
        normal = ROOT / "tests/build/x86info"
        sanitized = ROOT / "tests/build-asan/x86info"
        options = ["-c", "-f", "--addr", "--bugs", "--connector", "--all-cpus"]
        outputs = []
        for name, args in [("default", []), ("decoded", options),
                           ("verbose", ["-v", *options]),
                           ("raw", ["-r", "--all-cpus"])]:
            output = replay(normal, dump, args, path / f"{name}.out")
            asan = replay(sanitized, dump, args, path / f"{name}.asan.out")
            # Version strings can differ between separately built binaries.
            if output.splitlines()[1:] != asan.splitlines()[1:]:
                raise RuntimeError(f"sanitized output differs for {name}")
            if name == "verbose" and re.search(rb"[\t ]+\n", output):
                raise RuntimeError("verbose output contains trailing whitespace")
            outputs.append(output)

        canonical = path / "canonical.dump"
        lines = outputs[-1].splitlines(keepends=True)
        canonical.write_bytes(b"".join(line for line in lines
                                      if line.startswith((b"CPU #", b"eax in:"))))
        if not canonical.stat().st_size:
            raise RuntimeError("empty raw dump")
        again = replay(normal, canonical, ["-r", "--all-cpus"], path / "roundtrip.out")
        if again != outputs[-1]:
            raise RuntimeError("raw CPUID roundtrip differs")
        # Canonicalizing must preserve every decoded result too.
        decoded = replay(normal, canonical, options, path / "canonical.out")
        if decoded != outputs[1]:
            raise RuntimeError("canonical dump loses decoded data")
        unknown = sorted(set(re.findall(
            r"[^\n]*(?:Unknown CPU|Unknown model|Unknown vendor)[^\n]*",
            outputs[1].decode(errors="replace"), re.IGNORECASE)))
        return {"model": model, "status": "PASS", "unknown": unknown}
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as exc:
        return {"model": model, "status": "FAIL", "reason": str(exc)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--qemu", default="qemu-system-x86_64")
    parser.add_argument("--accel", choices=("tcg", "kvm"), default="tcg")
    parser.add_argument("--models", nargs="+", help="default: every model from -cpu help")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--timeout", type=float, default=20)
    parser.add_argument("--output", type=Path, default=ROOT / "tests/build/qemu")
    args = parser.parse_args()
    if args.jobs < 1 or args.timeout <= 0:
        parser.error("jobs and timeout must be positive")
    help_text = subprocess.check_output([args.qemu, "-cpu", "help"], text=True)
    accelerators = subprocess.check_output([args.qemu, "-accel", "help"], text=True)
    if not re.search(rf"^{args.accel}$", accelerators, re.MULTILINE):
        parser.error(f"this QEMU lacks {args.accel}; use a binary with that accelerator")
    available = re.findall(r"^  (\S+)[^\n]*$", help_text.split(
        "Recognized CPUID flags:")[0], re.MULTILINE)
    models = args.models or available
    if any(model not in available for model in models):
        parser.error("requested model is absent from QEMU's -cpu help")
    directory = args.output.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "cpu-help.txt").write_text(help_text)
    version = subprocess.check_output([args.qemu, "--version"], text=True)
    (directory / "qemu-version.txt").write_text(version)
    results = []
    # 'host' requires KVM and cannot run under TCG.
    if args.accel == "tcg" and "host" in models:
        models = [model for model in models if model != "host"]
        results.append({"model": "host", "status": "SKIP", "reason": "requires KVM"})
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for result in pool.map(lambda model: test_model(
                args.qemu, args.accel, model, directory, args.timeout), models):
            results.append(result)
            detail = result.get("reason") or "; ".join(result.get("unknown", []))
            print(f"{result['status']} {result['model']}" +
                  (f": {detail}" if detail else ""), flush=True)
    (directory / "summary.json").write_text(json.dumps(results, indent=2) + "\n")
    counts = {status: sum(r["status"] == status for r in results)
              for status in ("PASS", "FAIL", "SKIP")}
    print(f"{counts}; reports: {directory}")
    return int(bool(counts["FAIL"]))


if __name__ == "__main__":
    raise SystemExit(main())
