#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Shared plumbing for the breadth tier - the cheap whole-tree analysers.

CBMC proves properties of one function at a time.  clang --analyze walks
paths inside one translation unit.  Both are DEEP and both are SLOW, and
between them they still do not look at whole classes of defect: an
implicit narrowing conversion, a shift whose count is a mask, a `realloc`
written back over its only pointer, a `memset` with its arguments the
wrong way round.  Nothing here is path-sensitive.  That is the trade: a
breadth analyser is allowed to be shallow because it is cheap enough to
run over every file every time.

Everything in this tier emits the SAME RECORD as tools/verify/analyze.py,

    {"file": rel, "status": ..., "findings": [...], "flags": digest,
     "tool": name}

so report.py, classify.py and confidence.py can merge the tiers without
knowing which instrument produced a line.

The status vocabulary is the part that matters and the part that is easy
to get wrong:

    OK       the tool ran and this is what it said - INCLUDING the empty
             list, which means clean.
    ERROR    the tool could not read this translation unit.  It
             contributes NO findings.  A file that does not compile is
             not a clean file, and the whole reason analyze.py inventories
             its ERROR set is that the two are indistinguishable in every
             total anybody prints.
    TIMEOUT  same, with a clock as the reason.
    NOTRUN   the INSTRUMENT is absent from this machine.  Not clean, not
             broken source - unchecked.  confidence.py must be able to
             see the difference, so a driver whose tool is missing still
             emits one NOTRUN record per translation unit in scope and an
             install hint in the meta record, rather than an empty file
             that reads as a clean sweep.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path


def _root() -> Path:
    """The paranoidbsd checkout, wherever this file has been dropped."""
    env = os.environ.get("PBSD_ROOT")
    if env and (Path(env) / "hbsd" / "src").is_dir():
        return Path(env)
    here = Path(__file__).resolve()
    for p in here.parents:
        if (p / "hbsd" / "src").is_dir() and (p / "tools" / "verify").is_dir():
            return p
    for p in (Path("/home/user/paranoidbsd"), Path.cwd()):
        if (p / "hbsd" / "src").is_dir():
            return p
    raise SystemExit("cannot find the paranoidbsd checkout; set PBSD_ROOT")


ROOT = _root()
VERIFY = ROOT / "tools" / "verify"
if str(VERIFY) not in sys.path:
    sys.path.insert(0, str(VERIFY))

# Reused, never reimplemented.  includes.py is 5,000 lines of build-system
# archaeology - which generated header each translation unit needs, which
# -D its `optional' clause implies, which architecture can build it at
# all.  A second copy of that would be a second set of answers.
from includes import (SRC, USERLAND_TOP, arch_of, files_cpu_alternatives,  # noqa: E402
                      files_opt_arch_index, files_option_alternatives,
                      include_flags, incs_shim, is_kernel_tu, lang_flags)
import userland_names  # noqa: E402

# Same normalisation analyze.py applies before digesting the flag list:
# incs_shim() and friends mkdtemp, so the directory NAMES are a run's own
# and only their contents are part of what was asked.
TMPDIR = re.compile(r"(/tmp/pbsd_[a-z]+_)[A-Za-z0-9_]+")

# One line of a compiler-shaped diagnostic.  [^:\n] and not [^:] - see the
# comment on analyze.py's DIAG, where a newline-swallowing file group put
# three lines of source context inside the filename of 430 findings.
DIAG = re.compile(r"^(?P<file>[^:\n]+):(?P<line>\d+):(?P<col>\d+): "
                  r"(?P<sev>warning|error|note|style|performance|portability|"
                  r"information): (?P<msg>.*?)\s*\[(?P<checker>[^\]\n]+)\]$",
                  re.M)

DEFAULT_SCOPES = ["sys/geom", "sys/x86"]


def digest(flags: list[str]) -> str:
    """Twelve hex of the command, so two sweeps can be compared at all."""
    return hashlib.sha256(
        "\n".join(TMPDIR.sub(r"\1<tmp>", f) for f in flags).encode()
    ).hexdigest()[:12]


def rel_to_src(name: str) -> str:
    try:
        return Path(name).resolve().relative_to(SRC).as_posix()
    except (ValueError, OSError):
        return str(name)


def tu_flags(job: dict, cc: str = "clang") -> list[str]:
    """The compile line for one translation unit, from includes.py."""
    src = Path(job["src"])
    arch = job.get("arch") or arch_of(job["rel"])
    return [*lang_flags(src, job["rel"]),
            *include_flags(src, arch, cc=cc, opts=job.get("opts"),
                           cpu=job.get("cpu"))]


