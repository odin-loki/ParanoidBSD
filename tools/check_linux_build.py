#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Gate the Linux build against a fixed set of known-unbuildable modules.

PBSD targets FreeBSD. A hosted Ubuntu runner cannot build the FreeBSD userland
ports - their sources include headers that live inside the FreeBSD tree or do
not exist on Linux - so `ninja` will always report failures there. That does
not make the build useless as a gate: everything else must still build, and a
module that stops building is a regression worth failing on.

This reads a ninja log, extracts the modules that failed, and compares them
with docs/migration/linux_build_exceptions.txt:

  a failure in the list      expected, ignored
  a failure not in the list  regression, exits 1
  an entry that now builds   reported, so the list can shrink

Usage:
    ninja -C build -k 0 2>&1 | tee build.log
    python3 tools/check_linux_build.py build.log [--write]
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
EXCEPTIONS = ROOT / "docs" / "migration" / "linux_build_exceptions.txt"
FAILED = re.compile(r"^FAILED:.*?CMakeFiles/pbsd\.dir/(\S+\.cppm)\.o", re.M)
PROGRESS = re.compile(r"^\[(\d+)/(\d+)\]", re.M)
MIN_STEPS = 100   # a real run scans well over a thousand modules


def load_exceptions() -> set[str]:
    if not EXCEPTIONS.exists():
        return set()
    return {
        line.strip()
        for line in EXCEPTIONS.read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.startswith("#")
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("log", help="ninja output captured with -k 0")
    ap.add_argument("--write", action="store_true",
                    help="rewrite the exception list from this log")
    args = ap.parse_args()

    text = pathlib.Path(args.log).read_text(encoding="utf-8", errors="replace")
    failed = set(FAILED.findall(text))
    known = load_exceptions()

    # `cmake --build ... || true` in CI means a build that dies immediately
    # leaves an empty log, and an empty log has no failures in it - which would
    # read as a clean pass. Require evidence that ninja actually did work.
    steps = [int(a) for a, _ in PROGRESS.findall(text)]
    if not args.write and (not steps or max(steps) < MIN_STEPS):
        print(f"FAIL  the build did not run: {len(steps)} ninja step(s) in {args.log}.")
        print("      An empty or truncated log has no failures in it, which is not "
              "the same as building cleanly.")
        return 1

    if args.write:
        header = [l for l in EXCEPTIONS.read_text(encoding="utf-8").splitlines()
                  if l.startswith("#")] if EXCEPTIONS.exists() else []
        EXCEPTIONS.write_text("\n".join(header + sorted(failed)) + "\n", encoding="utf-8")
        print(f"wrote {len(failed)} entries to {EXCEPTIONS.relative_to(ROOT)}")
        return 0

    new = sorted(failed - known)
    cleared = sorted(known - failed)

    for m in cleared:
        print(f"note  {m} now builds on Linux — drop it from the exception list")
    for m in new:
        print(f"FAIL  {m} no longer builds, and is not a known Linux exception")

    print(f"\n{len(failed)} module(s) failed, {len(known)} expected, "
          f"{len(new)} regression(s), {len(cleared)} cleared")
    if new:
        print("\nA module outside docs/migration/linux_build_exceptions.txt stopped "
              "building. Fix it, or add it with a reason if it is genuinely "
              "FreeBSD-only.")
        return 1
    print("No regressions: every failure is a known FreeBSD-only port.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
