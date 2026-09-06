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
import resource
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


def _mem_capped(mb: int):
    """An RLIMIT_AS for the child, or None to leave it uncapped.

    A checker that can kill its host is worse than one that reports
    ERROR, and this one did - twice. The sys/dev shard of verify run 4:

        FAILED sys/dev/ata/chipsets/ata-cypress.c:ata_cypress_setmode
        ##[error]The runner has received a shutdown signal.
        ##[error]Process completed with exit code 143.
        Cleaning up orphan processes
        Terminate orphan process: pid (34642) (cbmc)

    SIGTERM thirteen minutes in, with one cbmc still running and the
    `if: always()` artifact skipped, so the partial .jsonl went with it
    and --resume had nothing to resume from. --jobs 2 and --timeout 60
    did not prevent it, because the bound that was missing is memory:
    subprocess.run's timeout does bound wall time, and a solver can
    exhaust a 16GB runner well inside sixty seconds.

    RLIMIT_AS makes that instance fail its allocation and exit, which
    the driver records as an ERROR for that one function. One function
    unproved beats a shard lost - and the "nothing could be checked"
    gate still fires if it is every function rather than a few.

    ProcessPoolExecutor means each task already runs in its own
    process, so preexec_fn here is safe.
    """
    if mb <= 0:
        return None

    def _limit() -> None:
        lim = mb * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (lim, lim))

    return _limit


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
    # A POINTER function is checked under an explicit, stated precondition:
    # its pointer arguments are valid, non-null objects. Without it the run
    # reports the ABSENCE of that precondition as a defect - strcat went
    # from 15 failures to 3 with --min-null-tree-depth 3, and the 15 were
    # every dereference of a nondeterministic char *.
    #
    # It does NOT encode "NUL-terminated within the object". The three that
    # survive on strcat are exactly that, and they are the caller's contract
    # rather than a bug. So a POINTER result is never PROVED outright: it is
    # recorded with the assumption that produced it.
    if task.get("null_depth"):
        cmd += ["--min-null-tree-depth", str(task["null_depth"])]
    t0 = time.time()
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout,
                           preexec_fn=_mem_capped(task.get("mem_mb", 0)))
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
    elif task.get("null_depth"):
        # Not PROVED. Proved GIVEN that the pointer arguments were valid
        # objects, which is an assumption this run made and did not check.
        status = "PROVED-ASSUMING"
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


# Stamped into every result, and checked by --resume.
#
# A resumed run reuses records verbatim, which is right while the meaning of
# a record has not changed and silently wrong the moment it has. The linkage
# fix is the case that made this necessary: `linkage` is copied out of
# classes.json, every record written before that fix carries the value the
# broken regex produced, and a --resume over them keeps 26,548 kernel
# functions labelled "exported" no matter how correct the code now is. The
# fix would have been real and the report would not have shown it.
#
# So: bump this whenever what a record MEANS changes - a new or removed
# check, a change to how status is decided, a change to a field's source.
# Not for a bug fix that leaves the fields meaning what they meant.
#
#   1  the sweep as first run
#   2  linkage is read from goto-instrument's Flags line (it was never read)
RESULT_VERSION = 2


def _rec(task: dict, status: str, **kw) -> dict:
    return {
        "v": RESULT_VERSION,
        "file": task["file"],
        "function": task["function"],
        "tier": task["tier"],
        "class": task.get("class"),
        "linkage": task.get("linkage"),
        "null_depth": task.get("null_depth", 0),
        "unwind": task["unwind"],
        "status": status,
        **kw,
    }