def retries(job: dict) -> list[dict]:
    """The alternative build configurations analyze.py tries on ERROR.

    Same order and same rule: architecture, then the `optional' clause's
    alternatives, then the CPU.  A file that already compiled is never
    re-interpreted; a file that did not has nothing to lose.  The ordering
    is not arbitrary - it is cheapest-first and most-likely-first, and it
    is what took sys/dev from a fraction of its files to 2,419 of 2,612.
    """
    out = []
    arch = job.get("arch") or arch_of(job["rel"])
    if not job.get("retried"):
        for alt in files_opt_arch_index().get(job["rel"], ()):
            if alt != arch:
                out.append(dict(job, arch=alt, retried=True))
    if not job.get("retried_opts"):
        for alt in files_option_alternatives().get(job["rel"], ()):
            out.append(dict(job, opts=alt, retried_opts=True))
    if not job.get("retried_cpu"):
        for alt in files_cpu_alternatives().get(job["rel"], {}).get(arch, ()):
            out.append(dict(job, cpu=alt, retried_cpu=True))
    return out


def with_retries(job: dict, run) -> dict:
    """run(job) once, then over each alternative build until it is not ERROR."""
    r = run(job)
    if r["status"] != "ERROR":
        return r
    for alt in retries(job):
        r2 = run(alt)
        if r2["status"] != "ERROR":
            for k in ("arch", "opts", "cpu"):
                if alt.get(k) is not None and alt.get(k) != job.get(k):
                    r2[k] = list(alt[k]) if isinstance(alt[k], tuple) else alt[k]
            return r2
    return r


def enumerate_tus(scopes: list[str] | None, limit: int | None = None
                  ) -> list[dict]:
    """Every translation unit under --scope, the way analyze.py counts them.

    Userland C that happens to live under sys/ is SKIPPED rather than
    recorded as ERROR: "wants <stdio.h> under -D_KERNEL" is not a finding
    about the kernel, and 700+ of them drowned the ones that were.
    """
    jobs: list[dict] = []
    for s in (scopes or DEFAULT_SCOPES):
        base = SRC / s
        if not base.exists():
            print(f"warning: --scope {s} is not in the tree", file=sys.stderr)
            continue
        for pat in ("*.c", "*.cpp"):
            for f in sorted(base.rglob(pat)):
                rel = f.relative_to(SRC).as_posix()
                if not is_kernel_tu(rel):
                    continue
                jobs.append({"src": str(f), "rel": rel})
    if limit:
        jobs = jobs[:limit]
    return jobs


def warm(jobs: list[dict]) -> None:
    """Build the include shims in the PARENT, once per architecture.

    Not an optimisation.  A cold cache makes every worker run the whole
    bmake walk and write its 1.1MB result non-atomically over the others,
    so a worker can read half of it - and half is not JSON, so the read
    fails and builders() returns {}, which is indistinguishable from
    "no directory names this source".
    """
    arches = sorted({j.get("arch") or arch_of(j["rel"]) for j in jobs})
    want_warm = {j.get("arch") or arch_of(j["rel"]) for j in jobs
                 if j["rel"].split("/")[0] in USERLAND_TOP}
    for a in arches:
        incs_shim(a)
        if a in want_warm:
            userland_names.warm((a,))


def add_common_args(ap: argparse.ArgumentParser, out_default: str) -> None:
    # default=None and not a list: argparse's "append" APPENDS to whatever
    # default it is handed, so a non-empty default makes `--scope X' mean
    # "the default scopes AND X" - a narrowed run that silently does the
    # whole tree, and a superset still contains the answer so nothing says
    # so.  analyze.py lost 1,552 translation units to exactly this.
    ap.add_argument("--scope", action="append",
                    help="repeatable; paths under hbsd/src "
                         f"(default: {' '.join(DEFAULT_SCOPES)})")
    ap.add_argument("--jobs", type=int, default=max(1, os.cpu_count() or 4))
    ap.add_argument("--timeout", type=int, default=180,
                    help="seconds per translation unit")
    ap.add_argument("--out", default=out_default)
    ap.add_argument("--limit", type=int)
    ap.add_argument("--resume", action="store_true",
                    help="keep the records already in --out and check only "
                         "the files missing from it. Without it, --out is "
                         "TRUNCATED: two generations of answers in one file "
                         "is a total nobody can read.")
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 on a gate-subset finding, or on any NOTRUN "
                         "unit. A missing instrument is a failed gate, not a "
                         "pass: nothing checked it.")
    ap.add_argument("--gate-max", type=int, default=0,
                    help="gate-subset findings to tolerate (a baseline)")
    ap.add_argument("--check-errors", action="store_true",
                    help="also fail on a translation unit that does not "
                         "compile and is not in expected_errors.py. That "
                         "inventory was built against analyze.py's clang "
                         "front end, so it is meaningful for the clang-tidy "
                         "and warning drivers and only advisory elsewhere.")


