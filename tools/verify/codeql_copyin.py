#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Run copyin_call.ql against an existing CodeQL database.

Does not extract. Default DB is the kern_ffclock smoke from
codeql_smoke.py. A hit means the extractor recorded a copyin() call in
that TU. It is not TRUST-UNVALIDATED-INPUT coverage of the tree.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
PACK = HERE / "codeql"
QUERY = PACK / "copyin_call.ql"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--db", default=str(Path.home() / "pbsd-sweep" / "codeql-ffclock-db"))
    ap.add_argument("--out", default=str(Path.home() / "pbsd-sweep" / "codeql-copyin.jsonl"))
    ap.add_argument("--query", default=str(QUERY),
                    help="ql file; default copyin_call.ql")
    ap.add_argument("--hit-text", default="copyin call",
                    help="substring in query stdout that means a hit")
    args = ap.parse_args()
    query = Path(args.query)
    codeql = shutil.which("codeql")
    rec = {"v": 1, "query": str(query), "db": args.db, "codeql": codeql}
    if not codeql:
        rec["status"] = "ERROR"
        rec["detail"] = "codeql not on PATH"
        Path(args.out).write_text(json.dumps(rec) + "\n", encoding="utf-8")
        return 1
    db = Path(args.db)
    if not db.is_dir():
        rec["status"] = "ERROR"
        rec["detail"] = f"missing db {db}"
        Path(args.out).write_text(json.dumps(rec) + "\n", encoding="utf-8")
        return 1
    subprocess.run([codeql, "pack", "install", str(PACK)],
                   capture_output=True, text=True)
    p = subprocess.run(
        [codeql, "query", "run", str(query), "--database", str(db)],
        capture_output=True, text=True)
    rec["query_rc"] = p.returncode
    rec["query_out"] = ((p.stdout or "") + (p.stderr or ""))[-1200:]
    hit = args.hit_text in (p.stdout or "")
    tag = "COPYIN"
    if "dest_used" in query.name:
        tag = "DEST-USED"
    if p.returncode == 0 and hit:
        rec["status"] = f"{tag}-OK"
    elif p.returncode == 0:
        rec["status"] = f"{tag}-EMPTY"
    else:
        rec["status"] = "ERROR"
        rec["detail"] = "query run failed"
    Path(args.out).write_text(json.dumps(rec) + "\n", encoding="utf-8")
    print("status", rec["status"], flush=True)
    print(rec.get("query_out", "")[-500:], flush=True)
    return 0 if rec["status"].endswith("OK") or rec["status"].endswith("EMPTY") else 1


if __name__ == "__main__":
    os.chdir(HERE.parents[1])
    raise SystemExit(main())
