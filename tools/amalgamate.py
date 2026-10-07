#!/usr/bin/env python3
"""Flatten one exercise variant into a single translation unit, for Compiler Explorer.

    tools/amalgamate.py exercises/s03-compile-time/solution -o /tmp/s03-solution.cpp

Project headers (`#include "telemetry/x.h"`) are inlined once each, in dependency order;
system includes are hoisted to the top. Then every src/*.cpp, then main.cpp. The result
compiles with the same flags as the CMake build and reads its data from stdin when run
with `-`, which is how the godbolt links feed it data/sample.csv.
"""
import argparse
import pathlib
import re
import sys

LOCAL = re.compile(r'^\s*#include\s+"([^"]+)"')
SYSTEM = re.compile(r'^\s*#include\s+<[^>]+>')


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("variant", type=pathlib.Path)
    ap.add_argument("-o", "--output", type=pathlib.Path, required=True)
    a = ap.parse_args()
    inc = a.variant / "include"
    done: set[pathlib.Path] = set()
    system: list[str] = []
    body: list[str] = []

    def emit(path: pathlib.Path) -> None:
        path = path.resolve()
        if path in done:
            return
        done.add(path)
        text = path.read_text()
        out = [f"// ---- {path.relative_to(a.variant.resolve())} ----"]
        for line in text.splitlines():
            if line.strip() == "#pragma once":
                continue
            m = LOCAL.match(line)
            if m:
                target = inc / m.group(1)
                if not target.exists():
                    target = path.parent / m.group(1)
                emit(target)
                continue
            if SYSTEM.match(line):
                if line.strip() not in system:
                    system.append(line.strip())
                continue
            out.append(line)
        body.append("\n".join(out))

    for src in sorted((a.variant / "src").glob("*.cpp")):
        emit(src)
    emit(a.variant / "main.cpp")

    root = pathlib.Path(__file__).resolve().parent.parent
    variant = a.variant.resolve()
    label = str(variant.relative_to(root)) if variant.is_relative_to(root) else variant.name
    header = [f"// {label}: single-file amalgamation for Compiler Explorer (tools/amalgamate.py).",
              "// Run with argument `-` and data/sample.csv on stdin.", ""]
    a.output.write_text("\n".join(header + sorted(system) + [""] + body) + "\n")
    print(f"wrote {a.output} ({len(done)} files)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
