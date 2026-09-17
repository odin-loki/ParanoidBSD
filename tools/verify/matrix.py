#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""One row per function, one column per instrument. What is still untouched.

inventory.py says which functions exist - 329,629 of them across hbsd/src,
kde/ and pbsd/. This says, for each of them, what each instrument returned.
The cell that matters most is the empty one.

The point of the shape
----------------------
A summary that counts FINDINGS answers "what did we see". A matrix that
counts FUNCTIONS answers "what did we LOOK AT", and those diverge in the
one direction that is dangerous: a scope with no findings and a scope
nobody ran are the same row in a findings report and different rows here.

So there is no cell value that means "fine". There are values that mean
an instrument returned a verdict, and values that mean it did not, and
the report separates a function that nobody attempted from a function
that somebody attempted and failed on. Those two are both UNTOUCHED and
they need completely different work: one needs a run, the other needs a
build fixed or a bound raised.

Cell values
-----------
From a per-function engine (cbmc, esbmc, fusebmc):

    PROVED           the checked properties hold for every input within
                     the unwind bound. The strongest cell in the table,
                     and still not "correct" - see confidence.py.
    PROVED-ASSUMING  as above, under a precondition somebody stated.
    BOUNDED          loops were not closed; it holds up to the bound.
    FAILED           a counterexample exists. It may be a defect or a
                     missing precondition; report.py's buckets decide.
    TIMEOUT / ERROR  no answer. The function is UNCHECKED, not clean.
    NOFUNC           the engine's model did not contain it at all.

From a per-translation-unit instrument (clang-analyze, clang-tidy,
cppcheck, coccinelle, compiler-warnings, and the C++ tier):

    REPORTED   the tool named THIS function in a finding.
    TU-CLEAN   the unit built, the tool ran, and it said nothing about
               this function. Weaker than PROVED by a long way: it means
               one approximate instrument had no complaint.
    TU-ERROR   the unit did not build. THIS IS THE IMPORTANT ONE. A file
               that does not compile produces zero findings and is
               indistinguishable from a clean one in every findings
               report ever written. Here it is a distinct cell and it
               counts against coverage.

And the absence of a cell is NOTRUN, which is not a verdict about the
code at all.

