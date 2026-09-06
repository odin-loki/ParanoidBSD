#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Bounded model checking over every function in hbsd/src, with CBMC.

WHAT THIS PROVES, AND WHAT IT DOES NOT

CBMC is a bounded model checker. Run with --unwind K it explores every path
whose loops iterate at most K times. That is not a theorem about the
function unless every loop actually terminates within K, and
--unwinding-assertions is what tells us which case we are in: if a loop
would need a K+1st iteration, CBMC emits an `unwinding assertion` failure.

So a run lands in exactly one of these, and they are NEVER merged:

  PROVED    every property SUCCESS, and no unwinding assertion failed. The
            loops closed inside the bound, so this IS a proof over all
            inputs for the checked properties.
  BOUNDED   every property SUCCESS, but an unwinding assertion failed. No
            counterexample exists within K iterations. Says nothing beyond.
  FAILED    a real property violation, with a counterexample trace.
  ERROR     did not compile as a standalone translation unit (usually a
            missing FreeBSD header on a Linux host - see PREPROCESSING).
  NOFUNC    the named function is not in this translation unit (it is a
            macro, an alias, ifdef'd out, or lives in an #included file).
  TIMEOUT   the solver did not finish.

THE CHECK SET, AND WHY IT IS TWO TIERS

The first real function this was ever run on, lib/libc/string/ffs.c, came
back VERIFICATION FAILED on

    arithmetic overflow on signed to unsigned type conversion in
    (unsigned int)mask

which is not a bug. Converting a negative int to unsigned int is DEFINED -
modular reduction, C17 6.3.1.3p2. --conversion-check flags value-changing
conversions, which is a lint, and systems code does that deliberately on
nearly every line. Left on, it would have reported a defect in a
three-line function that has none, 114,217 times over.

So: UB_CHECKS are the ones whose failure is undefined behaviour and
therefore a defect. ADVISORY_CHECKS are defined-but-suspicious and run
separately, reported separately, and gate nothing.

  UB          bounds, pointer, div-by-zero, signed overflow, undefined
              shift, pointer overflow, memory leak
  ADVISORY    conversion, unsigned overflow (defined: modular), NaN,
              float overflow

PREPROCESSING

hbsd/src is FreeBSD source. On a Linux host its headers collide with
glibc's - `typedef redefinition ... '__int32_t' (aka 'int') vs 'long'` -
so most files cannot be compiled standalone here at all. Those come back
ERROR, honestly, with the compiler's message recorded.

The fix is to preprocess on a FreeBSD host, where the real headers are, and
verify the resulting self-contained .i anywhere. --preprocessed DIR points
this driver at such a tree. Until that exists, the ERROR count is a
measurement of how much of the corpus is out of reach, not something to
hide.

Results are appended to JSONL as they are produced, so a run is resumable
and a kill loses at most the in-flight functions.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

UB_CHECKS = [
    "--bounds-check",
    "--pointer-check",
    "--div-by-zero-check",
    "--signed-overflow-check",
    "--undefined-shift-check",
    "--pointer-overflow-check",
    "--memory-leak-check",
]

ADVISORY_CHECKS = [
    "--conversion-check",
    "--unsigned-overflow-check",
    "--nan-check",
    "--float-overflow-check",
]

# CBMC names the unwinding assertion property class exactly this.
UNWIND_RE = re.compile(r"\bunwinding assertion\b", re.I)
RESULT_RE = re.compile(r"^\[([^\]]+)\]\s+(.*?):\s+(SUCCESS|FAILURE)\s*$", re.M)


def cbmc_bin() -> str:
    return shutil.which("cbmc") or "cbmc"


def verify_one(task: dict) -> dict:
    """Run CBMC on one (translation unit, function) pair."""
    src = task["src"]
    fn = task["function"]
    unwind = task["unwind"]
    timeout = task["timeout"]
    checks = ADVISORY_CHECKS if task["tier"] == "advisory" else UB_CHECKS

    cmd = [
        cbmc_bin(), src, "--function", fn,
        *checks,
        "--unwind", str(unwind),
        "--unwinding-assertions",
        *task.get("extra", []),
    ]
    t0 = time.time()
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        out = (p.stdout or "") + "\n" + (p.stderr or "")
        rc = p.returncode
    except subprocess.TimeoutExpired:
        return _rec(task, "TIMEOUT", elapsed=timeout)
    except OSError as e:
        return _rec(task, "ERROR", detail=str(e), elapsed=time.time() - t0)

    elapsed = time.time() - t0
    props = [
        {"name": m.group(1), "desc": m.group(2), "ok": m.group(3) == "SUCCESS"}
        for m in RESULT_RE.finditer(out)
    ]

    # No properties parsed at all means CBMC never got as far as checking.
    if not props:
        low = out.lower()
        if "not found" in low and fn.lower() in low:
            return _rec(task, "NOFUNC", detail=_tail(out), elapsed=elapsed)
        if "error:" in low or "conversion" in low or rc not in (0, 10):
            return _rec(task, "ERROR", detail=_tail(out), elapsed=elapsed)
        return _rec(task, "ERROR", detail=_tail(out), elapsed=elapsed)

    failed = [p for p in props if not p["ok"]]
    unwind_failed = [p for p in failed if UNWIND_RE.search(p["desc"])]
    real_failed = [p for p in failed if not UNWIND_RE.search(p["desc"])]

    if real_failed:
        status = "FAILED"
    elif unwind_failed:
        status = "BOUNDED"
    else:
        status = "PROVED"

    return _rec(
        task, status,
        properties=len(props),
        failures=[{"name": p["name"], "desc": p["desc"]} for p in real_failed[:12]],
        unwind_hit=bool(unwind_failed),
        elapsed=elapsed,
    )


def _tail(s: str, n: int = 600) -> str:
    s = s.strip()
    return s[-n:]


def _rec(task: dict, status: str, **kw) -> dict:
    return {
        "file": task["file"],
        "function": task["function"],
        "tier": task["tier"],
        "unwind": task["unwind"],
        "status": status,
        **kw,
    }


def load_tasks(plan: Path, scopes: list[str], unwind: int, timeout: int,
               tier: str, preprocessed: Path | None) -> list[dict]:
    d = json.loads(plan.read_text())
    tasks = []
    for rec in d["records"]:
        path = rec.get("path") or ""
        if scopes and not any(path.startswith(s) for s in scopes):
            continue
        if not rec.get("functions"):
            continue
        if preprocessed:
            cand = preprocessed / (path + ".i")
            if not cand.is_file():
                continue
            src = str(cand)
        else:
            cand = SRC / path
            if not cand.is_file():
                # a landed port: .c became .cpp
                alt = cand.with_suffix(".cpp")
                if not alt.is_file():
                    continue
                cand = alt
            src = str(cand)
        for fn in rec["functions"]:
            tasks.append({
                "file": path, "src": src, "function": fn,
                "unwind": unwind, "timeout": timeout, "tier": tier,
            })
    return tasks


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--plan", default=str(ROOT / "docs" / "port_plan.json"))
    ap.add_argument("--scope", action="append", default=[],
                    help="restrict to paths starting with this (repeatable)")
    ap.add_argument("--unwind", type=int, default=16)
    ap.add_argument("--timeout", type=int, default=60, help="seconds per function")
    ap.add_argument("--tier", choices=["ub", "advisory"], default="ub")
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    ap.add_argument("--preprocessed", help="tree of .i files from a FreeBSD host")
    ap.add_argument("--out", default="verify_results.jsonl")
    ap.add_argument("--limit", type=int)
    ap.add_argument("--resume", action="store_true",
                    help="skip (file, function) pairs already in --out")
    args = ap.parse_args()

    pre = Path(args.preprocessed) if args.preprocessed else None
    tasks = load_tasks(Path(args.plan), args.scope, args.unwind,
                       args.timeout, args.tier, pre)

    out = Path(args.out)
    done: set[tuple[str, str]] = set()
    if args.resume and out.is_file():
        for line in out.read_text().splitlines():
            try:
                r = json.loads(line)
                done.add((r["file"], r["function"]))
            except (ValueError, KeyError):
                pass
        tasks = [t for t in tasks if (t["file"], t["function"]) not in done]

    if args.limit:
        tasks = tasks[:args.limit]

    print(f"{len(tasks)} (file, function) pairs to check"
          + (f", {len(done)} already done" if done else ""), flush=True)
    if not tasks:
        return 0

    counts: dict[str, int] = {}
    t0 = time.time()
    with out.open("a") as fh, ProcessPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(verify_one, t): t for t in tasks}
        for i, fut in enumerate(as_completed(futs), 1):
            r = fut.result()
            counts[r["status"]] = counts.get(r["status"], 0) + 1
            fh.write(json.dumps(r) + "\n")
            fh.flush()
            if r["status"] == "FAILED":
                print(f"  FAILED {r['file']}:{r['function']}", flush=True)
                for f in r.get("failures", [])[:3]:
                    print(f"      {f['desc'][:150]}", flush=True)
            if i % 200 == 0 or i == len(tasks):
                rate = i / max(1e-9, time.time() - t0)
                print(f"  [{i}/{len(tasks)}] {rate:.1f}/s  "
                      + "  ".join(f"{k}={v}" for k, v in sorted(counts.items())),
                      flush=True)

    print("\n== totals")
    for k, v in sorted(counts.items()):
        print(f"  {k:9s} {v}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
