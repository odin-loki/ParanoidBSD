#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Coccinelle over hbsd/src - patterns written from defects already found.

Everything else in this tier is a general-purpose analyser being pointed
at a kernel.  Coccinelle is the opposite: it was written FOR kernel C, by
people maintaining Linux, and a semantic patch is a pattern in the
language the code is actually written in rather than in the analyser's
internal model.  It matches on unpreprocessed source, so an `#ifdef' that
a build configuration would have deleted is still there to match, and it
does not need the translation unit to compile.

What makes it worth having here is not generality.  It is that the
patterns can be written FROM THIS TREE'S OWN FINDINGS.  Every rule in
cocci/ cites the docs/security/UB_FINDINGS.md entry it came from:

    tautological_bound.cocci   MPASS(idx >= 0 || idx < MAX) - the bounds
                               assertion that asserts nothing
    masked_index.cocci         pci_ea_fill_info's three-bit PCIM_EA_ES
                               indexing a four-dword array
    alloc_nocheck.cocci        an M_NOWAIT allocation dereferenced with no
                               NULL test, including the two wrapper
                               spellings nowait_check.py's token test
                               could not see
    realloc_self.cocci         tftp and ppp: realloc written back over the
                               only pointer to the block
    shift_bit31.cocci          lge, sis, DC_DEVID, pci.c:6096 - a left
                               shift that reaches bit 31 of an int
    onesided_bound.cocci       es_vlangroup: an upper-bound test with no
                               `< 0' beside it, on a signed index

A pattern that was written from a defect is a pattern that has at least
one true positive by construction, which is a much better starting point
than a checker somebody else's corpus tuned.

PARSE COVERAGE is this driver's version of the ERROR problem, and it is
sharper here than anywhere else in the tier, because Coccinelle does not
fail when it cannot parse something - it SKIPS that function and carries
on.  A file it read at 60% reports findings from 60% of the code and
looks exactly like a file it read whole.  So every translation unit is
probed with --parse-c first and the percentage is recorded, the summary
prints the distribution, and --min-parsed turns a bad one into an ERROR.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbsd_breadth as B  # noqa: E402

TOOL = "coccinelle"
VERSION = 1
INSTALL = ("apt-get install -y coccinelle    # if the archive's solver "
           "refuses (a broken libz3-dev pin will do it): apt-get download "
           "coccinelle libparmap-ocaml ocaml-findlib ocaml-base "
           "ocaml-base-nox libpcre-ocaml && dpkg -i --force-depends ./*.deb")

COCCI_DIR = Path(__file__).resolve().parent / "cocci"

# The one line every pattern in cocci/ prints, so the driver does not have
# to parse six different report formats or read spatch's diff output.
HIT = re.compile(r"^PBSD\t(?P<file>[^\t]*)\t(?P<line>\d+)\t(?P<col>\d+)\t"
                 r"(?P<rule>[^\t\n]+)\t(?P<msg>.*)$", re.M)
# --parse-c's last line.
STATS = re.compile(r"nb good = (\d+),\s+nb bad = (\d+)")
FATAL = re.compile(r"(?:parse error|Fatal error|Exception|"
                   r"minus: parse error|SEMANTIC:|^ERROR)", re.M)

# A finding here is a defect with no reachability argument to make: there
# is no reading of `||' in a bounds assertion, or of `p = realloc(p, n)',
# that is correct.  The rest are candidate lists and are advisory - see
# each .cocci's own Confidence line, which says the same thing.
# Measured over sys/geom + sys/x86 and, for the first two, over the whole
# of sys/: tautological-bound 2 hits tree-wide, both real; shift-bit31 3
# hits in the sample, all three real; realloc-self 0 false once M_WAITOK
# was excluded from the kernel form.  onesided-index and masked-index are
# candidate lists by construction - Coccinelle runs with --no-includes and
# cannot tell a u_int index from an int one - and are advisory.
GATE = {"tautological-bound", "realloc-self", "nowait-deref", "shift-bit31"}


def patterns(d: Path) -> list[Path]:
    return sorted(d.glob("*.cocci"))


