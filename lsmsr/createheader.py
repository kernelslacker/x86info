#!/usr/bin/python3

"""Create C header files from register description files.

Author: Andreas Herrmann <andreas.herrmann3@amd.com>

Copyright (C) 2008 Advanced Micro Devices, Inc.

Licensed under the terms of the GNU GENERAL PUBLIC LICENSE version 2.
See file COPYING for details.

The general format for those register descriptions is

    {<name>=<address>;[<description>]
    [<field name>]:<bit width>[;<value>=<string>|]
    [<field name>]:<bit width>[;<value>=<string>|]
    ...
    [<field name>]:<bit width>[;<value>=<string>|]
    }

If no field name is provided the field is reserved and might not be
displayed with the msr viewing tool. The optional <value>=<string> data is
for bit field decoding but it's currently not yet supported.

Todos/Nice-to-have:
- create information for bit field decoding
- warn if names contain whitespace characters
- add include/insert statement to avoid dup definitions of common register
- allow annotations for registers (e.g. write-only)
"""

import sys
from pathlib import Path

REGISTER_BITS: int = 64

HEADER: str = """/*
 * Licensed under the terms of the GNU GENERAL PUBLIC LICENSE version 2.
 * See file COPYING for details.
 */
"""


def usage(progname: str) -> None:
    """Print the command line help."""
    print(progname + ": invalid number of arguments")
    print("\nUSAGE:")
    print(progname + " <input file> <struct name>")
    print("\nEXAMPLE:")
    print(progname + " k8regs.txt k8 >k8.h")


class Register:
    """One register description: its header and its bit fields."""

    def __init__(self, line: str) -> None:
        head = line.split("{")[1].split("=")
        rest = head[1].split(";")
        self.name: str = head[0].strip()
        self.addr: str = rest[0].strip()
        self.desc: str = rest[1].strip() if len(rest) > 1 else ""
        self.fields: list[tuple[str, str]] = []

    def add_field(self, line: str) -> None:
        """Add a '<name>:<width>' field line."""
        field = line.split(";")[0].split(":")
        self.fields.append((field[0].strip(), field[1].strip()))

    def macros(self, family: str) -> tuple[str, str, str]:
        """Return the _RANGE, _NAMES and _SPEC macro invocations."""
        width = sum(int(fwidth) for _, fwidth in self.fields)
        if width != REGISTER_BITS:
            msg = f"{self.name}: fields add up to {width} bits"
            raise ValueError(f"{msg}, not {REGISTER_BITS}")
        ranges = "_RANGE(" + family + "_" + self.name
        names = "_NAMES(" + family + "_" + self.name
        for fname, fwidth in self.fields:
            names += ',"' + fname + '"' if fname else ",0"
            ranges += "," + fwidth
        desc = '"' + self.desc + '"' if self.desc else "NULL"
        spec = f"_SPEC({self.addr}, {self.name}, {desc}, {family}_)"
        return ranges + ",0);", names + ");", spec


def parse_registers(path: Path) -> list[Register]:
    """Read all register descriptions from a file."""
    regs: list[Register] = []
    reg: Register | None = None

    with path.open() as f:
        for line in f:
            if line.startswith("#"):
                continue
            if line.startswith("{"):
                reg = Register(line)
            if line.startswith("}") and reg:
                regs.append(reg)
            if ":" in line and reg:
                reg.add_field(line)
    return regs


def main(argv: list[str]) -> int:
    """Write the header for argv[1] with struct name argv[2] to stdout."""
    progname: str = Path(argv[0]).name
    if len(argv) < 3:
        usage(progname)
        return 1

    family: str = argv[2]
    specs: list[str] = []
    lines: list[str] = []

    try:
        for reg in parse_registers(Path(argv[1])):
            ranges, names, spec = reg.macros(family)
            lines += [ranges, names]
            specs.append(spec)
    except ValueError as err:
        print(f"{progname}: {argv[1]}: {err}", file=sys.stderr)
        return 1

    print(HEADER)
    print("#ifndef " + family + "_h")
    print("#define " + family + "_h\n")
    print('#include "msr.h"\n')
    for line in lines:
        print(line)
    print("\nstruct reg_spec " + family + "_spec [] = {")
    for spec in specs:
        print("\t" + spec + ",")
    print("\t{0, NULL, NULL, NULL, NULL},")
    print("};\n")
    print("#endif /* " + family + "_h */")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
