#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Every instrument over a scope, resumable, with what is missing NAMED.

This is the one command. It runs each instrument this tree can drive over
a scope, in an order that respects what feeds what, writes each one's
results where matrix.py can ingest them, and finishes by printing the
three numbers that matter together.

The rule it exists to enforce
-----------------------------
A stage that cannot run WRITES THAT DOWN. It does not skip quietly, it
does not exit 0 with nothing, and it never leaves the absence of a result
looking like a clean result. Every stage lands in the state file with one
of:

    ok        it ran and produced output
    failed    it ran and the command returned non-zero
    NOTRUN    its tool is not installed, and the install line is printed
    missing   its driver is not in this tree yet, and that is named too
    skipped   the operator excluded it with --stage / --skip

`--report` prints every one of those, always, so the last thing on screen
after a twelve-hour run is the list of what did NOT happen.

Profiling, so an estimate is a measurement
------------------------------------------
`--profile SCOPE` runs the whole pipeline over a small scope and records
seconds-per-unit for each stage in `rates.json`. `--dry-run` then reads
that file and extrapolates to the real scope, printing the sample size it
extrapolated FROM beside every figure. With no rates.json it says
"not measured" rather than inventing a number, because a wrong estimate
on a twelve-hour job is worse than no estimate.

Resuming
--------
State lives in `<out>/state.json`. `--resume` skips stages already `ok`.
A killed run loses at most the stage it was in, and the per-stage drivers
that support `--resume` are given it, so even that stage restarts from
its own partial file rather than from the beginning.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
V = ROOT / "tools" / "verify"


class Stage:
    """One instrument, and everything needed to decide whether it can run."""

    def __init__(self, name, engine, needs_bin, driver, argv,
                 feeds=None, install=None, note="", kind="c",
                 fixed_cost=False):
        self.name = name
        self.engine = engine          # matrix.py column, or None
        self.needs_bin = needs_bin    # executable that must be on PATH
        self.driver = driver          # path under the tree, or None
        self.argv = argv              # callable(ctx) -> list[str]
        self.feeds = feeds or []      # stages that must be ok first
        self.install = install or {}  # {"pkg": ..., "apt": ...}
        self.note = note
        self.kind = kind              # c | cxx | meta
        # A stage whose cost does not scale with the scope. The universe
        # is built over the WHOLE tree whatever --scope says, so pricing
        # it per unit of the scope inflates a tree-wide estimate by the
        # square of the tree.
        self.fixed_cost = fixed_cost

    def availability(self) -> tuple[str, str]:
        if self.driver and not (ROOT / self.driver).exists():
            return "missing", f"{self.driver} is not in this tree yet"
        if self.needs_bin and not shutil.which(self.needs_bin):
            bits = []
            if self.install.get("pkg"):
                bits.append(f"pkg install {self.install['pkg']}")
            if self.install.get("apt"):
                bits.append(f"apt install {self.install['apt']}")
            how = "; ".join(bits) or "no package - see tools/verify/taxonomy.py --missing"
            return "NOTRUN", f"{self.needs_bin} not on PATH  ({how})"
        return "ok", ""


def _scopes(ctx):
    out = []
    for s in ctx["scopes"]:
        out += ["--scope", s]
    return out


