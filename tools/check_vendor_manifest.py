#!/usr/bin/env python3
"""Check that vendor files matched by an ignore rule are still tracked.

hbsd/src/.gitignore belongs to upstream and matches files upstream itself
tracks: prebuilt driver objects, bmake and dtrace test fixtures, gettext
catalogues.  Git does not un-track a file when a rule starts matching it,
so these survive normal work -- but a re-import that runs 'git add' over a
fresh checkout drops all of them without a word.

That already happened once.  sys/dev/hpt27xx/amd64-elf.hpt27xx_lib.o went
missing and buildkernel stopped with "don't know how to make" it, two
minutes into a run, with nothing to say why the file was not there.

So the list is written down, and this asserts it.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys

DEFAULT_MANIFEST = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                "vendor_manifest.txt")


def read_manifest(path: str) -> list[str]:
    entries = []
    with open(path, "r", encoding="utf-8") as fh:
        for line in fh:
            line = line.strip()
            if line and not line.startswith("#"):
                entries.append(line)
    return entries


def tracked_paths(repo: str) -> set[str]:
    out = subprocess.run(["git", "-C", repo, "ls-files", "-z"],
                         capture_output=True, text=True, check=True).stdout
    return set(p for p in out.split("\0") if p)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", default=DEFAULT_MANIFEST)
    ap.add_argument("--repo", default=".")
    args = ap.parse_args()

    entries = read_manifest(args.manifest)
    tracked = tracked_paths(args.repo)

    untracked = [p for p in entries if p not in tracked]
    ondisk_missing = [p for p in entries
                      if not os.path.exists(os.path.join(args.repo, p))]

    print(f"manifest entries: {len(entries)}")
    print(f"not tracked:      {len(untracked)}")
    print(f"not on disk:      {len(ondisk_missing)}")

    for p in untracked:
        print(f"  UNTRACKED {p}")
    for p in ondisk_missing:
        print(f"  MISSING   {p}")

    return 1 if (untracked or ondisk_missing) else 0


if __name__ == "__main__":
    sys.exit(main())
