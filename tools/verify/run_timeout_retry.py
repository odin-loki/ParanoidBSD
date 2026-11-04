#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Retry CBMC TIMEOUT rows without rewriting the live sweep jsonl.

`--retry-status TIMEOUT` on the live cbmc.jsonl drops every TIMEOUT
record in that file, even when --limit is 40. This driver reads the
live file, writes a pair list, and runs cbmc_driver.py against a
*new* --out. The live jsonl is never opened for write.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]


def extract_pairs(jsonl: Path, scope: str, klass: str, status: str
                  ) -> tuple[list[tuple[str, str]], dict[str, int]]:
    st: Counter[str] = Counter()
    pairs: list[tuple[str, str]] = []
    seen: set[tuple[str, str]] = set()
    with jsonl.open(encoding="utf-8") as fh:
        for line in fh:
            if not line.strip():
                continue
            rec = json.loads(line)
            if rec.get("_meta"):
                continue
            st[rec.get("status", "?")] += 1
            fl = rec.get("file") or ""
            fn = rec.get("function")
            if rec.get("status") != status or not fn:
                continue
            if scope and not fl.startswith(scope):
                continue
            if klass and rec.get("class") != klass:
                continue
            key = (fl, fn)
            if key in seen:
                continue
            seen.add(key)
            pairs.append(key)
    return pairs, dict(st)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--in-jsonl", default="/home/odin/pbsd-sweep/cbmc.jsonl")
    ap.add_argument("--out-jsonl",
                    default="/home/odin/pbsd-sweep/cbmc-timeout-sys.jsonl")
    ap.add_argument("--pair-list",
                    default="/home/odin/pbsd-sweep/sys-scalar-timeout.pairs")
    ap.add_argument("--classes", default="/home/odin/pbsd-sweep/classes.json")
    ap.add_argument("--scope", default="sys/")
    ap.add_argument("--class-name", default="SCALAR",
                    dest="klass")
    ap.add_argument("--status", default="TIMEOUT")
    ap.add_argument("--limit", type=int, default=40)
    ap.add_argument("--unwind", type=int, default=32)
    ap.add_argument("--timeout", type=int, default=180)
    ap.add_argument("--jobs", type=int, default=2)
    ap.add_argument("--extract-only", action="store_true")
    ap.add_argument("--daemon", action="store_true",
                    help="double-fork; log next to --out-jsonl. Unix only.")
    args = ap.parse_args()

    if args.daemon:
        if not hasattr(os, "fork"):
            print("--daemon needs Unix", file=sys.stderr)
            return 2
        logp = Path(args.out_jsonl).with_suffix(".log")
        logp.parent.mkdir(parents=True, exist_ok=True)
        if os.fork():
            return 0
        os.setsid()
        if os.fork():
            return 0
        log = os.open(str(logp), os.O_WRONLY | os.O_CREAT | os.O_APPEND, 0o644)
        dn = os.open("/dev/null", os.O_RDWR)
        os.dup2(dn, 0)
        os.dup2(log, 1)
        os.dup2(log, 2)

    src = Path(args.in_jsonl)
    if not src.is_file():
        print(f"missing {src}", file=sys.stderr)
        return 1
    pairs, st = extract_pairs(src, args.scope, args.klass, args.status)
    print("cbmc status", st, flush=True)
    print(f"{args.scope} {args.klass} {args.status}: {len(pairs)}",
          flush=True)
    plist = Path(args.pair_list)
    plist.parent.mkdir(parents=True, exist_ok=True)
    take = pairs[: args.limit] if args.limit else pairs
    plist.write_text("".join(f"{a}\t{b}\n" for a, b in take), encoding="utf-8")
    print(f"wrote {plist} ({len(take)} pairs)", flush=True)
    if args.extract_only:
        return 0

    out = Path(args.out_jsonl)
    if out.resolve() == src.resolve():
        print("refusing to write retry results onto the live jsonl",
              file=sys.stderr)
        return 2

    cmd = [
        sys.executable, str(HERE / "cbmc_driver.py"),
        "--classes", args.classes,
        "--scope", args.scope.rstrip("/"),
        "--allow", args.klass,
        "--pair-list", str(plist),
        "--out", str(out),
        "--unwind", str(args.unwind),
        "--timeout", str(args.timeout),
        "--jobs", str(args.jobs),
        "--resume",
    ]
    print("+", " ".join(cmd), flush=True)
    return subprocess.call(cmd)


if __name__ == "__main__":
    # Never inherit a Windows cwd when this is the WSL entry point.
    if (ROOT / "hbsd" / "src").is_dir():
        os.chdir(ROOT)
    sys.exit(main())
