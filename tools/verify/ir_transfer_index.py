#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Which CBMC-PROVED C files have a C++ twin that could inherit the proof.

This does not run the IR oracle (that needs FreeBSD for real TUs). It
answers the cheaper question the matrix cannot: of the functions CBMC
already proved on hbsd C, which files even have a port sitting in
pbsd/ or as a sibling .cpp. Those are the queue for
`tools/run_todo_passes.py --file …` on a FreeBSD host.

A twin is not a proof. ir.equal ∧ abi_equal is.
"""
from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HBSD = ROOT / "hbsd" / "src"
PBSD = ROOT / "pbsd"


def load_proved(path: Path) -> dict[str, list[str]]:
    by_file: dict[str, list[str]] = {}
    for line in path.open(encoding="utf-8"):
        line = line.strip()
        if not line:
            continue
        rec = json.loads(line)
        if rec.get("_meta") or rec.get("status") != "PROVED":
            continue
        by_file.setdefault(rec["file"], []).append(rec["function"])
    return by_file


def twins_of(rel: str) -> dict[str, str]:
    """rel is hbsd-relative, e.g. lib/libc/stdlib/abs.c"""
    found = {}
    c = HBSD / rel
    sibling = c.with_suffix(".cpp")
    if sibling.is_file():
        found["hbsd_cpp"] = str(sibling.relative_to(ROOT)).replace("\\", "/")
    pbsd = PBSD / rel
    for cand in (pbsd.with_suffix(".cpp"), pbsd.with_suffix(".cppm"),
                 Path(str(pbsd) + "pp"), pbsd):
        if cand.is_file():
            found["pbsd"] = str(cand.relative_to(ROOT)).replace("\\", "/")
            break
    # pbsd often drops src/ and uses the same path under pbsd/
    alt = PBSD / Path(rel)
    if "pbsd" not in found:
        for suf in (".cpp", ".cppm"):
            q = alt.with_suffix(suf)
            if q.is_file():
                found["pbsd"] = str(q.relative_to(ROOT)).replace("\\", "/")
                break
    return found


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--cbmc", default=str(Path.home() / "pbsd-sweep" / "cbmc.jsonl"))
    ap.add_argument("--out", help="jsonl of files that have a twin")
    args = ap.parse_args()
    proved = load_proved(Path(args.cbmc))
    n_fn = sum(len(v) for v in proved.values())
    counts = Counter()
    rows = []
    for rel, fns in sorted(proved.items()):
        tw = twins_of(rel)
        kind = "none"
        if "pbsd" in tw and "hbsd_cpp" in tw:
            kind = "both"
        elif "pbsd" in tw:
            kind = "pbsd"
        elif "hbsd_cpp" in tw:
            kind = "hbsd_cpp"
        counts[kind] += 1
        counts["fn_" + kind] += len(fns)
        rec = {"file": rel, "proved_functions": len(fns),
               "functions": fns[:40], "twin": tw, "kind": kind}
        if kind != "none":
            rows.append(rec)
    print(f"PROVED functions {n_fn} in {len(proved)} files")
    print(f"  no twin     files={counts['none']}  fn={counts['fn_none']}")
    print(f"  hbsd .cpp   files={counts['hbsd_cpp']}  fn={counts['fn_hbsd_cpp']}")
    print(f"  pbsd twin   files={counts['pbsd']}  fn={counts['fn_pbsd']}")
    print(f"  both        files={counts['both']}  fn={counts['fn_both']}")
    print("A twin is not ir.equal. Run the oracle on a FreeBSD host:")
    print("  python3 tools/run_todo_passes.py --file hbsd/src/<file> "
          "--all-passes --skip-corpus --ir-limit -1 --diff-limit 0")
    if args.out:
        out = Path(args.out)
        with out.open("w", encoding="utf-8") as fh:
            fh.write(json.dumps({"_meta": True, "proved_functions": n_fn,
                                 "proved_files": len(proved),
                                 "counts": dict(counts)}) + "\n")
            for rec in rows:
                fh.write(json.dumps(rec) + "\n")
        print(f"wrote {out} ({len(rows)} files with a twin)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
