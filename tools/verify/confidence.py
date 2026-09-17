#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""How much of a scope has been checked, and how much of that is settled.

Two questions get confused constantly and this separates them:

    "how many findings are left?"        -- the wrong question
    "how much of this could the instruments SEE, and of what they
     saw, how much has somebody resolved?"

The wrong question is wrong because a scope nobody checked reports zero
findings and looks exactly like one that is clean. Every large win in
docs/security/UB_FINDINGS.md has been an instance of that: the port
ledger's SKIP tags hid 2,465 translation units from the model checker;
`-D__seg_gs=' was ONE LINE and took sys/dev from a fraction to 2,419 of
2,612; before the interface-header fix sys/ was a few hundred usable
translation units out of thousands. In each case the number of findings
went UP when the instrument got better, and a metric that rewards a low
finding count would have called every one of those a regression.

So the score here is a PRODUCT of three fractions, and the components
are always printed beside it. A product, because these compose the way
the pipeline does -- a translation unit that will not compile is never
attempted, and a function with no verdict is never read -- and because
a product cannot be inflated by one strong component. 90% visible, 90%
answered and 90% read is 73%, and that is the honest number.

    VISIBILITY   of the translation units in scope, how many the
                 instrument could build. clang ERROR and goto-cc
                 TU-ERROR both count against it.
    ANSWER       of the functions attempted, how many returned a
                 verdict. TIMEOUT and CBMC's own ERROR count against
                 it -- those functions are UNCHECKED, not clean.
    RESOLUTION   of the verdicts returned, how many are settled: a
                 proof, or a failure in a bucket whose rule explains
                 it, or a failure in a READ-THESE bucket that somebody
                 read and put in the not-a-defect table.

A scope with NO DATA scores zero and is listed. It is not omitted and
it is not scored n/a, because "we have no data" and "it is clean" are
the two things this whole document set exists to keep apart.

What the score is NOT
---------------------
It is not a proof obligation discharged, and CONFIDENCE 1.0 does not
mean correct. What CBMC proves is that the CHECKED PROPERTIES -- signed
overflow, shifts, division, array bounds, pointer validity -- hold for
all inputs WITHIN THE LOOP BOUND. It says nothing about whether the
function computes the right answer. A driver that returns the wrong
value for every input can score 1.0.

It is also weighted by TRANSLATION UNIT and FUNCTION, not by risk. A
scope of ten leaf helpers and a scope of ten on-disk parsers score the
same way, and they are not the same exposure.
"""
from __future__ import annotations

import argparse
import collections
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))

import report  # noqa: E402  (bucket(), is_triaged(), desc_line())

# A verdict that says something about the code.
ANSWERED = {"PROVED", "PROVED-ASSUMING", "BOUNDED", "FAILED"}
# A verdict that says something about the INSTRUMENT.
NO_ANSWER = {"TIMEOUT", "ERROR", "NOFUNC"}

# The buckets whose rule explains the failure. A record here is settled
# by the rule itself -- that is what the rule is for, and each one cost
# a night to find. The three READ-THESE buckets are deliberately absent:
# those are settled only by a person, via the not-a-defect table.
EXPLAINED = (
    "pointer/memory",
    "static (callers constrain",
    "extern-driven",
    "CBMC could not model it",
    "a per-CPU read CBMC cannot model",
    "float div-by-zero",
    "no detail",
)


def settled(rec: dict) -> bool:
    """Is this FAILED record resolved, or is it still somebody's evening?"""
    b = report.bucket(rec)
    if any(b.startswith(x) for x in EXPLAINED):
        return True
    # A READ-THESE record is settled only if the document says a person
    # read it. Matching on the deciding failures rather than the first
    # two, because the first two can be pointer failures the bucket
    # already stripped.
    lines = [report.desc_line(d) for d in report.deciding_failures(rec)]
    lines = [x for x in lines if x is not None]
    return bool(lines) and all(report.is_triaged(rec["file"], x)
                               for x in lines)


def ledger_scope(prefix: str) -> tuple[int, int]:
    """(translation units, functions) the ledger names under a prefix.

    The ledger is the authority on what is IN SCOPE, which is the
    denominator nothing else can supply: the instruments only know
    about files they were handed.
    """
    plan = ROOT / "docs" / "port_plan.json"
    if not plan.is_file():
        return (0, 0)
    recs = json.loads(plan.read_text())["records"]
    tus = fns = 0
    for r in recs:
        p = r.get("path", "")
        if not p.startswith(prefix):
            continue
        if not p.endswith((".c", ".cc", ".cpp", ".cxx")):
            continue
        f = r.get("functions") or []
        if not f:
            continue            # nothing for a checker to check
        tus += 1
        fns += len(f)
    return (tus, fns)


def load_classes(paths: list[str]) -> dict:
    """classify.py's index: the only artefact that knows which
    translation units goto-cc actually BUILT.

    The first version of this tool counted distinct files appearing in
    the model-check jsonl, which was wrong in the direction that
    matters: that file holds one record per (file, function) CHECKED,
    and most translation units contribute none -- 1,419 pairs came from
    570 of sys/dev's 2,419 built units, because the rest had no SCALAR
    or VOID function to attempt. So it reported 570/2612 = 0.03 for a
    scope that is really 2419/2612 = 0.93, and understated the one
    number this project has spent the most effort raising.
    """
    out = {}
    for p in paths:
        fp = Path(p)
        if not fp.is_file():
            continue
        try:
            d = json.loads(fp.read_text(errors="replace"))
        except ValueError:
            continue
        if isinstance(d, dict):
            out.update(d)
    return out


def load(paths: list[str]) -> list[dict]:
    out = []
    for p in paths:
        fp = Path(p)
        if not fp.is_file():
            continue
        for ln in fp.read_text(errors="replace").splitlines():
            ln = ln.strip()
            if not ln or ln.startswith("//"):
                continue
            try:
                r = json.loads(ln)
            except ValueError:
                continue
            if isinstance(r, dict) and not r.get("_meta"):
                out.append(r)
    return out


def score(scope: str, analyse: list[dict], model: list[dict],
          classes: dict) -> dict:
    a = [r for r in analyse if r.get("file", "").startswith(scope)]
    m = [r for r in model if r.get("file", "").startswith(scope)]
    cls = {k: v for k, v in classes.items() if k.startswith(scope)}
    tus, fns = ledger_scope(scope)

    a_ok = sum(1 for r in a if r.get("status") == "OK")
    a_err = sum(1 for r in a if r.get("status") not in ("OK", None))
    findings = sum(len(r.get("findings") or []) for r in a)

    # VISIBILITY. Two instruments, and the weaker one decides -- a file
    # clang reads and goto-cc cannot is still half unseen.
    #
    # "NO DATA" and "FAILED TO BUILD" are kept apart, because folding
    # them together is the same error as counting a scope nobody ran as
    # clean. An instrument that produced no records for this scope is
    # reported as not-run and takes no part in the fraction; only an
    # instrument that ran and could not build a file counts against it.
    # If NEITHER ran, visibility is 0 and the verdict says UNSEEN.
    m_ran = bool(cls)
    a_ran = bool(a)
    m_ok = sum(1 for v in cls.values() if v.get("ok"))
    parts = []
    if m_ran:
        parts.append(m_ok / tus if tus else 0.0)
    if a_ran:
        parts.append(a_ok / tus if tus else 0.0)
    vis = min(min(parts), 1.0) if parts else 0.0

    st = collections.Counter(r.get("status") for r in m)
    answered = sum(st[k] for k in ANSWERED)
    noans = sum(st[k] for k in NO_ANSWER)
    ans = (answered / (answered + noans)) if (answered + noans) else 0.0

    failed = [r for r in m if r.get("status") == "FAILED"]
    unread = [r for r in failed if not settled(r)]
    res = ((answered - len(unread)) / answered) if answered else 0.0

    return {
        "scope": scope, "tus": tus, "functions": fns,
        "analysed_ok": a_ok, "analysed_err": a_err, "findings": findings,
        "modelled": m_ok, "model_tus": len(cls),
        "model_ran": m_ran, "analyse_ran": a_ran,
        "checked": answered, "no_answer": noans,
        "proved": st["PROVED"], "failed": len(failed),
        "unread": len(unread),
        "visibility": vis, "answer": ans, "resolution": res,
        "confidence": vis * ans * res,
    }


# The port gate. Porting a file means rewriting it, so an unread
# finding carried across is a defect that survives WITHOUT its
# provenance: the new code has the bug and no record saying where it
# came from. That is why `unread' is an absolute and not a threshold.
#
# The other two are thresholds because perfect is not available: 0.95
# visibility is what sys/fs and sys/geom already reach (119/119 and
# 78/78), so it is a demonstrated bar rather than an aspiration, and
# 0.70 answer is roughly what sys/dev returned (1001 of 1419) without
# any tuning. A scope that cannot meet them is not blocked from being
# ported -- it is blocked from being ported QUIETLY.
GATE = {"visibility": 0.95, "answer": 0.70}