def load_resume(out: Path, tool: str, version: int, resume: bool
                ) -> set[str]:
    """Which files are already answered in --out, and rewrite it if not."""
    if not resume:
        if out.is_file():
            out.write_text("")
        return set()
    if not out.is_file():
        return set()
    done, keep, stale = set(), [], 0
    for line in out.read_text().splitlines():
        if not line.strip():
            continue
        try:
            r = json.loads(line)
        except ValueError:
            continue
        if r.get("_meta"):
            continue
        # A record from an older driver does not mean what this one's
        # fields mean.  Drop it and check the file again rather than
        # resume onto an answer to a different question.
        if r.get("tool") != tool or r.get("v") != version:
            stale += 1
            continue
        # NOTRUN is not an answer - it is the absence of one.  A resumed
        # run on a machine that now HAS the tool must redo them.
        if r.get("status") == "NOTRUN":
            stale += 1
            continue
        done.add(r["file"])
        keep.append(line)
    if stale:
        print(f"  {stale} record(s) are from an older driver or were NOTRUN; "
              f"rechecking those", flush=True)
    out.write_text("".join(x + "\n" for x in keep))
    return done


def tool_version(argv: list[str]) -> str:
    try:
        p = subprocess.run(argv, capture_output=True, text=True, timeout=30)
        return ((p.stdout or p.stderr).splitlines() or ["unknown"])[0].strip()
    except (OSError, subprocess.SubprocessError) as e:
        return f"unknown ({e})"


def notrun(jobs: list[dict], out: Path, tool: str, version: int,
           hint: str, meta: dict) -> int:
    """The instrument is not on this machine.  Say so, per file.

    The temptation is to print one line and exit 0.  That produces an
    empty --out, and an empty --out is what a clean sweep looks like -
    the exact confusion confidence.py's docstring exists to prevent.  So
    every translation unit in scope gets a record saying UNCHECKED, and
    the summary carries the command that would fix it.
    """
    with out.open("a") as fh:
        fh.write(json.dumps(dict(meta, _meta=True, tool=tool, v=version,
                                 status="NOTRUN", install=hint,
                                 units=len(jobs))) + "\n")
        for j in jobs:
            fh.write(json.dumps({"file": j["rel"], "status": "NOTRUN",
                                 "detail": f"{tool} is not installed",
                                 "findings": [], "tool": tool,
                                 "v": version}) + "\n")
    print(f"\nNOTRUN  {tool} is not installed; {len(jobs)} translation "
          f"unit(s) are UNCHECKED, not clean")
    print(f"        install it with:  {hint}")
    return len(jobs)


def summarise_errors(path: Path, top: int = 12) -> None:
    """Why the translation units that did not compile did not compile.

    An ERROR is not a finding, so a file that will not compile contributes
    NOTHING and says so nowhere.  Bucketing these by hand three times is
    what found device_if.h, vnode_if_typedef.h and usbdevs.h, and each one
    roughly doubled what could be checked - so it is a mode, not a
    one-liner rewritten each time.
    """
    buckets: collections.Counter = collections.Counter()
    example: dict[str, str] = {}
    try:
        text = path.read_text(errors="replace")
    except OSError:
        return
    for line in text.splitlines():
        if not line.strip():
            continue
        try:
            r = json.loads(line)
        except ValueError:
            continue
        if r.get("status") not in ("ERROR", "TIMEOUT"):
            continue
        d = r.get("detail") or r.get("status") or ""
        m = re.search(r"fatal error: '([^']+)' file not found", d)
        if m:
            key = f"missing header: {m.group(1)}"
        else:
            m2 = re.search(r"error: (.{0,72})", d)
            key = (m2.group(1) if m2 else d[-72:]).strip()
            key = re.sub(r"\s+", " ", key)
        buckets[key] += 1
        example.setdefault(key, r["file"])
    if not buckets:
        return
    print(f"\n== why {sum(buckets.values())} translation unit(s) could not "
          "be read")
    print("   A file the instrument could not read contributes no findings")
    print("   and is indistinguishable from a clean one in every total.")
    for key, n in buckets.most_common(top):
        print(f"  {n:5d}  {key}")
        print(f"         e.g. {example[key]}")
    rest = len(buckets) - min(top, len(buckets))
    if rest > 0:
        print(f"  ...and {rest} more distinct reason(s)")


