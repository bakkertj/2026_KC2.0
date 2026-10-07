#!/usr/bin/env python3
"""Compiler Explorer links for every demo.

Two kinds of link:

  long   https://godbolt.org/clientstate/<base64 JSON>
         Self-contained: the source, compiler, flags and an execution pane are all
         encoded in the URL. Works offline, needs no API, a few KB long. Fine for the
         handout index, too long for a slide or a source comment.

  short  https://godbolt.org/z/xxxxxxx
         Produced by POSTing the same state to godbolt's shortener. Needs network
         access to godbolt.org (run --shorten from a machine that has it).

Usage:
  tools/godbolt-links.py                 # write handouts/compiler-explorer-links.md with long links
  tools/godbolt-links.py --shorten       # also shorten, cache in tools/godbolt-links.json, and
                                         # write the short link into each demo's header comment
  tools/godbolt-links.py --check         # exit non-zero if any demo still has <add short link>
  tools/godbolt-links.py --dump f.json   # write the client states so something else can shorten them;
                                         # put the results in tools/godbolt-links.json as {key: url}

Per-demo settings come from the file itself:
  - a `#ifdef SHOW_ERRORS` anywhere      -> a second link with -DSHOW_ERRORS
  - `// godbolt: <extra flags>`          -> appended to the compile options
  - `// godbolt: skip`                   -> no link (multi-file demos, TBB-dependent demos)
"""
from __future__ import annotations

import argparse
import base64
import json
import pathlib
import re
import sys
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parent.parent
DEMOS = ROOT / "demos"
INDEX = ROOT / "handouts" / "compiler-explorer-links.md"
CACHE = ROOT / "tools" / "godbolt-links.json"

COMPILER = "g142"                       # x86-64 gcc 14.2 on godbolt.org
BASE_OPTIONS = "-std=c++23 -O2 -Wall -Wextra -Wpedantic"
PLACEHOLDER = "<add short link>"
HEADER_RE = re.compile(r"^// Compiler Explorer: .*$", re.M)
HINT_RE = re.compile(r"^// godbolt: (.*)$", re.M)
TITLE_RE = re.compile(r"^// Demo: (.*)$", re.M)


def client_state(source: str, options: str, arguments: str = "", stdin: str = "") -> dict:
    """The JSON godbolt reads from /clientstate/ URLs: one editor, one compiler, one executor."""
    return {
        "sessions": [{
            "id": 1,
            "language": "c++",
            "source": source,
            "compilers": [{
                "id": COMPILER,
                "options": options,
                "libs": [],
                "filters": {"execute": True, "intel": True, "demangle": True,
                            "labels": True, "directives": True, "commentOnly": True},
            }],
            "executors": [{
                "compiler": {"id": COMPILER, "options": options, "libs": []},
                # compilerVisible: show the executor's own compiler and flags row, so adding
                # -fsanitize=address or -DSHOW_ERRORS there is obvious (the assembly pane's flags
                # do not reach the executor).
                "compilerVisible": True, "compilerOutputVisible": False,
                "arguments": arguments, "argumentsVisible": bool(arguments),
                "stdin": stdin, "stdinVisible": bool(stdin),
            }],
        }]
    }


def long_url(state: dict) -> str:
    raw = json.dumps(state, separators=(",", ":")).encode()
    return "https://godbolt.org/clientstate/" + base64.urlsafe_b64encode(raw).decode()


def shorten(state: dict) -> str:
    req = urllib.request.Request(
        "https://godbolt.org/api/shortener",
        data=json.dumps(state).encode(),
        headers={"Content-Type": "application/json", "Accept": "application/json"},
    )
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.load(resp)["url"]