def load_tasks(plan: Path, scopes: list[str], unwind: int, timeout: int,
               tier: str, classes: Path, allow: set[str],
               null_depth: int = 0, mem_mb: int = 0) -> list[dict]:
    """One task per (translation unit, function the ledger says it defines).

    Two intersections, and both matter.

    classify.py reports every function in the goto model, which includes
    every `static inline` dragged in from a header - the same one in
    hundreds of translation units. The ledger says which functions a FILE
    defines, so intersecting with it checks each function once, in its own
    unit, instead of once per includer.

    And `allow` keeps only the classes where an unguarded modular check is
    sound. A POINTER function needs a precondition and is not run here.
    """
    d = json.loads(plan.read_text())
    cls = json.loads(classes.read_text())
    tasks, skipped = [], {"no-model": 0, "class": 0, "not-in-model": 0}
    for rec in d["records"]:
        path = rec.get("path") or ""
        if scopes and not any(path.startswith(s) for s in scopes):
            continue
        if not rec.get("functions"):
            continue
        c = cls.get(path)
        if not c or not c.get("ok"):
            skipped["no-model"] += len(rec["functions"])
            continue
        fns = c["functions"]
        for fn in rec["functions"]:
            if fn not in fns:
                skipped["not-in-model"] += 1
                continue
            if fns[fn] not in allow:
                skipped["class"] += 1
                continue
            tasks.append({
                "file": path, "src": c["gb"], "function": fn,
                "class": fns[fn],
                "linkage": c.get("linkage", {}).get(fn, "?"),
                "unwind": unwind, "timeout": timeout, "tier": tier,
                "null_depth": null_depth, "mem_mb": mem_mb,
            })
    print("  skipped: " + "  ".join(f"{k}={v}" for k, v in skipped.items()),
          flush=True)
    return tasks


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--plan", default=str(ROOT / "docs" / "port_plan.json"))
    ap.add_argument("--scope", action="append", default=[],
                    help="restrict to paths starting with this (repeatable)")
    ap.add_argument("--unwind", type=int, default=16)
    ap.add_argument("--timeout", type=int, default=60, help="seconds per function")
    ap.add_argument("--mem-mb", type=int, default=0,
                    help="RLIMIT_AS per CBMC instance, MB. 0 (default) is "
                         "uncapped, which is right on a workstation and "
                         "wrong on a 16GB runner - see _mem_capped().")
    ap.add_argument("--tier", choices=["ub", "advisory"], default="ub")
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    ap.add_argument("--classes", default="verify_classes.json",
                    help="output of classify.py")
    ap.add_argument("--null-depth", type=int, default=0,
                    help="CBMC --min-null-tree-depth. Non-zero states the "
                         "precondition that pointer arguments are valid, "
                         "non-null objects; a result under it is a proof "
                         "UNDER THAT ASSUMPTION and is recorded as such.")
    ap.add_argument("--allow", default="SCALAR,VOID",
                    help="classes to check (SCALAR,VOID are sound unguarded)")
    ap.add_argument("--out", default="verify_results.jsonl")
    ap.add_argument("--limit", type=int)
    ap.add_argument("--resume", action="store_true",
                    help="skip (file, function) pairs already in --out")
    args = ap.parse_args()

    tasks = load_tasks(Path(args.plan), args.scope, args.unwind,
                       args.timeout, args.tier, Path(args.classes),
                       set(args.allow.split(",")), args.null_depth,
                       args.mem_mb)

    out = Path(args.out)
    done: set[tuple[str, str]] = set()
    if args.resume and out.is_file():
        stale = 0
        keep = []
        for line in out.read_text().splitlines():
            if not line.strip():
                continue
            try:
                r = json.loads(line)
            except ValueError:
                continue
            if r.get("v") != RESULT_VERSION:
                # Written by an older driver, so its fields do not mean what
                # this one's mean. Drop it and check the function again
                # rather than resume onto an answer to a different question.
                stale += 1
                continue
            try:
                done.add((r["file"], r["function"]))
            except KeyError:
                continue
            keep.append(line)
        if stale:
            print(f"  {stale} result(s) were written by an older driver "
                  f"(result version != {RESULT_VERSION}); rechecking those",
                  flush=True)
            # Rewrite --out with only the still-valid records, so the file
            # never holds two generations of answers at once.
            out.write_text("".join(l + "\n" for l in keep))
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

    # A run in which NOTHING could be checked is not a success, and this
    # exited 0 for one. The first CI sweep reported
    #
    #     [1258/1258] 180.0/s  ERROR=1258
    #     == totals
    #       ERROR     1258
    #
    # in seven seconds, the report folded it in, and every job in the
    # workflow was green. The cause was that classify's real output is the
    # goto binaries in --outdir and the pipeline carried only the JSON
    # index, so cbmc had nothing to run on - but the cause is not the
    # point. A driver that cannot check anything has to say so in its exit
    # status, because that is the only thing the layer above reads.
    #
    # The floor is ALL of them, not a percentage: an ERROR is a normal
    # result for a function CBMC cannot model, and 1257 of 1258 is a bad
    # day rather than a broken pipeline. Nothing at all is broken.
    checked = sum(v for k, v in counts.items() if k != "ERROR")
    if tasks and checked == 0:
        print(f"\nFAIL  {len(tasks)} function(s) and not one could be")
        print("      checked. Every result is ERROR, so this run measured")
        print("      the instrument rather than the tree - most often the")
        print("      goto binaries classify.py wrote to its --outdir are")
        print("      not where this process can see them.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
