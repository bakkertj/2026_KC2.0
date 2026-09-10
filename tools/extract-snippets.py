#!/usr/bin/env python3
"""Keep code on slides in sync with the demo files.

In a demo .cpp file, mark a region:

    // [snippet: name]
    ...code...
    // [/snippet]

In a deck, reference it with a marker line immediately followed by a cpp fence:

    <!-- snippet: demos/s01/spaceship.cpp#name -->
    ```cpp
    (contents are replaced in place)
    ```

Usage: extract-snippets.py slides/01-everyday-language.md [more decks...]
Paths in markers are relative to the repository root. Exits non-zero if a
referenced snippet does not exist, so CI catches stale references.
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
MARKER = re.compile(r"<!--\s*snippet:\s*(\S+?)#(\w+)\s*-->")
START = re.compile(r"//\s*\[snippet:\s*(\w+)\]")
END = re.compile(r"//\s*\[/snippet\]")


def load_snippets(path: pathlib.Path) -> dict[str, str]:
    out, cur, buf = {}, None, []
    for line in path.read_text().splitlines():
        if cur is None:
            m = START.search(line)
            if m:
                cur, buf = m.group(1), []
        elif END.search(line):
            body = "\n".join(buf)
            indent = min((len(l) - len(l.lstrip()) for l in buf if l.strip()), default=0)
            out[cur] = "\n".join(l[indent:] for l in body.splitlines())
            cur = None
        else:
            buf.append(line)
    return out


def process(deck: pathlib.Path) -> int:
    lines = deck.read_text().splitlines()
    out, i, errors = [], 0, 0
    while i < len(lines):
        m = MARKER.search(lines[i])
        if m and i + 1 < len(lines) and lines[i + 1].startswith("```"):
            src = ROOT / m.group(1)
            snippets = load_snippets(src) if src.exists() else {}
            if m.group(2) not in snippets:
                print(f"{deck}:{i+1}: missing snippet {m.group(1)}#{m.group(2)}", file=sys.stderr)
                errors += 1
                out.append(lines[i]); i += 1; continue
            out.append(lines[i]); out.append(lines[i + 1])
            out.append(snippets[m.group(2)])
            j = i + 2
            while j < len(lines) and not lines[j].startswith("```"):
                j += 1
            out.append("```")
            i = j + 1
        else:
            out.append(lines[i]); i += 1
    deck.write_text("\n".join(out) + "\n")
    return errors


if __name__ == "__main__":
    total = sum(process(pathlib.Path(p)) for p in sys.argv[1:])
    sys.exit(1 if total else 0)