def variants(path: pathlib.Path) -> list[tuple[str, str]]:
    """(label, options) pairs for one demo, or [] when it is marked skip."""
    text = path.read_text()
    extra = " ".join(m.strip() for m in HINT_RE.findall(text))
    if "skip" in extra.split():
        return []
    options = (BASE_OPTIONS + " " + extra).strip()
    out = [("run", options)]
    if "SHOW_ERRORS" in text:
        out.append(("errors", options + " -DSHOW_ERRORS"))
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--shorten", action="store_true")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--dump", metavar="FILE", help="write {key: client state} JSON for an external shortener run")
    args = ap.parse_args()
    dump: dict[str, dict] = {}

    cache = json.loads(CACHE.read_text()) if CACHE.exists() else {}
    demos = sorted(DEMOS.glob("s0*/*.cpp"))

    if args.check:
        missing = [p for p in demos if PLACEHOLDER in p.read_text() and variants(p)]
        for p in missing:
            print(f"{p.relative_to(ROOT)}: no Compiler Explorer link", file=sys.stderr)
        return 1 if missing else 0

    rows: dict[str, list[tuple[str, str, list[tuple[str, str]]]]] = {}
    for p in demos:
        session = p.parent.name
        text = p.read_text()
        title = (TITLE_RE.search(text) or [None, p.stem])[1]
        links = []
        for label, options in variants(p):
            # The source in the link is the file with the header lines dropped and
            # the snippet markers removed, so students see clean code.
            src = "\n".join(l for l in text.splitlines()
                            if not l.startswith("// Compiler Explorer:")
                            and not l.startswith("// Session:")
                            and "[snippet:" not in l and "[/snippet]" not in l
                            and not HINT_RE.match(l)) + "\n"
            state = client_state(src, options)
            key = f"{p.relative_to(ROOT)}#{label}"
            dump[key] = state
            url = cache.get(key)
            if args.shorten and not url:
                try:
                    url = cache[key] = shorten(state)
                    print(f"{key}: {url}")
                except Exception as e:                      # noqa: BLE001
                    print(f"{key}: shortener failed ({e}); using long link", file=sys.stderr)
            links.append((label, url or long_url(state)))
        rows.setdefault(session, []).append((p.name, title, links))

        # Write the "run" link into the header comment when it is short enough to read.
        if links and links[0][1].startswith("https://godbolt.org/z/"):
            new = HEADER_RE.sub(f"// Compiler Explorer: {links[0][1]}", text, count=1)
            if new != text:
                p.write_text(new)

    # The exercise programs: each starter and solution flattened to one file (tools/amalgamate.py),
    # run with `-` and data/sample.csv on stdin, so the whole telemetry processor runs on godbolt.
    import subprocess, tempfile
    exercises: list[tuple[str, str, str]] = []
    for ex in sorted((ROOT / "exercises").glob("s0*-*")):
        for variant in ("starter", "solution"):
            if not (ex / variant / "main.cpp").exists():
                continue
            with tempfile.NamedTemporaryFile(suffix=".cpp") as tmp:
                subprocess.run([sys.executable, str(ROOT / "tools" / "amalgamate.py"), str(ex / variant),
                                "-o", tmp.name], check=True, capture_output=True)
                source = pathlib.Path(tmp.name).read_text()
            std = "11" if (ex.name.startswith("s01") and variant == "starter") else "23"
            options = BASE_OPTIONS.replace("-std=c++23", f"-std=c++{std}")
            state = client_state(source, options, arguments="-", stdin=(ex / "data" / "sample.csv").read_text())
            key = f"exercises/{ex.name}/{variant}"
            dump[key] = state
            url = cache.get(key)
            if args.shorten and not url:
                try:
                    url = cache[key] = shorten(state)
                    print(f"{key}: {url}")
                except Exception as e:                      # noqa: BLE001
                    print(f"{key}: shortener failed ({e}); using long link", file=sys.stderr)
            exercises.append((ex.name, variant, url or long_url(state)))

    if args.dump:
        pathlib.Path(args.dump).write_text(json.dumps(dump))
    if args.shorten:
        CACHE.write_text(json.dumps(cache, indent=1, sort_keys=True) + "\n")

    with INDEX.open("w") as f:
        f.write("# Compiler Explorer links\n\n")
        f.write("One link per demo, preconfigured for x86-64 GCC 14.2 with "
                f"`{BASE_OPTIONS}` and an execution pane, so the program's output appears "
                "under the assembly. Click **run**; where a demo has deliberate errors behind "
                "`SHOW_ERRORS`, **errors** opens the same file with `-DSHOW_ERRORS` so you can "
                "read the diagnostics. Generated by `tools/godbolt-links.py`; do not edit by hand.\n\n")
        f.write("Demos that need more than one file or a library godbolt does not provide "
                "(`demos/s05/modules`, `demos/s04/parallel.cpp`) are built from the repo instead.\n\n")
        for session in sorted(rows):
            f.write(f"## Session {session[-1]}\n\n")
            # Long dashed separators: pandoc then gives the columns proportional widths
            # (30/50/20) and the table spans the page in the DOCX/PDF export.
            f.write("| Demo | What it shows | Open |\n|" + "-" * 30 + "|" + "-" * 50 + "|" + "-" * 20 + "|\n")
            for name, title, links in rows[session]:
                cell = " · ".join(f"[{label}]({url})" for label, url in links) or "from the repo"
                f.write(f"| `{name}` | {title} | {cell} |\n")
            f.write("\n")
        f.write("## The exercise programs\n\n")
        f.write("The whole telemetry processor, one link per starter and solution, flattened to a single "
                "file with `data/sample.csv` on stdin: the report appears in the output pane. "
                "The Session 1 starter is compiled as C++11, everything else as C++23. The repo's "
                "test suites do not run here; use these to read and tweak the program, not to grade it.\n\n")
        f.write("| Session | Starter | Solution |\n|" + "-" * 40 + "|" + "-" * 30 + "|" + "-" * 30 + "|\n")
        by_ex: dict[str, dict[str, str]] = {}
        for name, variant, url in exercises:
            by_ex.setdefault(name, {})[variant] = url
        for name, links in by_ex.items():
            cells = " | ".join(f"[{v}]({links[v]})" if v in links else "" for v in ("starter", "solution"))
            f.write(f"| `{name}` | {cells} |\n")
        f.write("\n")
    print(f"wrote {INDEX.relative_to(ROOT)} ({sum(len(v) for v in rows.values())} demos, {len(exercises)} exercise programs)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