STAGES = [
    Stage("universe", None, None, "tools/verify/inventory.py",
          lambda c: [sys.executable, str(V / "inventory.py"),
                     "--out", str(c["dir"] / "universe.jsonl")],
          kind="meta", fixed_cost=True,
          note="the row set every coverage fraction is divided by"),

    Stage("classify", None, "goto-cc", "tools/verify/classify.py",
          lambda c: [sys.executable, str(V / "classify.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "classes.json")]
                    + (["--resume"] if c["resume"] else []),
          install={"pkg": "cbmc", "apt": "cbmc"},
          note="goto models; cbmc and the class index both need it"),

    Stage("cbmc", "cbmc", "cbmc", "tools/verify/cbmc_driver.py",
          lambda c: [sys.executable, str(V / "cbmc_driver.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--classes", str(c["dir"] / "classes.json"),
                     "--timeout", str(c["timeout"]),
                     "--out", str(c["dir"] / "cbmc.jsonl")]
                    + (["--resume"] if c["resume"] else []),
          feeds=["classify"], install={"pkg": "cbmc", "apt": "cbmc"},
          note="the only instrument here that PROVES anything"),

    Stage("esbmc", "esbmc", "esbmc", "tools/verify/esbmc_driver.py",
          lambda c: [sys.executable, str(V / "esbmc_driver.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--classes", str(c["dir"] / "classes.json"),
                     "--out", str(c["dir"] / "esbmc.jsonl"),
                     "--mode", "kinduction"]
                    + (["--resume", "--retry-status", "ERROR"]
                       if c["resume"] else []),
          feeds=["classify"],
          note="k-induction: the only route here to a verdict that is not "
               "bounded by an unwind depth. No distribution packages it; "
               "a release binary from github.com/esbmc/esbmc goes on PATH"),

    Stage("fusebmc", "fusebmc", "cbmc", "tools/verify/fusebmc.py",
          lambda c: [sys.executable, str(V / "fusebmc.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "fusebmc.jsonl")]
                    + (["--resume"] if c["resume"] else []),
          install={"pkg": "cbmc", "apt": "cbmc"},
          note="FuSeBMC's method - fuzz for seeds, then model check - "
               "over the model checker this tree already has"),

    Stage("analyze", "clang-analyze", "clang", "tools/verify/analyze.py",
          lambda c: [sys.executable, str(V / "analyze.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "analyze.jsonl")],
          install={"pkg": "llvm", "apt": "clang"},
          note="path-sensitive, interprocedural WITHIN a translation unit"),

    Stage("tidy", "clang-tidy", "clang-tidy",
          "tools/verify/clang_tidy_driver.py",
          lambda c: [sys.executable, str(V / "clang_tidy_driver.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "tidy.jsonl")],
          install={"pkg": "llvm", "apt": "clang-tidy"},
          note="bugprone-* and cert-*, which the analyser does not run"),

    Stage("cppcheck", "cppcheck", "cppcheck",
          "tools/verify/cppcheck_driver.py",
          lambda c: [sys.executable, str(V / "cppcheck_driver.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "cppcheck.jsonl")],
          install={"pkg": "cppcheck", "apt": "cppcheck"},
          note="its own value-flow engine; reaches uninitialised struct "
               "members clang's analyser does not"),

    Stage("coccinelle", "coccinelle", "spatch",
          "tools/verify/coccinelle_driver.py",
          lambda c: [sys.executable, str(V / "coccinelle_driver.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "cocci.jsonl")],
          install={"pkg": "coccinelle", "apt": "coccinelle"},
          note="the only instrument that can be TAUGHT a defect shape "
               "found by hand and then swept for it tree-wide"),

    Stage("warnings", "compiler-warnings", "gcc",
          "tools/verify/compiler_warnings.py",
          lambda c: [sys.executable, str(V / "compiler_warnings.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "warnings.jsonl")],
          install={"pkg": "gcc", "apt": "gcc"},
          note="-Wconversion and -Wsign-compare: cheap, noisy, and the "
               "mechanism behind most one-sided index bugs"),

    Stage("cxx-analyze", "cxx-analyze", "clang++",
          "tools/verify/cxx_analyze.py",
          lambda c: [sys.executable, str(V / "cxx_analyze.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "cxx_analyze.jsonl")],
          install={"pkg": "llvm", "apt": "clang"}, kind="cxx",
          note="KDE and hbsd/src's C++; no model checker here handles it"),

    Stage("cxx-tidy", "cxx-tidy", "clang-tidy", "tools/verify/cxx_tidy.py",
          lambda c: [sys.executable, str(V / "cxx_tidy.py"),
                     *_scopes(c), "--jobs", str(c["jobs"]),
                     "--out", str(c["dir"] / "cxx_tidy.jsonl")],
          install={"pkg": "llvm", "apt": "clang-tidy"}, kind="cxx",
          note="use-after-move and the lifetime checks, C++ only"),
]

STAGE_BY_NAME = {s.name: s for s in STAGES}


# ------------------------------------------------------------------- state

def load_state(d: Path) -> dict:
    f = d / "state.json"
    if f.exists():
        return json.loads(f.read_text())
    return {"stages": {}, "started": time.time()}


def save_state(d: Path, st: dict):
    (d / "state.json").write_text(json.dumps(st, indent=1))


REFERENCE_RATES = Path(__file__).resolve().parent / "rates-reference.json"


def load_rates(d: Path) -> dict:
    """This run's own profile if there is one, else the shipped reference.

    The reference is measured on somebody ELSE'S machine and is marked
    as such in every line it produces, because an unlabelled estimate
    from the wrong hardware is worse than no estimate: it is a number
    with a provenance a reader will not think to ask about.
    """
    f = d / "rates.json"
    if f.exists():
        r = json.loads(f.read_text())
        r["_source"] = "this machine"
        return r
    if REFERENCE_RATES.is_file():
        r = json.loads(REFERENCE_RATES.read_text())
        r["_source"] = "reference"
        return r
    return {}


# --------------------------------------------------------------------- run

LEDGER = ROOT / "docs" / "port_plan.json"
LEDGER_HINT = (
    f"{LEDGER} is not on disk. It is generated and .gitignore'd - the "
    "markdown is what is committed - so a fresh checkout has none.\n"
    "  regenerate it with: python3 tools/port_plan.py")


C_EXT = (".c",)
CXX_EXT = (".cpp", ".cc", ".cxx", ".C")


def scope_files(scopes, exts) -> int:
    """Sources of these extensions under these scopes.

    A scope is resolved under hbsd/src first, then under the repo root,
    because the C drivers take hbsd/src-relative paths and the C++ ones
    also take repo-root-relative ones (kde/ lives there).
    """
    n = 0
    for s in scopes:
        for base in (ROOT / "hbsd" / "src" / s, ROOT / s):
            if not base.is_dir():
                continue
            for e in exts:
                n += sum(1 for _ in base.rglob("*" + e))
            break
    return n


def count_units(scopes) -> tuple[int, str]:
    """(translation units, where the count came from).

    The ledger first, because that is what the C drivers take their
    corpus from and the rate has to be per the same unit both times.

    But the ledger names NOTHING under kde/ - 0 of 35,050 records - so a
    KDE scope counted that way is zero, every per-unit estimate reads
    0.00 h, and a three-hour job prints as free. That is the same lie as
    a missing ledger, one level in. So a scope the ledger does not name
    falls back to counting sources on disk, and the SOURCE of the count
    is returned with it and printed, because a disk count and a ledger
    count do not mean the same thing.

    A missing ledger is still -1 and still refuses.
    """
    if not LEDGER.is_file():
        return (-1, "none")
    plan = json.loads(LEDGER.read_text())
    n = 0
    for rec in plan["records"]:
        p = rec["path"]
        if any(p.startswith(s) for s in scopes) and p.endswith(
                (".c", ".cpp", ".cc", ".cxx")):
            n += 1
    if n:
        return (n, "ledger")
    on_disk = scope_files(scopes, C_EXT + CXX_EXT)
    return (on_disk, "disk" if on_disk else "nothing")


def run_stage(st: Stage, ctx, state, dry) -> dict:
    avail, why = st.availability()
    if avail != "ok":
        return {"status": avail, "why": why}
    # A C scope handed to a C++ stage is not a failure of the stage. The
    # C++ drivers correctly refuse a scope that matches no files - "NOT
    # RUN, and not a clean scope" - but the refusal is sweep_all's fault
    # for asking, and a `failed' line that means nothing is worse than
    # useless: it teaches a reader to skim the failure list, which is the
    # one part of this output that has to stay trustworthy.
    if st.kind in ("c", "cxx"):
        exts = C_EXT if st.kind == "c" else CXX_EXT
        if scope_files(ctx["scopes"], exts) == 0:
            return {"status": "skipped", "kind_mismatch": True,
                    "why": f"no {st.kind} sources under these scopes; "
                           f"there is nothing here for it to cover"}
    satisfied = {"ok", "would-run"} if dry else {"ok"}
    for dep in st.feeds:
        if state["stages"].get(dep, {}).get("status") not in satisfied:
            return {"status": "blocked",
                    "why": f"{dep} did not complete, and {st.name} reads "
                           f"what it writes"}
    cmd = st.argv(ctx)
    if dry:
        return {"status": "would-run", "cmd": " ".join(cmd)}
    t0 = time.time()
    print(f"\n>>> {st.name}: {' '.join(cmd)}", flush=True)
    p = subprocess.run(cmd, cwd=ROOT)
    dt = time.time() - t0
    return {"status": "ok" if p.returncode == 0 else "failed",
            "rc": p.returncode, "seconds": round(dt, 1),
            "cmd": " ".join(cmd)}


def do_report(d: Path, state, scopes, units, rates):
    print("\n" + "=" * 68)
    print("== what ran, and what did not")
    print(f"   scope: {' '.join(scopes)}   units: {units}")
    order = ["ok", "failed", "blocked", "NOTRUN", "missing", "skipped",
             "would-run"]
    for want in order:
        hits = [(n, r) for n, r in state["stages"].items()
                if r.get("status") == want]
        if not hits:
            continue
        print(f"\n  {want}")
        for n, r in hits:
            extra = r.get("why") or r.get("cmd", "")
            secs = f"  {r['seconds']}s" if "seconds" in r else ""
            print(f"    {n:14}{secs}  {extra[:96]}")
    missing = [n for n, r in state["stages"].items()
               if r.get("status") in ("NOTRUN", "missing", "blocked",
                                      "failed")]
    mism = [n for n, r in state["stages"].items() if r.get("kind_mismatch")]
    print(f"\n  {len(missing)} of {len(state['stages'])} instruments "
          "produced NO data for this scope.")
    if mism:
        print(f"  ({len(mism)} more were skipped because this scope holds "
              f"no source of their kind:")
        print(f"   {' '.join(sorted(mism))} - that is not a coverage gap,")
        print("   there was nothing there for them to cover.)")
    if missing:
        print("  Every function they would have covered is UNTOUCHED, not")
        print("  clean. Run `python3 tools/verify/taxonomy.py --missing'")
        print("  for what each one would have bought.")
    if rates:
        print("\n  measured rates (seconds per translation unit)")
        for k, v in sorted(rates.get("per_unit", {}).items()):
            print(f"    {k:14} {v:8.3f}   from {rates.get('units', '?')} units")
        if rates.get("fixed"):
            print("\n  fixed costs (seconds, independent of the scope)")
            for k, v in sorted(rates["fixed"].items()):
                print(f"    {k:14} {v:8.1f}")


def do_matrix(d: Path, state):
    """Fold whatever landed into the matrix and print the coverage."""
    ing = []
    for st in STAGES:
        if st.engine is None:
            continue
        r = state["stages"].get(st.name, {})
        if r.get("status") != "ok":
            continue
        name = {"cbmc": "cbmc.jsonl", "esbmc": "esbmc.jsonl",
                "fusebmc": "fusebmc.jsonl", "clang-analyze": "analyze.jsonl",
                "clang-tidy": "tidy.jsonl", "cppcheck": "cppcheck.jsonl",
                "coccinelle": "cocci.jsonl",
                "compiler-warnings": "warnings.jsonl",
                "cxx-analyze": "cxx_analyze.jsonl",
                "cxx-tidy": "cxx_tidy.jsonl"}[st.engine]
        f = d / name
        if f.exists():
            ing += ["--ingest", f"{st.engine}={f}"]
    if not (d / "universe.jsonl").exists():
        print("\n  no universe.jsonl: the matrix cannot be built, and that")
        print("  means there is no denominator, not that there is no work.")
        return
    cmd = [sys.executable, str(V / "matrix.py"),
           "--universe", str(d / "universe.jsonl"), *ing,
           "--out", str(d / "matrix.jsonl"), "--report", "--by-scope"]
    print(f"\n>>> matrix: {' '.join(cmd[:6])} ... ({len(ing)//2} engines)")
    subprocess.run(cmd, cwd=ROOT)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--scope", action="append", default=[],
                    help="path prefix under hbsd/src, repeatable")
    ap.add_argument("--out", default="sweep_out", help="results directory")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--timeout", type=int, default=60,
                    help="seconds per function, for the model checkers")
    ap.add_argument("--stage", action="append", default=[],
                    help="run only these stages, repeatable")
    ap.add_argument("--skip", action="append", default=[], help="repeatable")
    ap.add_argument("--kind", choices=["c", "cxx", "all"], default="all")
    ap.add_argument("--resume", action="store_true",
                    help="skip stages already ok, and pass --resume on")
    ap.add_argument("--dry-run", action="store_true",
                    help="print the plan and the estimate, run nothing")
    ap.add_argument("--profile", action="store_true",
                    help="record seconds-per-unit into rates.json")
    ap.add_argument("--list", action="store_true")
    args = ap.parse_args(argv)

    if args.list:
        print(f"{'stage':14} {'engine':18} {'needs':12} status")
        for s in STAGES:
            a, why = s.availability()
            print(f"  {s.name:14} {str(s.engine):18} "
                  f"{str(s.needs_bin):12} {a:8} {why}")
        return 0

    if not args.scope:
        ap.error("--scope is required; there is no whole-tree default, "
                 "because a twelve-hour run should be asked for explicitly")

    d = Path(args.out)
    d.mkdir(parents=True, exist_ok=True)
    state = load_state(d) if args.resume else {"stages": {},
                                               "started": time.time()}
    rates = load_rates(d)
    units, unit_src = count_units(args.scope)
    ctx = {"dir": d, "scopes": args.scope, "jobs": args.jobs,
           "timeout": args.timeout, "resume": args.resume}

    if units < 0:
        print(f"sweep_all: {LEDGER_HINT}", file=sys.stderr)
        print("  Without it there is no unit count, so no estimate and no",
              file=sys.stderr)
        print("  denominator. Refusing rather than reporting zero units.",
              file=sys.stderr)
        return 2
    if units == 0:
        print(f"sweep_all: no source found under {' '.join(args.scope)}.",
              file=sys.stderr)
        print("  Neither the ledger nor the filesystem names a .c or .cpp",
              file=sys.stderr)
        print("  there. An empty scope is NOT RUN, not a clean one.",
              file=sys.stderr)
        return 2
    print(f"== scope {' '.join(args.scope)}   {units} translation units "
          f"(counted from the {unit_src})   {args.jobs} jobs")
    if unit_src == "disk":
        print("   the port ledger names nothing under this scope, so the "
              "count is a")
        print("   file count rather than a corpus count - the C engines "
              "take their")
        print("   work from the ledger and will find less than this.")
    scale = 1.0
    if args.dry_run:
        pu = rates.get("per_unit", {})
        if pu:
            pjobs = rates.get("jobs", args.jobs) or args.jobs
            scale = pjobs / args.jobs
            where = rates.get("_source", "?")
            print(f"   estimates extrapolated from a profile of "
                  f"{rates.get('units')} units at {pjobs} jobs")
            if where == "reference":
                print("   *** that profile is the SHIPPED REFERENCE, "
                      "measured on another")
                print(f"   *** machine ({rates.get('_host', 'unknown')}). "
                      "It is not yours.")
                print("   *** Run --profile on a small scope to replace it.")
            if args.jobs != pjobs:
                print(f"   scaled x{scale:.2f} for {args.jobs} jobs, "
                      "assuming perfect parallelism - so these are a "
                      "FLOOR, not a prediction")
        else:
            print("   no rates.json: times are NOT MEASURED and are not "
                  "guessed either")

    chosen = [s for s in STAGES
              if (not args.stage or s.name in args.stage)
              and s.name not in args.skip
              and (args.kind == "all" or s.kind in (args.kind, "meta"))]

    for s in STAGES:
        if s not in chosen:
            state["stages"].setdefault(s.name, {"status": "skipped",
                                                "why": "not selected"})

    total_est = 0.0
    for s in chosen:
        if args.resume and state["stages"].get(s.name, {}).get("status") == "ok":
            print(f"    {s.name:14} ok (resumed)")
            continue
        r = run_stage(s, ctx, state, args.dry_run)
        state["stages"][s.name] = r
        if args.dry_run:
            if s.fixed_cost:
                cost = rates.get("fixed", {}).get(s.name)
            else:
                per = rates.get("per_unit", {}).get(s.name)
                # The rate was measured at rates["jobs"] parallel workers.
                # Scaling it to a different core count assumes the stage
                # is perfectly parallel, which it is NOT - the model
                # checkers come close, the ones bounded by disk do not.
                # So this is a floor on the time, not a prediction, and
                # the line below says so.
                cost = per * units * scale if per else None
            est = (f"{cost / 3600:.2f} h" if cost and cost >= 60
                   else f"{cost:.0f} s" if cost else "not measured")
            if cost:
                total_est += cost
            print(f"    {s.name:14} {r['status']:10} {est:>14}  {s.note[:44]}")
        save_state(d, state)

    if args.dry_run:
        if total_est:
            print(f"\n   estimated total: {total_est / 3600:.1f} h at "
                  f"{args.jobs} jobs, from a {rates.get('units')}-unit "
                  f"profile taken at {rates.get('jobs')} jobs")
            miss = [n for n, r in state["stages"].items()
                    if r.get("status") in ("NOTRUN", "missing")]
            if miss:
                print(f"   and that total EXCLUDES {len(miss)} instruments "
                      "that cannot run here:")
                print(f"     {' '.join(sorted(miss))}")
                print("   Installing them makes the run LONGER and the "
                      "coverage larger. Neither number is in the total.")
        do_report(d, state, args.scope, units, rates)
        return 0

    if args.profile:
        pu, fixed = {}, {}
        for n, r in state["stages"].items():
            if r.get("status") != "ok" or not r.get("seconds"):
                continue
            if STAGE_BY_NAME[n].fixed_cost:
                fixed[n] = r["seconds"]
            elif units:
                pu[n] = r["seconds"] / units
        (d / "rates.json").write_text(json.dumps(
            {"units": units, "scopes": args.scope, "jobs": args.jobs,
             "per_unit": pu, "fixed": fixed}, indent=1))
        print(f"\nwrote {d / 'rates.json'} from {units} units")
        rates = load_rates(d)

    do_matrix(d, state)
    do_report(d, state, args.scope, units, rates)

    bad = [n for n, r in state["stages"].items()
           if r.get("status") in ("failed", "blocked")]
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