def check_expected_errors(path: Path) -> int:
    """analyze.py --check-errors, over any driver's output."""
    try:
        from expected_errors import EXPECTED, NOT_BUILT, not_built
    except ImportError as e:
        print(f"  --check-errors: {e}")
        return 0
    seen, erred = set(), set()
    for line in path.read_text(errors="replace").splitlines():
        if not line.strip():
            continue
        try:
            r = json.loads(line)
        except ValueError:
            continue
        if r.get("_meta"):
            continue
        seen.add(r["file"])
        if r.get("status") in ("ERROR", "TIMEOUT"):
            erred.add(r["file"])
    unexpected = sorted(e for e in erred
                        if e not in EXPECTED and not not_built(e))
    stale = sorted((set(EXPECTED) & seen) - erred)
    absorbed: dict[str, int] = {}
    looked: dict[str, int] = {}
    for f in seen:
        pre = not_built(f)
        if pre:
            looked[pre] = looked.get(pre, 0) + 1
            if f in erred:
                absorbed[pre] = absorbed.get(pre, 0) + 1
    dead = sorted(p for p in looked if not absorbed.get(p))
    for f in unexpected:
        print(f"FAIL  {f} does not compile and is not in EXPECTED")
    for f in stale:
        print(f"FAIL  {f} compiles now; its EXPECTED entry is stale "
              f"({EXPECTED[f]})")
    for p_ in dead:
        print(f"FAIL  every file under {p_} compiles now; its NOT_BUILT "
              f"entry is stale ({NOT_BUILT[p_]})")
    if unexpected or stale or dead:
        print("\n      tools/verify/expected_errors.py is the inventory.")
        return 1
    if erred:
        print(f"\nok    all {len(erred)} unreadable translation unit(s) are "
              f"on the record")
    return 0


def sweep(jobs: list[dict], fn, args, tool: str, version: int, meta: dict,
          gate_checks: set[str], quiet_findings: bool = False) -> int:
    """Run fn over jobs, write --out, print the numbers, decide the gate."""
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    done = load_resume(out, tool, version, args.resume)
    todo = [j for j in jobs if j["rel"] not in done]
    print(f"{len(todo)} translation unit(s) to check"
          + (f", {len(done)} already in {out}" if done else ""), flush=True)
    warm(todo)

    counts: collections.Counter = collections.Counter()
    bycheck: collections.Counter = collections.Counter()
    nfind, t0 = 0, time.time()
    mode = "a" if done else "w"
    with out.open(mode) as fh, ProcessPoolExecutor(args.jobs) as ex:
        if not done:
            # First record: what produced these numbers.  A finding count
            # is only comparable to another count from the same checker,
            # and nothing in a bare .jsonl says which one ran.
            fh.write(json.dumps(dict(meta, _meta=True, tool=tool, v=version,
                                     scopes=args.scope or DEFAULT_SCOPES,
                                     units=len(jobs))) + "\n")
        futs = [ex.submit(fn, j) for j in todo]
        for i, fut in enumerate(as_completed(futs), 1):
            try:
                r = fut.result()
            except Exception as e:                      # noqa: BLE001
                r = {"file": "?", "status": "ERROR", "detail": repr(e),
                     "findings": []}
            r.setdefault("tool", tool)
            r["v"] = version
            counts[r["status"]] += 1
            nfind += len(r["findings"])
            for f in r["findings"]:
                bycheck[f["checker"]] += 1
                if not quiet_findings:
                    print(f"  {f['where']}  [{f['checker']}]\n"
                          f"      {f['msg'][:130]}", flush=True)
            fh.write(json.dumps(r) + "\n")
            fh.flush()
            if i % 25 == 0 or i == len(todo):
                print(f"  [{i}/{len(todo)}] "
                      f"{i/max(1e-9, time.time()-t0):.2f}/s  "
                      f"findings={nfind}  "
                      + "  ".join(f"{k}={v}" for k, v in sorted(counts.items())),
                      flush=True)

    wall = time.time() - t0
    print(f"\n{nfind} finding(s) across {len(todo)} translation unit(s) "
          f"in {wall:.0f}s")
    for k, v in sorted(counts.items()):
        print(f"  {k:8s} {v}")
    if bycheck:
        print("\n== findings by check")
        for k, v in bycheck.most_common():
            mark = "GATE" if k in gate_checks else "    "
            print(f"  {mark} {v:6d}  {k}")
    if counts["ERROR"] or counts["TIMEOUT"]:
        summarise_errors(out)

    rc = 0
    if args.check_errors:
        rc |= check_expected_errors(out)
    if args.gate:
        ngate = sum(v for k, v in bycheck.items() if k in gate_checks)
        if counts["NOTRUN"]:
            print(f"\nFAIL  {counts['NOTRUN']} translation unit(s) NOTRUN: "
                  f"the instrument is missing, so nothing checked them. "
                  f"That is not a pass.")
            rc |= 1
        if ngate > args.gate_max:
            print(f"\nFAIL  {ngate} gate-subset finding(s) "
                  f"(--gate-max {args.gate_max})")
            rc |= 1
        if not rc:
            print(f"\nok    gate clean ({ngate} gate-subset finding(s), "
                  f"{nfind - ngate} advisory)")
    return rc


def have(prog: str) -> bool:
    return shutil.which(prog) is not None