def verdict(s: dict) -> tuple[str, list[str]]:
    why = []
    if s["tus"] == 0:
        return ("NO LEDGER ENTRIES", ["nothing in scope names a function"])
    if not s["model_ran"] and not s["analyse_ran"]:
        return ("UNSEEN", ["no instrument has returned data for this scope;"
                           " that is not the same as clean"])
    if not s["model_ran"]:
        why.append("the model checker has no data here (not run)")
    if not s["analyse_ran"]:
        why.append("the analyser has no data here (not run)")
    if s["visibility"] < GATE["visibility"]:
        bits = []
        if s["model_ran"]:
            bits.append(f"{s['modelled']}/{s['tus']} built by goto-cc")
        if s["analyse_ran"]:
            bits.append(f"{s['analysed_ok']}/{s['tus']} built by clang")
        why.append(f"visibility {s['visibility']:.2f} < "
                   f"{GATE['visibility']} ({', '.join(bits)})")
    if s["answer"] < GATE["answer"]:
        why.append(f"answer {s['answer']:.2f} < {GATE['answer']}"
                   f" ({s['no_answer']} of"
                   f" {s['checked'] + s['no_answer']} gave no verdict)")
    if s["unread"]:
        why.append(f"{s['unread']} finding(s) nobody has read")
    return ("PORT-READY" if not why else "HOLD", why)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--scope", action="append", default=[],
                    help="path prefix to score; repeatable")
    ap.add_argument("--analyze", action="append", default=[],
                    help="analyse jsonl; repeatable")
    ap.add_argument("--model", action="append", default=[],
                    help="model-check jsonl; repeatable")
    ap.add_argument("--classes", action="append", default=[],
                    help="classify.py index json -- the only artefact "
                         "that knows which units goto-cc BUILT; "
                         "repeatable")
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 unless every scope is PORT-READY")
    args = ap.parse_args()

    if not args.scope:
        ap.error("--scope is required: there is no meaningful whole-tree "
                 "number, only per-area ones")

    analyse = load(args.analyze)
    model = load(args.model)
    classes = load_classes(args.classes)

    print("== how much of each scope is checked, and how much is settled")
    print("   CONFIDENCE = VISIBILITY x ANSWER x RESOLUTION.")
    print("   It is not a proof of correctness; see the module docstring.")
    print()
    hdr = (f"{'scope':<34} {'TUs':>6} {'vis':>5} {'ans':>5} {'res':>5} "
           f"{'CONF':>6}  {'proved':>6} {'unread':>6}")
    print(hdr)
    print("-" * len(hdr))

    rows = [score(s, analyse, model, classes)
            for s in args.scope]
    for s in rows:
        print(f"{s['scope']:<34} {s['tus']:>6} "
              f"{s['visibility']:>5.2f} {s['answer']:>5.2f} "
              f"{s['resolution']:>5.2f} {s['confidence']:>6.2f}  "
              f"{s['proved']:>6} {s['unread']:>6}")

    print("\n== the port gate")
    bad = 0
    for s in rows:
        v, why = verdict(s)
        if v != "PORT-READY":
            bad += 1
        print(f"  {v:<18} {s['scope']}")
        for w in why:
            print(f"       - {w}")

    print("\n  PORT-READY means the instruments could see nearly all of it,")
    print("  they answered for most of what they saw, and nothing they")
    print("  found is still unread. It does NOT mean the code is correct.")
    print("  Porting a file rewrites it, so an unread finding carried")
    print("  across becomes a defect WITHOUT its provenance -- which is")
    print("  why that one is an absolute and not a threshold.")

    if args.gate and bad:
        print(f"\nFAIL  {bad} scope(s) are not PORT-READY")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
