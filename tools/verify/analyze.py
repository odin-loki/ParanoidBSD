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
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from includes import (include_flags, is_kernel_tu, lang_flags,  # noqa: E402
                      files_option_alternatives, files_cpu_alternatives,
                      SRC)
from includes import arch_of, files_opt_arch_index  # noqa: E402
from expected_errors import EXPECTED, NOT_BUILT, not_built  # noqa: E402

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

# The mkdtemp'd directories incs_shim() and rpc_headers() build. Their
# NAMES are a run's own; their contents are what was asked for.
TMPDIR = re.compile(r"(/tmp/pbsd_[a-z]+_)[A-Za-z0-9_]+")

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
    arch = job.get("arch") or arch_of(job["rel"])
    flags = [*lang_flags(src, job["rel"]),
             *include_flags(src, arch, opts=job.get("opts"),
                            cpu=job.get("cpu"))]
    cmd = ["clang", "--analyze", "-Xclang", "-analyzer-output=text",
           *flags, str(src), "-o", "/dev/null"]
    # A finding that appears in one sweep and not the next is either a
    # change in the CODE or a change in the COMMAND, and a record that
    # carries only the finding cannot tell you which.
    # sys/fs/nfsserver/nfs_nfsdport.c:2683 was absent in sweep 13,
    # present in 14 and absent in 15 with nothing in that file touched,
    # and every hypothesis had to be ruled out by hand - the flag list
    # is deterministic across processes, the analyser is deterministic
    # on it, a no-op macro changes nothing - to arrive at "the flags
    # must have differed", which is where the evidence ran out.
    #
    # The temp directories are per-run: incs_shim() and rpc_headers()
    # mkdtemp, so their names are not part of what was asked, only their
    # contents are. Normalised away so two sweeps of the same tree
    # agree.
    digest = hashlib.sha256(
        "\n".join(TMPDIR.sub(r"\1<tmp>", f) for f in flags).encode()
    ).hexdigest()[:12]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True,
                           timeout=job["timeout"], cwd="/tmp")
    except subprocess.TimeoutExpired:
        return {"file": job["rel"], "status": "TIMEOUT", "findings": [],
                "flags": digest}
    except OSError as e:
        return {"file": job["rel"], "status": "ERROR", "detail": str(e),
                "findings": [], "flags": digest}
    if p.returncode != 0 and "error:" in p.stderr:
        # It did not compile for the architecture arch_of() picked. The
        # build system may name one that CAN build it - see
        # includes.files_opt_arch_index(), which is a hint and not an
        # answer precisely because it is only trustworthy here: a file
        # that already compiled is never re-interpreted, and a file that
        # did not has nothing to lose. sys/contrib/alpine-hal is eleven
        # files of ARM barrier intrinsics named by the ARCHITECTURE-
        # NEUTRAL sys/conf/files, `optional al_iofic', and the only
        # place al_iofic appears is two ARM configs.
        # ALL the candidates, in turn. `optional gpioregulator fdt' leaves
        # aarch64 and armv7, and sys/dev/gpio/gpioregulator.c compiles as
        # armv7 and not as aarch64 - so trying only the first was trying
        # the wrong one for twelve of sys/dev's files.
        for alt in files_opt_arch_index().get(job["rel"], ()):
            if alt == arch or job.get("retried"):
                continue
            r = analyze(dict(job, arch=alt, retried=True))
            if r["status"] != "ERROR":
                r["arch"] = alt
                return r
        # ...and then the OPTIONS. A file's `optional' clause gives the
        # intersection of its alternatives, which is the only part that
        # is not a guess -- and sys/arm/arm/debug_monitor.c is
        # `optional ddb | gdb' and defines dbg_monitor_init() twice,
        # once inside `#ifdef DDB' and once outside, so with neither it
        # is a redefinition. Same rule as the architecture retry: a file
        # that already compiled is never re-interpreted, and one that
        # did not has nothing to lose.
        if not job.get("retried_opts"):
            for alt in files_option_alternatives().get(job["rel"], ()):
                r = analyze(dict(job, opts=alt, retried_opts=True))
                if r["status"] != "ERROR":
                    r["opts"] = list(alt)
                    return r
        # ...and then the CPU. sys/powerpc/powerpc/trap.c is
        # `standard' - every powerpc kernel builds it - and half of
        # them are AIM and half BOOKE, so the intersection is empty.
        # Its frame_is_trap_inst() reads frame->cpu.booke.esr in the
        # #else of an `#ifdef AIM', and ESR_PTR is inside
        # `#if defined(BOOKE)' in sys/powerpc/include/spr.h:713, so
        # with neither macro the file cannot compile either way.
        if not job.get("retried_cpu"):
            for alt in files_cpu_alternatives().get(
                    job["rel"], {}).get(arch, ()):
                r = analyze(dict(job, cpu=alt, retried_cpu=True))
                if r["status"] != "ERROR":
                    r["cpu"] = list(alt)
                    return r
        return {"file": job["rel"], "status": "ERROR",
                "detail": p.stderr.strip()[-300:], "findings": [],
                "flags": digest}
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
    return {"file": job["rel"], "status": "OK", "findings": out,
            "flags": digest}


DEFAULT_SCOPES = ["lib/libc", "lib/msun"]


