#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""clang's static analyser over hbsd/src - a second instrument, not a second opinion.

CBMC is a bounded model checker run per function. It explores one
function's paths exhaustively and knows nothing about its callers, which
is why a static helper's findings need caller analysis and why an extern's
return is unconstrained.

clang --analyze is the opposite trade. It is path-sensitive and
INTERPROCEDURAL within a translation unit, unbounded in loop depth but
approximate - it will follow malloc through three functions and tell you
the result is leaked, which no modular check can see, and it will also
miss things CBMC proves.

They disagree by construction. That is the point: a bug found by one and
not the other is still a bug, and agreement on a finding is much stronger
evidence than either alone.

Checkers are chosen the same way CBMC's were - by what a failure MEANS.
`core.*` and `unix.Malloc` failures are defects. `deadcode.*` and
`security.insecureAPI.*` are style and policy for a 1990s codebase that
deliberately calls strcpy, and they are not run.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from includes import include_flags, lang_flags, SRC  # noqa: E402

# Failure of one of these is a defect, not a matter of taste.
CHECKERS = [
    "core.CallAndMessage", "core.DivideZero", "core.NonNullParamChecker",
    "core.NullDereference", "core.StackAddressEscape",
    "core.UndefinedBinaryOperatorResult", "core.VLASize",
    "core.uninitialized.ArraySubscript", "core.uninitialized.Assign",
    "core.uninitialized.Branch", "core.uninitialized.UndefReturn",
    "unix.Malloc", "unix.MallocSizeof", "unix.MismatchedDeallocator",
    "unix.cstring.BadSizeArg", "unix.cstring.NullArg",
]

# [^:]+ matches NEWLINES, so the file group swallowed every preceding line
# of -analyzer-output=text's source context until it found the next
# colon-digit-colon. 430 of 705 findings came back with a location like
#
#   '   60 |         *dstlenp = len;\n      |          ~~~~~~~ ^\n/home/.../sysctl.c:110'
#
# which is unusable for grouping and unreadable in a report. [^:\n] keeps
# the match on one line, which is what a diagnostic header is.
DIAG = re.compile(r"^(?P<file>[^:\n]+):(?P<line>\d+):(?P<col>\d+): "
                  r"warning: (?P<msg>.*?)\s*\[(?P<checker>[^\]]+)\]$", re.M)


def analyze(job: dict) -> dict:
    """Run the analyser's DEFAULT checkers and filter the findings.

    The first version passed -analyzer-disable-all-checks and then listed
    the wanted checkers with -analyzer-checker. That combination runs
    NOTHING: on a probe with a deliberate leak, a null dereference, a
    division by zero and an uninitialised read it reported zero, while
    plain `clang --analyze` on the same file reports five.

    So it would have printed "0 findings across 60 translation units" and
    that number would have meant nothing - the third time tonight a check
    agreed with itself because it could not see. Selecting by flag is
    replaced by selecting from the OUTPUT, which cannot be silently empty:
    the probe in tools/verify/test_analyze.py fails if it is.
    """
    src = Path(job["src"])
    cmd = ["clang", "--analyze", "-Xclang", "-analyzer-output=text",
           *lang_flags(src), *include_flags(src), str(src), "-o", "/dev/null"]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True,
                           timeout=job["timeout"], cwd="/tmp")
    except subprocess.TimeoutExpired:
        return {"file": job["rel"], "status": "TIMEOUT", "findings": []}
    except OSError as e:
        return {"file": job["rel"], "status": "ERROR", "detail": str(e),
                "findings": []}
    if p.returncode != 0 and "error:" in p.stderr:
        return {"file": job["rel"], "status": "ERROR",
                "detail": p.stderr.strip()[-300:], "findings": []}
    out = []
    wanted = set(CHECKERS)
    for m in DIAG.finditer(p.stderr):
        if m.group("checker") not in wanted:
            continue
        try:
            rel = Path(m.group("file")).resolve().relative_to(SRC).as_posix()
        except ValueError:
            rel = m.group("file")
        out.append({"where": f"{rel}:{m.group('line')}",
                    "checker": m.group("checker"), "msg": m.group("msg")})
    return {"file": job["rel"], "status": "OK", "findings": out}


DEFAULT_SCOPES = ["lib/libc", "lib/msun"]


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    # default=None, not default=[...]: argparse's "append" action APPENDS to
    # whatever default it is given, it does not replace it. With a non-empty
    # default, `--scope lib/libc/inet` produced
    #     ["lib/libc", "lib/msun", "lib/libc/inet"]
    # so a narrowed run silently did the whole of libc and msun - 1552
    # translation units where 14 were asked for. The narrowing never worked
    # and nothing said so, because a superset still contains the answer.
    ap.add_argument("--scope", action="append")
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--out", default="analyze_results.jsonl")
    ap.add_argument("--limit", type=int)
    args = ap.parse_args()

    jobs = []
    for s in (args.scope or DEFAULT_SCOPES):
        for pat in ("*.c", "*.cpp"):
            for f in sorted((SRC / s).rglob(pat)):
                jobs.append({"src": str(f), "rel": f.relative_to(SRC).as_posix(),
                             "timeout": args.timeout})
    if args.limit:
        jobs = jobs[:args.limit]
    print(f"{len(jobs)} translation units to analyse", flush=True)

    counts, nfind, t0 = {}, 0, time.time()
    with open(args.out, "w") as fh, ProcessPoolExecutor(args.jobs) as ex:
        futs = [ex.submit(analyze, j) for j in jobs]
        for i, fut in enumerate(as_completed(futs), 1):
            r = fut.result()
            counts[r["status"]] = counts.get(r["status"], 0) + 1
            nfind += len(r["findings"])
            fh.write(json.dumps(r) + "\n")
            for f in r["findings"]:
                print(f"  {f['where']}  [{f['checker']}]\n      {f['msg'][:110]}",
                      flush=True)
            if i % 200 == 0 or i == len(jobs):
                print(f"  [{i}/{len(jobs)}] {i/max(1e-9,time.time()-t0):.1f}/s  "
                      f"findings={nfind}  "
                      + "  ".join(f"{k}={v}" for k, v in sorted(counts.items())),
                      flush=True)
    print(f"\n{nfind} finding(s) across {len(jobs)} translation units")
    for k, v in sorted(counts.items()):
        print(f"  {k:8s} {v}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
