#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Turn raw model-checker output into the short list worth reading.

A run over lib/libc and lib/msun reports roughly 165 failures. Around 128
of them are not defects, and which 128 is decided by five rules that cost
a night to find. This applies them, so the output is the ~37 that need a
person rather than the 165 that do not.

  pointer/memory        a nondeterministic pointer includes NULL and
                        includes a dangling one, so the report is the
                        function's MISSING PRECONDITION, not a bug.
  float div-by-zero     IEEE-754 defines x/0.0, and lib/msun depends on it
                        for log(0), logb, rsqrt and the catrig family.
                        CBMC's --div-by-zero-check does not distinguish it
                        from the integer case, which IS undefined.
  static                the signature domain is the wrong domain: the only
                        callers are in the same file and they constrain it.
                        Deferred, not dismissed.
  extern-driven         an unmodelled extern's return is as unconstrained
                        as a pointer parameter - clock() via getrusage,
                        s_significand via ilogb.
  unwinding             a loop wanted more iterations than --unwind gave.
                        BOUNDED, which is a weaker claim, not a failure.

What survives is: EXPORTED, arithmetic, parameter-driven. Both confirmed
findings in docs/security/UB_FINDINGS.md came out of that bucket.
"""

from __future__ import annotations

import argparse
import collections
import json
import re
import sys
from pathlib import Path

PTR_WORDS = ("dereference", "pointer", "object", "memory never freed",
             "array ")
# Functions whose inputs come from an extern this run did not model.
EXTERN_DRIVEN = {
    "clock", "alarm", "svc_run", "cap_sandboxed", "significand",
    "significandf", "cosl", "sinl", "tanl", "__rebuild_environ",
    "__enlarge_env", "__remove_putenv",
}


def kinds(rec: dict) -> set[str]:
    return {re.sub(r"^line \d+ ", "", d["desc"]).split(" in ")[0]
            for d in rec.get("failures", [])}


def bucket(rec: dict) -> str:
    k = kinds(rec)
    if not k:
        return "no detail"
    if any(w in x for x in k for w in PTR_WORDS):
        return "pointer/memory (a missing precondition, not a bug)"
    if k == {"division by zero"} and rec["file"].startswith("lib/msun"):
        return "float div-by-zero (IEEE-754 defines it; msun depends on it)"
    if rec.get("linkage") == "static":
        return "static (callers constrain the domain - deferred)"
    if rec["function"] in EXTERN_DRIVEN:
        return "extern-driven (an unmodelled return, unconstrained)"
    return "EXPORTED, arithmetic - READ THESE"


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("results", nargs="+", help="cbmc_driver.py .jsonl output")
    ap.add_argument("--analyze", help="analyze.py .jsonl output, folded in")
    args = ap.parse_args()

    recs = []
    for p in args.results:
        for line in Path(p).read_text().splitlines():
            if line.strip():
                recs.append(json.loads(line))

    status = collections.Counter(r["status"] for r in recs)
    print("== what the model checker concluded")
    order = ["PROVED", "PROVED-ASSUMING", "BOUNDED", "FAILED",
             "TIMEOUT", "ERROR", "NOFUNC"]
    for k in order + [k for k in status if k not in order]:
        if status.get(k):
            print(f"  {k:16s} {status[k]}")
    print(f"  {'':16s} {sum(status.values())} checked")
    if status.get("PROVED"):
        print("\n  PROVED means every checked property holds for ALL inputs -")
        print("  the loops closed inside the bound. BOUNDED and")
        print("  PROVED-ASSUMING are weaker and are never folded into it.")

    failed = [r for r in recs if r["status"] == "FAILED"]
    buckets = collections.defaultdict(list)
    for r in failed:
        buckets[bucket(r)].append(r)

    print(f"\n== {len(failed)} failures, bucketed by the five rules")
    for b in sorted(buckets, key=lambda x: -len(buckets[x])):
        print(f"  {len(buckets[b]):4d}  {b}")

    real = buckets.get("EXPORTED, arithmetic - READ THESE", [])
    print(f"\n== the {len(real)} worth a person's time")
    for r in sorted(real, key=lambda x: (x["file"], x["function"])):
        print(f"  {r['file']}:{r['function']}")
        for d in r.get("failures", [])[:2]:
            print(f"      {d['desc'][:100]}")

    if args.analyze:
        an = [json.loads(l) for l in Path(args.analyze).read_text().splitlines()
              if l.strip()]
        finds = [(f, r) for r in an for f in r.get("findings", [])]
        by = collections.Counter(f["checker"] for f, _ in finds)
        print(f"\n== clang --analyze: {len(finds)} finding(s), a DIFFERENT "
              f"instrument")
        for c, n in by.most_common():
            print(f"  {n:4d}  {c}")
        print("\n  Path-sensitive and interprocedural where CBMC is")
        print("  exhaustive and modular, and approximate where CBMC is")
        print("  exact. A finding in BOTH is much stronger than either.")

    print("\n== before you fix anything")
    print("  Confirm it with UBSan first. Every entry in")
    print("  docs/security/UB_FINDINGS.md was, and three plausible")
    print("  findings died that way - FP_ILOGB0 is -INT_MAX and not")
    print("  INT_MIN, so -ilogb(x) cannot overflow.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
