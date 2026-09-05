#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Compare hbsd/src against upstream HardenedBSD, file by file.

hbsd/src arrived as a single commit with no remote, no submodule and no
vendor branch - a flat copy with upstream history and file modes discarded.
That leaves a question nothing in the repo answers: which of these 107,357
files has PBSD actually changed?

Without that, an upstream sync either clobbers local work or silently keeps
stale files, and there is no way to tell which. This compares blob hashes, so
it is fast and exact.

    git clone --depth 1 -b hardened/15-stable/main \\
        https://github.com/HardenedBSD/hardenedBSD.git /tmp/hbsd-upstream
    python3 tools/upstream_drift.py /tmp/hbsd-upstream

Measured 2026-09-05 against hardened/15-stable/main: 87.0% byte-identical,
10.6% differing (6,846 of those in contrib/llvm-project alone), 2,664
PBSD-only and 4,423 present upstream but absent here - the snapshot dropped
sys/arm, sys/i386, sys/powerpc and sys/riscv wholesale. Only two files under
hbsd/src carry a PBSD marker, so the differences are overwhelmingly upstream
moving on rather than local work. PBSD's own code lives in pbsd/.

That last number is not academic: the missing sys/i386 broke buildkernel on
amd64, because sys/conf/kmod.mk links i386 into every module for 32-bit
compat regardless of target.
"""
from __future__ import annotations

import argparse
import collections
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PREFIX = "hbsd/src/"


def blobs(repo: Path, ref: str, prefix: str = "") -> dict[str, str]:
    out = subprocess.run(
        ["git", "-C", str(repo), "ls-tree", "-r", ref],
        capture_output=True, text=True, check=True,
    ).stdout
    found: dict[str, str] = {}
    for line in out.splitlines():
        meta, path = line.split("\t", 1)
        mode, kind, sha = meta.split()
        if kind != "blob":
            continue
        if prefix:
            if not path.startswith(prefix):
                continue
            path = path[len(prefix):]
        found[path] = sha
    return found


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("upstream", help="path to a HardenedBSD clone")
    ap.add_argument("--ref", default="HEAD")
    ap.add_argument("--show", type=int, default=12, help="dirs per section")
    args = ap.parse_args()

    up_repo = Path(args.upstream)
    if not (up_repo / ".git").exists():
        print(f"FAIL {up_repo} is not a git clone.")
        return 1

    ours = blobs(ROOT, "HEAD", PREFIX)
    theirs = blobs(up_repo, args.ref)
    if not ours:
        print(f"FAIL nothing under {PREFIX} in HEAD.")
        return 1

    same = {p for p in ours if theirs.get(p) == ours[p]}
    diff = {p for p in ours if p in theirs and theirs[p] != ours[p]}
    only = set(ours) - set(theirs)
    absent = set(theirs) - set(ours)

    n = len(ours)
    print(f"hbsd/src files : {n}")
    print(f"upstream files : {len(theirs)}\n")
    print(f"identical      : {len(same):>6}  ({100 * len(same) / n:.1f}%)")
    print(f"modified here  : {len(diff):>6}  ({100 * len(diff) / n:.1f}%)")
    print(f"PBSD-only      : {len(only):>6}")
    print(f"absent here    : {len(absent):>6}")

    for label, group in (("modified", diff), ("absent here", absent)):
        if not group:
            continue
        print(f"\ntop {label} directories:")
        counts = collections.Counter("/".join(p.split("/")[:2]) for p in group)
        for path, count in counts.most_common(args.show):
            print(f"  {count:>5}  {path}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:
        # Piping into head is the normal way to read this.
        sys.exit(0)