def one(job: dict) -> dict:
    src = job["src"]
    dig = B.digest([job["spatch"], *job["pats"], str(job["macro"] or "")])
    common = ["--very-quiet", "--no-show-diff", "--timeout",
              str(job.get("timeout", 180))]
    if job["no_includes"]:
        common.append("--no-includes")
    if job["macro"]:
        common += ["--macro-file", job["macro"]]

    # How much of this file did Coccinelle actually read?  Asked FIRST,
    # because a pattern that matches nothing in a file that half parsed is
    # not the same answer as a pattern that matches nothing in a file that
    # parsed whole, and the record has to be able to tell them apart.
    try:
        pp = subprocess.run([job["spatch"], "--parse-c", src],
                            capture_output=True, text=True,
                            timeout=job.get("timeout", 180), cwd="/tmp")
    except subprocess.TimeoutExpired:
        return {"file": job["rel"], "status": "TIMEOUT", "findings": [],
                "flags": dig, "detail": "--parse-c timed out"}
    except OSError as e:
        return {"file": job["rel"], "status": "ERROR", "detail": str(e),
                "findings": [], "flags": dig}
    m = STATS.search(pp.stdout + pp.stderr)
    good, bad = (int(m.group(1)), int(m.group(2))) if m else (0, 0)
    pct = 100.0 * good / max(1, good + bad)
    if good == 0:
        return {"file": job["rel"], "status": "ERROR",
                "detail": f"Coccinelle parsed none of this file "
                          f"({bad} unparsed token runs)",
                "findings": [], "flags": dig, "parsed": 0.0}
    if pct < job["min_parsed"]:
        return {"file": job["rel"], "status": "ERROR",
                "detail": f"Coccinelle parsed only {pct:.0f}% of this file "
                          f"(--min-parsed {job['min_parsed']})",
                "findings": [], "flags": dig, "parsed": round(pct, 1)}

    out, seen, errs = [], set(), []
    for pat in job["pats"]:
        try:
            p = subprocess.run([job["spatch"], *common, "--sp-file", pat, src],
                               capture_output=True, text=True,
                               timeout=job.get("timeout", 180), cwd="/tmp")
        except subprocess.TimeoutExpired:
            errs.append(f"{Path(pat).name}: timeout")
            continue
        except OSError as e:
            errs.append(f"{Path(pat).name}: {e}")
            continue
        text = p.stdout + "\n" + p.stderr
        if FATAL.search(p.stderr) and not HIT.search(text):
            errs.append(f"{Path(pat).name}: "
                        f"{p.stderr.strip().splitlines()[0][:120]}"
                        if p.stderr.strip() else f"{Path(pat).name}: failed")
            continue
        for h in HIT.finditer(text):
            rel = B.rel_to_src(h.group("file"))
            key = (rel, h.group("line"), h.group("rule"))
            # Two rules in one .cocci can match the same site - see
            # realloc_self.cocci, where `p = realloc(p, n)' and
            # `x->p = realloc(x->p, n)' both fire on a field.  One site,
            # one finding.
            if key in seen:
                continue
            seen.add(key)
            out.append({"where": f"{rel}:{h.group('line')}",
                        "checker": h.group("rule"), "msg": h.group("msg"),
                        "pattern": Path(pat).name})
    rec = {"file": job["rel"], "status": "OK", "findings": out,
           "flags": dig, "parsed": round(pct, 1)}
    if errs:
        # A pattern that failed on this file checked NOTHING on it.  The
        # unit is not clean for that rule and the record says which.
        rec["pattern_errors"] = errs[:5]
    return rec


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    B.add_common_args(ap, "coccinelle_results.jsonl")
    ap.add_argument("--binary", default="spatch")
    ap.add_argument("--cocci-dir", default=str(COCCI_DIR),
                    help="directory of .cocci patterns")
    ap.add_argument("--pattern", action="append",
                    help="repeatable; run only these .cocci by name")
    ap.add_argument("--macro-file",
                    default=str(COCCI_DIR / "pbsd_macros.h"),
                    help="FreeBSD's attribute spellings, for the parser")
    ap.add_argument("--includes", action="store_true",
                    help="let spatch follow #include. Off by default: "
                         "Coccinelle is a text-level matcher and following "
                         "kernel headers costs minutes per file for a "
                         "pattern that does not need the types.")
    ap.add_argument("--min-parsed", type=float, default=0.0,
                    help="percent of a file Coccinelle must parse before "
                         "its findings count. Below it the unit is ERROR: "
                         "a half-read file reports findings from half the "
                         "code and looks exactly like a clean one.")
    args = ap.parse_args()

    pats = patterns(Path(args.cocci_dir))
    if args.pattern:
        want = {Path(x).name for x in args.pattern}
        pats = [p for p in pats if p.name in want]
    if not pats:
        print(f"no .cocci patterns in {args.cocci_dir}", file=sys.stderr)
        return 2

    jobs = B.enumerate_tus(args.scope, args.limit)
    macro = args.macro_file if Path(args.macro_file).is_file() else None
    for j in jobs:
        j["timeout"] = args.timeout
        j["spatch"] = args.binary
        j["pats"] = [str(p) for p in pats]
        j["macro"] = macro
        j["no_includes"] = not args.includes
        j["min_parsed"] = args.min_parsed
    print(f"{len(jobs)} translation unit(s) in scope, "
          f"{len(pats)} pattern(s): " + " ".join(p.name for p in pats),
          flush=True)

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    if not B.have(args.binary):
        B.load_resume(out, TOOL, VERSION, args.resume)
        B.notrun(jobs, out, TOOL, VERSION, INSTALL,
                 {"scopes": args.scope or B.DEFAULT_SCOPES,
                  "patterns": [p.name for p in pats]})
        return 1 if args.gate else 0

    meta = {"analyzer": B.tool_version([args.binary, "--version"]),
            "patterns": [p.name for p in pats],
            "macro_file": macro, "min_parsed": args.min_parsed}
    rc = B.sweep(jobs, one, args, TOOL, VERSION, meta, GATE)

    # Parse coverage, which is this instrument's VISIBILITY number and is
    # invisible in the finding count.
    import json
    pcts = []
    for line in out.read_text().splitlines():
        if not line.strip():
            continue
        try:
            r = json.loads(line)
        except ValueError:
            continue
        if r.get("_meta") or "parsed" not in r:
            continue
        pcts.append(r["parsed"])
    if pcts:
        pcts.sort()
        whole = sum(1 for x in pcts if x >= 99.95)
        print(f"\n== parse coverage over {len(pcts)} translation unit(s)")
        print(f"  {whole} parsed whole, {len(pcts) - whole} partial")
        print(f"  min {pcts[0]:.1f}%  median {pcts[len(pcts)//2]:.1f}%  "
              f"mean {sum(pcts)/len(pcts):.1f}%")
        worst = [x for x in pcts if x < 80]
        if worst:
            print(f"  {len(worst)} unit(s) below 80%: those findings are "
                  f"about the part Coccinelle could read")
    return rc


if __name__ == "__main__":
    sys.exit(main())