Row strength is the best cell: PROVED > CHECKED (BOUNDED or FAILED -
somebody's engine actually decided something about this function) >
SCANNED (only TU-level instruments looked) > UNTOUCHED.

Merging
-------
Ingest is idempotent and last-write-wins PER ENGINE, with one exception:
a non-verdict never overwrites a verdict from the same engine, because
the absence of data is not data. Re-ingesting the same file changes
nothing; ingesting a newer run of the same engine replaces its column.
"""
from __future__ import annotations

import argparse
import collections
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# Engines that speak per function.
FUNC_ENGINES = {"cbmc", "esbmc", "fusebmc"}
# Engines that speak per translation unit, with findings that name a function.
TU_ENGINES = {"clang-analyze", "clang-tidy", "cppcheck", "coccinelle",
              "compiler-warnings", "cxx-analyze", "cxx-tidy", "pbsd-lints"}
ENGINES = sorted(FUNC_ENGINES | TU_ENGINES)

PROVEN = {"PROVED", "PROVED-ASSUMING"}
DECIDED = {"BOUNDED", "FAILED"}
SCANNED_V = {"REPORTED", "TU-CLEAN"}
# A cell that says something about the INSTRUMENT, not about the code.
NO_VERDICT = {"TIMEOUT", "ERROR", "NOFUNC", "TU-ERROR", "NOTRUN"}

VERDICTS = PROVEN | DECIDED | SCANNED_V | NO_VERDICT

# Row strength, strongest first.
S_PROVED, S_CHECKED, S_SCANNED, S_UNTOUCHED = \
    "PROVED", "CHECKED", "SCANNED", "UNTOUCHED"


def strength(cells: dict) -> str:
    vals = set(cells.values())
    if vals & PROVEN:
        return S_PROVED
    if vals & DECIDED:
        return S_CHECKED
    if vals & SCANNED_V:
        return S_SCANNED
    return S_UNTOUCHED


def why_untouched(cells: dict) -> str:
    """Never attempted, or attempted and no answer. Different work each."""
    if not cells:
        return "never-attempted"
    if set(cells.values()) <= {"NOTRUN"}:
        return "never-attempted"
    return "attempted-no-answer"


# ------------------------------------------------------------------- storage

class Matrix:
    def __init__(self):
        self.rows: dict[tuple, dict] = {}     # (tree, file, fn) -> cells
        self.src: dict[tuple, str] = {}       # (tree, file, fn) -> ledger|textual
        self.provenance: dict[str, str] = {}  # engine -> where its column came from
        self.unknown: collections.Counter = collections.Counter()

    # -- load ------------------------------------------------------------
    def load_universe(self, path: Path):
        n = 0
        with path.open() as fh:
            for line in fh:
                d = json.loads(line)
                if d.get("_meta"):
                    continue
                key = (d["tree"], d["file"], d["function"])
                self.rows.setdefault(key, {})
                self.src[key] = d.get("src", "?")
                n += 1
        return n

    def load(self, path: Path):
        """Re-open a matrix written by save()."""
        n = 0
        with path.open() as fh:
            for line in fh:
                d = json.loads(line)
                if d.get("_meta"):
                    self.provenance.update(d.get("provenance", {}))
                    continue
                key = (d["tree"], d["file"], d["function"])
                self.rows[key] = d.get("cells", {})
                self.src[key] = d.get("src", "?")
                n += 1
        return n

    def save(self, path: Path):
        with path.open("w") as fh:
            fh.write(json.dumps({"_meta": True, "v": 1,
                                 "provenance": self.provenance}) + "\n")
            for key in sorted(self.rows):
                tree, f, fn = key
                fh.write(json.dumps({
                    "tree": tree, "file": f, "function": fn,
                    "src": self.src.get(key, "?"),
                    "cells": self.rows[key],
                }) + "\n")

    # -- ingest ----------------------------------------------------------
    def _set(self, key, engine, verdict):
        cells = self.rows.get(key)
        if cells is None:
            # A verdict about a function the universe does not know. That is
            # a real signal - the universe is incomplete - so it is counted
            # rather than dropped, but it is not invented as a row.
            self.unknown[engine] += 1
            return
        old = cells.get(engine)
        if old is not None and old not in NO_VERDICT and verdict in NO_VERDICT:
            return                       # absence never overwrites presence
        cells[engine] = verdict

    def ingest(self, engine: str, tree: str, path: Path) -> dict:
        if engine not in ENGINES:
            raise SystemExit(f"unknown engine {engine!r}; "
                             f"known: {' '.join(ENGINES)}")
        self.unknown = collections.Counter()
        counts = collections.Counter()
        # Which functions the universe holds per file, so a TU-level clean
        # can be written for every one of them.
        by_file: dict[tuple, list] = {}
        for (t, f, fn) in self.rows:
            if t == tree:
                by_file.setdefault((t, f), []).append(fn)

        with path.open() as fh:
            for line in fh:
                line = line.strip()
                if not line:
                    continue
                d = json.loads(line)
                if d.get("_meta"):
                    continue
                f = d.get("file")
                if f is None:
                    continue
                if engine in FUNC_ENGINES:
                    fn = d.get("function")
                    if fn is None:
                        continue
                    v = d.get("status", "ERROR")
                    if v not in VERDICTS:
                        v = "ERROR"
                    self._set((tree, f, fn), engine, v)
                    counts[v] += 1
                else:
                    status = d.get("status", "ERROR")
                    fns = by_file.get((tree, f), ())
                    if status == "ERROR":
                        for fn in fns:
                            self._set((tree, f, fn), engine, "TU-ERROR")
                        counts["TU-ERROR"] += len(fns)
                        continue
                    named = {g.get("fn") for g in d.get("findings", ())
                             if g.get("fn")}
                    for fn in fns:
                        v = "REPORTED" if fn in named else "TU-CLEAN"
                        self._set((tree, f, fn), engine, v)
                        counts[v] += 1
                    # A finding naming a function the universe does not
                    # hold is the universe being wrong; count it.
                    for g in named - set(fns):
                        self.unknown[engine] += 1
        self.provenance[engine] = str(path)
        counts["_unknown"] = self.unknown[engine]
        return counts


# -------------------------------------------------------------------- report

def top_scope(tree: str, f: str, depth: int = 2) -> str:
    parts = f.split("/")
    return tree + "/" + "/".join(parts[:depth])


def report(m: Matrix, by_scope: bool, depth: int):
    tot = collections.Counter()
    why = collections.Counter()
    per_engine = collections.defaultdict(collections.Counter)
    scopes = collections.defaultdict(collections.Counter)

    for key, cells in m.rows.items():
        s = strength(cells)
        tot[s] += 1
        if s == S_UNTOUCHED:
            why[why_untouched(cells)] += 1
        for eng, v in cells.items():
            per_engine[eng][v] += 1
        if by_scope:
            scopes[top_scope(key[0], key[1], depth)][s] += 1

    n = sum(tot.values())
    touched = n - tot[S_UNTOUCHED]
    print(f"== rows (functions the universe knows about)   {n}")
    print(f"   TOUCHED   {touched:>8}  {touched / n if n else 0:.3f}")
    print(f"     PROVED  {tot[S_PROVED]:>8}   some engine discharged the "
          "checked properties")
    print(f"     CHECKED {tot[S_CHECKED]:>8}   some engine returned a verdict")
    print(f"     SCANNED {tot[S_SCANNED]:>8}   only TU-level instruments "
          "looked")
    print(f"   UNTOUCHED {tot[S_UNTOUCHED]:>8}  "
          f"{tot[S_UNTOUCHED] / n if n else 0:.3f}")
    print(f"     never attempted      {why['never-attempted']:>8}   "
          "needs a RUN")
    print(f"     attempted, no answer {why['attempted-no-answer']:>8}   "
          "needs a BUILD FIXED or a BOUND RAISED")

    print("\n== per instrument")
    if not per_engine:
        print("   nothing ingested. Every row is NOTRUN, and that is not")
        print("   the same as every row being clean.")
    for eng in sorted(per_engine):
        c = per_engine[eng]
        real = sum(v for k, v in c.items() if k not in NO_VERDICT)
        print(f"   {eng:20} cells {sum(c.values()):>8}  with a verdict "
              f"{real:>8}")
        for v, k in sorted(c.items(), key=lambda kv: -kv[1]):
            print(f"       {v:18} {k}")
        if eng in m.provenance:
            print(f"       from {m.provenance[eng]}")

    if by_scope:
        print(f"\n== by scope (depth {depth}), worst first")
        rows = []
        for sc, c in scopes.items():
            tt = sum(c.values())
            rows.append((c[S_UNTOUCHED] / tt if tt else 1.0, tt, sc, c))
        rows.sort(reverse=True)
        print(f"   {'scope':44} {'rows':>7} {'untouched':>10} {'frac':>6}")
        for frac, tt, sc, c in rows:
            print(f"   {sc:44} {tt:>7} {c[S_UNTOUCHED]:>10} {frac:>6.2f}")


def list_untouched(m: Matrix, scope: str | None, limit: int, reason: str | None):
    n = 0
    for key in sorted(m.rows):
        cells = m.rows[key]
        if strength(cells) != S_UNTOUCHED:
            continue
        tree, f, fn = key
        full = f"{tree}/{f}"
        if scope and not full.startswith(scope):
            continue
        w = why_untouched(cells)
        if reason and w != reason:
            continue
        seen = ",".join(f"{k}={v}" for k, v in sorted(cells.items())) or "-"
        print(f"{w:20} {full}:{fn}  [{seen}]")
        n += 1
        if limit and n >= limit:
            break
    print(f"\n{n} listed")


# ---------------------------------------------------------------------- main

def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--universe", default="verify_universe.jsonl",
                    help="inventory.py output; used when --matrix is absent")
    ap.add_argument("--matrix", help="an existing matrix to extend")
    ap.add_argument("--ingest", action="append", default=[], metavar="SPEC",
                    help="ENGINE[@TREE]=PATH.jsonl, repeatable (default "
                         "tree: hbsd)")
    ap.add_argument("--out", help="write the matrix here")
    ap.add_argument("--report", action="store_true")
    ap.add_argument("--by-scope", action="store_true")
    ap.add_argument("--depth", type=int, default=2)
    ap.add_argument("--untouched", action="store_true",
                    help="list the rows nothing has a verdict for")
    ap.add_argument("--reason", choices=["never-attempted",
                                         "attempted-no-answer"])
    ap.add_argument("--scope", help="prefix filter, e.g. hbsd/sys/kern")
    ap.add_argument("--limit", type=int, default=100)
    ap.add_argument("--engines", action="store_true",
                    help="list the engine names --ingest accepts")
    args = ap.parse_args(argv)

    if args.engines:
        print("per function:", " ".join(sorted(FUNC_ENGINES)))
        print("per translation unit:", " ".join(sorted(TU_ENGINES)))
        return 0

    m = Matrix()
    if args.matrix:
        n = m.load(Path(args.matrix))
        print(f"matrix {args.matrix}: {n} rows", file=sys.stderr)
    else:
        n = m.load_universe(Path(args.universe))
        print(f"universe {args.universe}: {n} rows", file=sys.stderr)

    for spec in args.ingest:
        if "=" not in spec:
            raise SystemExit(f"--ingest wants ENGINE[@TREE]=PATH, got {spec!r}")
        lhs, path = spec.split("=", 1)
        engine, _, tree = lhs.partition("@")
        tree = tree or "hbsd"
        counts = m.ingest(engine, tree, Path(path))
        unk = counts.pop("_unknown", 0)
        print(f"ingest {engine}@{tree} <- {path}", file=sys.stderr)
        for v, k in sorted(counts.items(), key=lambda kv: -kv[1]):
            print(f"    {v:18} {k}", file=sys.stderr)
        if unk:
            print(f"    {'NOT IN UNIVERSE':18} {unk}   <- the universe is "
                  "incomplete, not the run", file=sys.stderr)

    if args.out:
        m.save(Path(args.out))
        print(f"wrote {args.out}", file=sys.stderr)

    if args.untouched:
        list_untouched(m, args.scope, args.limit, args.reason)
    if args.report or not (args.untouched or args.out):
        report(m, args.by_scope, args.depth)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