def summarise_errors(path: str, top: int = 12) -> None:
    """Why the translation units that did not compile did not compile.

    An ERROR is not a finding, so a file that will not compile contributes
    NOTHING and says so nowhere. That is how the kernel corpus sat at 1,227
    usable translation units out of 6,914 without anybody noticing: the
    summary line said "1227 OK, 5687 ERROR" and the reason was five
    thousand individual messages nobody had bucketed.

    Bucketing them by hand three times found three generated headers the
    build makes and this did not - device_if.h (2,540 files), then
    vnode_if_typedef.h, then usbdevs.h - and each one roughly doubled what
    could be checked. So it is a mode now rather than a one-liner rewritten
    each time.
    """
    import collections
    buckets: collections.Counter = collections.Counter()
    example: dict[str, str] = {}
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            for line in fh:
                if not line.strip():
                    continue
                try:
                    r = json.loads(line)
                except ValueError:
                    continue
                if r.get("status") != "ERROR":
                    continue
                d = r.get("detail") or ""
                m = re.search(r"fatal error: '([^']+)' file not found", d)
                if m:
                    key = f"missing header: {m.group(1)}"
                else:
                    m2 = re.search(r"error: (.{0,72})", d)
                    key = (m2.group(1) if m2 else d[-72:]).strip()
                    key = re.sub(r"\s+", " ", key)
                buckets[key] += 1
                example.setdefault(key, r["file"])
    except OSError:
        return
    if not buckets:
        return
    print(f"\n== why {sum(buckets.values())} translation unit(s) did not "
          "compile")
    print("   A file that does not compile contributes no findings and "
          "says so nowhere.")
    for key, n in buckets.most_common(top):
        print(f"  {n:5d}  {key}")
        print(f"         e.g. {example[key]}")
    rest = len(buckets) - min(top, len(buckets))
    if rest > 0:
        print(f"  ...and {rest} more distinct reason(s)")


def analyzer_version() -> str:
    """The exact clang that produced this run's findings."""
    try:
        out = subprocess.run(["clang", "--version"], capture_output=True,
                             text=True, timeout=20)
        return (out.stdout or out.stderr).splitlines()[0].strip()
    except (OSError, IndexError, subprocess.SubprocessError) as e:
        return f"unknown ({e})"


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
    ap.add_argument("--check-errors", action="store_true",
                    help="fail if a translation unit does not compile and is "
                         "not in tools/verify/expected_errors.py, or if one "
                         "listed there compiles now. An ERROR contributes no "
                         "findings and reads as clean in every total.")
    args = ap.parse_args()

    jobs = []
    for s in (args.scope or DEFAULT_SCOPES):
        for pat in ("*.c", "*.cpp"):
            for f in sorted((SRC / s).rglob(pat)):
                rel = f.relative_to(SRC).as_posix()
                # Userland C under sys/ - see includes.NOT_KERNEL. It is
                # skipped rather than recorded as ERROR, because "wants
                # <stdio.h> under -D_KERNEL" is not a finding about the
                # kernel and 700+ of them drowned the ones that were.
                if not is_kernel_tu(rel):
                    continue
                jobs.append({"src": str(f), "rel": rel,
                             "timeout": args.timeout})
    if args.limit:
        jobs = jobs[:args.limit]
    print(f"{len(jobs)} translation units to analyse", flush=True)

    counts, nfind, t0 = {}, 0, time.time()
    with open(args.out, "w") as fh, ProcessPoolExecutor(args.jobs) as ex:
        # First record: what produced these numbers.
        #
        # Two sweeps were compared - one from a workstation, one from a
        # GitHub runner - and a three-finding difference was attributed
        # to a source change. Nothing in either file said which clang
        # had run, and clang's analyser changes between releases, so
        # that attribution could not be made at all. A finding count is
        # only comparable to another count from the same checker.
        #
        # Marked with "_meta" so every reader that iterates findings
        # skips it: it has no "findings" key.
        fh.write(json.dumps({
            "_meta": True,
            "analyzer": analyzer_version(),
            "scopes": args.scope or DEFAULT_SCOPES,
            "units": len(jobs),
        }) + "\n")
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
    if counts.get("ERROR"):
        summarise_errors(args.out)

    # An ERROR is invisible in every total this prints: no findings, and
    # a file that reports none looks exactly like one that is clean.
    # rtld.c was ERROR on an unresolvable `#include "notes.h"' and the
    # defect behind it cost thirty-seven boot runs, so the ERROR set is
    # inventoried rather than merely summarised.
    if args.check_errors:
        seen, erred = set(), set()
        with open(args.out, encoding="utf-8") as fh:
            for line in fh:
                r = json.loads(line)
                if r.get("_meta"):
                    continue
                seen.add(r["file"])
                if r.get("status") == "ERROR":
                    erred.add(r["file"])
        unexpected = sorted(e for e in erred
                            if e not in EXPECTED and not not_built(e))
        # Only files this run actually looked at can be called stale.
        stale = sorted((set(EXPECTED) & seen) - erred)
        # A prefix is an inventory entry too, so it is reported, and a
        # prefix that absorbed nothing in a run that looked inside it is
        # as stale as a file entry that compiles now.
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
            print(f"FAIL  every file under {p_} compiles now; its "
                  f"NOT_BUILT entry is stale ({NOT_BUILT[p_]})")
        for p_ in sorted(absorbed):
            print(f"      {absorbed[p_]:4d} of {looked[p_]} under {p_} "
                  f"are NOT_BUILT")
        if unexpected or stale or dead:
            print("\n      tools/verify/expected_errors.py is the inventory.")
            print("      A file that will not compile reports zero findings")
            print("      and is indistinguishable from a clean one, which is")
            print("      how libexec/rtld-elf/rtld.c hid the boot bug.")
            return 1
        if erred:
            print(f"\nok    all {len(erred)} ERROR translation unit(s) are "
                  f"on the record")
    return 0


if __name__ == "__main__":
    sys.exit(main())
