#!/usr/bin/env python3
"""Fail if a feature slide has no standard badge.

A slide is a feature slide when it carries `<!-- _class: feature -->` or
`<!-- _class: twocol -->`. It must contain a `<span class="badge cppNN">`.
Usage: check-standard-tags.py slides/*.md
"""
import pathlib
import re
import sys

BADGE = re.compile(r'class="badge cpp(11|14|17|20|23)"')
FEATURE = re.compile(r"<!--\s*_class:\s*(feature|twocol)")

errors = 0
for p in map(pathlib.Path, sys.argv[1:]):
    slides = p.read_text().split("\n---\n")
    for n, s in enumerate(slides, 1):
        if FEATURE.search(s) and not BADGE.search(s):
            print(f"{p}: slide {n} is a feature slide with no standard badge", file=sys.stderr)
            errors += 1
sys.exit(1 if errors else 0)
