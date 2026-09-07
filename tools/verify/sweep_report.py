#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Read a sweep's shards, and say what changed since the last one.

A sweep is six analyze.py runs writing six .jsonl files, and the three
questions asked of every one of them are always the same:

  * what are the totals, and how do they compare with the sweep before;
  * which files changed status, in either direction - an ERROR that
    became OK is coverage, an OK that became ERROR is a regression and
    the only one of the two worth interrupting for;
  * of the ERRORs that --check-errors calls unlisted, which does the
    BUILD SYSTEM name and which does it not - because "not built" and
    "built and we cannot read it" are different problems with different
    answers, and sorting them by hand is how three of today's four
    reconciliations started.

Doing that by hand each time is how a sweep's numbers end up in a commit
message and nowhere else. This prints them the same way every time.
"""
from __future__ import annotations

import argparse
import collections
import functools
import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import includes  # noqa: E402
import userland_names  # noqa: E402
from expected_errors import EXPECTED, not_built  # noqa: E402

SYS = includes.SRC / "sys"


def load(p: Path) -> dict[str, dict]:
    out: dict[str, dict] = {}
    for line in p.read_text(errors="replace").splitlines():
        if not line.strip():
            continue
        r = json.loads(line)
        if r.get("_meta"):
            continue
        out[r["file"]] = r
    return out


def findings(recs: dict[str, dict]) -> set[tuple]:
    """(file, where, checker), with the shim's temp path normalised away.

    A finding inside a generated header carries the directory it was
    generated into, which is a fresh mkdtemp every run - so a diff of two
    sweeps reports every one of them as both new and gone. It is the same
    finding; the path is the run's, not the tree's.
    """
    out = set()
    for f_, r in recs.items():
        for x in (r.get("findings") or []):
            where = re.sub(r"^/tmp/pbsd_\w+_[A-Za-z0-9_]+/", "<generated>/",
                           x["where"])
            out.add((f_, where, x["checker"]))
    return out


@functools.lru_cache(maxsize=None)
def kernel_names() -> frozenset[str]:
    """Every kernel source the build names, as the build resolves it."""
    named = set()
    for p in sorted(SYS.glob("conf/files*")):
        t = p.read_text(errors="replace").replace("\\\n", " ")
        for line in t.splitlines():
            m = re.match(r"^(\S+\.[cS])\s", line)
            if m:
                named.add("sys/" + m.group(1))
    by_file, by_src, _ = includes.kernel_flag_index("amd64")
    return frozenset(named | set(by_file) | set(by_src))


def names_it(path: str) -> bool:
    """Does anything in the build name this source?

    The kernel's authority and userland's are different files asked
    different ways. Asking only the kernel's about lib/libc/gen/getcwd.c
    is how all 57 of one shard's unlisted ERRORs came back "nothing in
    the build names it" - a true statement about sys/conf/files and no
    statement at all about lib/libc.

    Both are asked, rather than one being picked by the path's scope,
    because the trees are not disjoint: lib/libc's own build names
    sys/kern/subr_capability.c, sys/kern/subr_acl_nfs4.c and
    sys/libkern/explicit_bzero.c through .PATH. Routing sys/ to the
    kernel would have answered for those with the wrong authority.
    """
    return path in kernel_names() or path in userland_names.names()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("shards", nargs="+", help="this sweep's .jsonl files")
    ap.add_argument("--against", action="append", default=[],
                    help="the previous sweep's .jsonl files")
    ap.add_argument("--unlisted", action="store_true",
                    help="also sort the unlisted ERRORs by whether the "
                         "build system names them")
    args = ap.parse_args()

    new: dict[str, dict] = {}
    per = {}
    for s in args.shards:
        p = Path(s)
        recs = load(p)
        new.update(recs)
        c = collections.Counter(r["status"] for r in recs.values())
        per[p.stem] = (c, sum(len(r.get("findings") or [])
                              for r in recs.values()))
    print(f"{'shard':14s} {'OK':>6s} {'ERROR':>6s} {'findings':>9s}")
    for k, (c, f) in sorted(per.items()):
        print(f"{k:14s} {c['OK']:6d} {c['ERROR']:6d} {f:9d}")
    tot = collections.Counter(r["status"] for r in new.values())
    print(f"{'TOTAL':14s} {tot['OK']:6d} {tot['ERROR']:6d} "
          f"{len(findings(new)):9d}   ({sum(tot.values())} TUs)")

    if args.against:
        old: dict[str, dict] = {}
        for s in args.against:
            old.update(load(Path(s)))
        gained = [k for k in new if new[k]["status"] == "OK"
                  and old.get(k, {}).get("status") == "ERROR"]
        lost = [k for k in new if new[k]["status"] == "ERROR"
                and old.get(k, {}).get("status") == "OK"]
        a, b = findings(old), findings(new)
        print(f"\nagainst the previous sweep: {len(old)} TUs")
        print(f"  ERROR -> OK   {len(gained)}")
        print(f"  OK -> ERROR   {len(lost)}"
              f"{'   <- REGRESSIONS' if lost else ''}")
        for k in sorted(lost):
            print(f"      {k}")
        print(f"  findings {len(a)} -> {len(b)}: "
              f"{len(b - a)} new, {len(a - b)} gone")
        for x in sorted(b - a)[:20]:
            print(f"      + {x[1]}  {x[2]}")
        if len(b - a) > 20:
            print(f"      ... and {len(b - a) - 20} more")

    if args.unlisted:
        unl = sorted(k for k, r in new.items() if r["status"] == "ERROR"
                     and k not in EXPECTED and not not_built(k))
        print(f"\nERRORs the inventory does not cover: {len(unl)}")
        by = collections.defaultdict(list)
        for k in unl:
            by["the build names it" if names_it(k)
               else "nothing in the build names it"].append(k)
        for kind, files in sorted(by.items()):
            print(f"  {len(files):4d}  {kind}")
            d = collections.Counter(str(Path(f).parent) for f in files)
            for k, v in sorted(d.items(), key=lambda kv: -kv[1])[:12]:
                print(f"          {v:3d}  {k}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
