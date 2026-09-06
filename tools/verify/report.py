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
import functools
import json
import re
import sys
from pathlib import Path

PTR_WORDS = ("dereference", "pointer", "object", "memory never freed",
             "array ",
             # CBMC's memcpy/memmove/memset checks are spelled
             #   memmove source region readable
             #   memset destination region writeable
             # with neither "pointer" nor "dereference" in them, so they
             # went to the READ THESE pile - lib/libc/aarch64/string's
             # bcopy and bzero were the first two entries a person was
             # asked to read, and both are "this function has a
             # precondition", the same as strcat.
             "region readable", "region writeable", "region writable")
# Functions whose inputs come from an extern this run did not model.
EXTERN_DRIVEN = {
    "clock", "alarm", "svc_run", "cap_sandboxed", "significand",
    "significandf", "cosl", "sinl", "tanl", "__rebuild_environ",
    "__enlarge_env", "__remove_putenv",
}


def kinds(rec: dict) -> set[str]:
    return {re.sub(r"^line \d+ ", "", d["desc"]).split(" in ")[0]
            for d in rec.get("failures", [])}


# CBMC saying it cannot model something. Not a property that failed.
#
# `destructors are not yet supported` is emitted for a pthread key with a
# destructor - lib/libc/locale/xlocale.c and lib/libc/resolv/mtctxres.c -
# and it arrived in the FAILED pile beside real arithmetic overflows,
# which is where it does the most damage: a reader who checks two of these
# and finds nothing stops reading the rest.
TOOL_LIMITS = ("not yet supported", "unwinding assertion",
               "no body for callee")


def deciding_failures(rec: dict) -> list:
    """The failures that survived bucket()'s strips, most-specific first.

    Printing rec["failures"][:2] under the "EXPORTED, arithmetic" heading
    shows a reader "dereference failure: pointer NULL" for a record that
    is in that bucket BECAUSE its pointer failures were set aside. The two
    lines then contradict each other and the arithmetic - the reason the
    record is there at all - may not be printed.
    """
    out = [d for d in rec.get("failures", [])
           if not any(w in d.get("desc", "") for w in PTR_WORDS)]
    if rec["file"].startswith("lib/msun"):
        out = [d for d in out if "division by zero" not in d.get("desc", "")] or out
    return out or rec.get("failures", [])


def bucket(rec: dict) -> str:
    k = kinds(rec)
    if not k:
        return "no detail"
    if any(w in x for x in k for w in TOOL_LIMITS):
        return "CBMC could not model it (not a property that failed)"
    # STRIP the pointer failures rather than letting one of them decide the
    # whole record - the same fix the msun comment below already describes,
    # left undone for this case. A translation unit that reports both
    #
    #   dereference failure: pointer NULL in ...
    #   arithmetic overflow on signed shl in ...
    #
    # went to "not a bug" and its overflow was never shown to anybody. In
    # the kernel sweep that hid 9 signed +, 5 signed shl, 3 array upper
    # bound, 2 signed -, 2 signed * and 2 array lower bound behind 176
    # NULL-pointer preconditions.
    #
    # An array bound is memory safety and does not belong in a bucket
    # labelled "a missing precondition"; it is only here because
    # PTR_WORDS has to contain "array " and "object" for CBMC's spelling
    # of the pointer checks.
    k = {x for x in k if not any(w in x for w in PTR_WORDS)}
    if not k:
        return "pointer/memory (a missing precondition, not a bug)"
    # Strip the IEEE-defined case BEFORE deciding, rather than only
    # recognising a record that has nothing else. lib/msun/ld128's cospil,
    # sinpil and tanpil each report `vzero / vzero` alongside a shift, and
    # requiring `k == {"division by zero"}` sent all three to the bucket a
    # person is asked to read.
    if rec["file"].startswith("lib/msun"):
        k = k - {"division by zero"}
        if not k:
            return ("float div-by-zero (IEEE-754 defines it; msun depends "
                    "on it)")
    if rec.get("linkage") == "static":
        return "static (callers constrain the domain - deferred)"
    if rec["function"] in EXTERN_DRIVEN:
        return "extern-driven (an unmodelled return, unconstrained)"
    return "EXPORTED, arithmetic - READ THESE"


ROOT = Path(__file__).resolve().parents[2]
DOC = ROOT / "docs/security/UB_FINDINGS.md"
_TRIAGED = re.compile(r"`([\w./-]+\.(?:c|cpp|h)):(\d+(?:\s*,\s*\d+)*)`")


@functools.lru_cache(maxsize=1)
def triaged() -> frozenset:
    """(path suffix, line) pairs already read and found not to be defects.

    docs/security/UB_FINDINGS.md's last section is the list of findings
    that looked like defects and were not, each with the reasoning that
    killed it. Re-listing them every sweep buries the ones nobody has
    read yet: 44 of them by now, against 65 in the bucket the report
    asks a person to read.

    They are MARKED and still printed, never dropped. A finding whose
    triage was wrong has to stay visible for that to be discoverable, and
    the whole point of writing the reasoning down was that the reasoning
    can be checked.

    Matching is on the path SUFFIX because that is what the document
    contains - `fread.c:129` for a libc path it names once and
    `sys/x86/isa/clock.c:200` for a kernel one it wants to disambiguate.
    """
    if not DOC.is_file():
        return frozenset()
    text = DOC.read_text(errors="replace")
    head = text.find("## Not defects, and why they looked like defects")
    if head < 0:
        return frozenset()
    out = set()
    for name, lines in _TRIAGED.findall(text[head:]):
        for ln in lines.split(","):
            out.add((name, ln.strip()))
    return frozenset(out)


_DESCLINE = re.compile(r"^line (\d+) ")


def desc_line(d: dict):
    """CBMC puts the line inside the description, not in a field of its own."""
    m = _DESCLINE.match(d.get("desc", ""))
    return m.group(1) if m else None


def is_triaged(path: str, line) -> bool:
    known = triaged()
    parts = path.split("/")
    return any((("/".join(parts[k:]), str(line)) in known)
               for k in range(len(parts)))


def is_test_file(path: str) -> bool:
    """A translation unit that ships as a test rather than as the system.

    23% of unix.Malloc is in lib/libc/tests - fortify_string_test.c
    alone has 21 - and a leak in a program that runs once and exits is
    not the same finding as a leak in libc. Reading "149 potential
    leaks" without knowing a fifth of them are tests overstates it.

    Counted separately and never dropped, for the same reason the
    [triaged] marker marks instead of hiding: a finding nobody can see
    is indistinguishable from one that is not there, and a test can
    have a real bug in it too.
    """
    return ("/tests/" in path or "/test/" in path
            or path.endswith(("_test.c", "_test.cpp")))


GENERATOR = re.compile(r"\b(?:RB|ARB|SPLAY|RQ)_(?:GENERATE|PROTOTYPE)\w*\s*\(")
_srccache: dict = {}


def source_line(where: str) -> str:
    """The text of the line a finding names, or "" if unreadable."""
    path, _, ln = where.rpartition(":")
    f = ROOT / "hbsd" / "src" / path
    if f not in _srccache:
        try:
            _srccache[f] = f.read_text(errors="replace").splitlines()
        except OSError:
            _srccache[f] = []
    try:
        return _srccache[f][int(ln) - 1]
    except (ValueError, IndexError):
        return ""


def generated_sites(sites) -> dict:
    """where -> macro name, for findings sitting on a *_GENERATE* line."""
    out = {}
    for (w, _c) in sites:
        m = GENERATOR.search(source_line(w))
        if m:
            out[w] = m.group(0).rstrip("( ")
    return out


def macro_report(sites, floor: int = 8) -> None:
    """Lines the analyser reports many times, which are macro expansions.

    sys/kern/subr_stats.c:351 came back 94 times in one sweep - 38% of
    every core.NullDereference under sys/kern, sys/vm, sys/net, sys/fs and
    sys/ufs put together. The line is

        ARB_GENERATE_STATIC(ctdth32, voistatdata_tdgstctd32, ctdlnk, ctd32cmp);

    a macro that generates an entire array-based red-black tree. Every
    finding inside the generated code is attributed to the one line that
    expanded it, so one macro outvotes every real defect in the sweep.

    Counting sites as well as findings is the fix, and printing the worst
    offenders is what stops the count being read as a defect count. They
    are not suppressed: a real bug in a generated tree is still a real
    bug, and it is still in the .jsonl.

    THE REPETITION HEURISTIC ALONE MISSES THE DISTRIBUTED CASE

    ARB_GENERATE_STATIC is 94 findings at TWO lines, so counting
    repetitions finds it. RB_GENERATE is 30 findings at 30 lines - one
    per file that instantiates a red-black tree - and every one of them
    looks like a lone finding. Same macro, same reason, invisible to a
    threshold.

    So the source line is read as well. 124 of 1626 findings in one
    sweep, 7.6%, sit on a *_GENERATE* invocation; a third of those were
    not visible before this.
    """
    heavy = [(w, c, n) for (w, c), n in sites.items() if n >= floor]
    gen = generated_sites(sites)
    if not heavy and not gen:
        return
    heavy.sort(key=lambda x: -x[2])
    total = sum(n for _, _, n in heavy)
    print(f"\n  {total} of those are at {len(heavy)} line(s) reported "
          f"{floor}+ times each,")
    print("  which is what a code-generating macro looks like - every")
    print("  finding inside the expansion carries the line that expanded")
    print("  it. Read them as one site, not as that many defects.")
    for w, c, n in heavy[:8]:
        path, _, ln = w.rpartition(":")
        mark = " [triaged]" if is_triaged(path, ln) else ""
        print(f"    {n:4d}  {w}  [{c}]{mark}")
    if gen:
        # sites is keyed (where, checker); one line can carry several
        # checkers, so sum the counts of every entry at a generated line.
        by_macro = collections.Counter()
        nfind = 0
        for (w, _c), n in sites.items():
            macro = gen.get(w)
            if macro:
                by_macro[macro] += n
                nfind += n
        print(f"\n  {nfind} finding(s) at {len(gen)} site(s) sit ON a")
        print("  macro-generator line, found by reading the source rather")
        print("  than by counting repeats - RB_GENERATE is one finding per")
        print("  file that instantiates a tree, so no threshold sees it.")
        for macro, n in by_macro.most_common(6):
            print(f"    {n:4d}  {macro}")


def agree(recs: list, an: list) -> None:
    """Where the two instruments land on the same line.

    This section used to be one sentence - "a finding in BOTH is much
    stronger than either" - and the intersection was never computed, so the
    claim was never tested. Computed, on a full sweep, it is nine lines out
    of 1,874 CBMC failures and 1,096 analyser findings, and reading them
    says something more useful than the sentence did:

      lib/libc/iconv/citrus_mapper.c:188 - both report a null call through
      cm->cm_ops->mo_uninit. Neither is right. mapper_open() validates all
      four operators before calling mo_init(), and mo_init() is the only
      thing that sets cm_closure, so `if (cm->cm_closure)` already implies
      the validation passed. Both instruments miss it for the SAME reason:
      the invariant lives across two functions and a struct field.

    So agreement is corroboration only when the two are failing
    independently. When they share a blind spot - an unconstrained pointer
    parameter, an invariant carried in a field - they agree and are both
    wrong. Six of the nine here are the pointer-precondition class that
    rule one already sets aside.

    Printed anyway, because a line both instruments dislike is worth
    thirty seconds, and because the honest version of the claim is more
    useful than the confident one.
    """
    cb: dict = collections.defaultdict(lambda: collections.defaultdict(list))
    for r in recs:
        if r.get("status") != "FAILED":
            continue
        for d in r.get("failures", []):
            m = re.match(r"line (\d+) (.*)", d.get("desc", ""))
            if m:
                cb[r["file"]][int(m.group(1))].append(
                    (r["function"], m.group(2)))

    ana: dict = collections.defaultdict(lambda: collections.defaultdict(list))
    for r in an:
        for f in r.get("findings", []):
            m = re.fullmatch(r"([^\s:]+):(\d+)", f.get("where", ""))
            if m:
                ana[m.group(1)][int(m.group(2))].append(
                    (f["checker"], f["msg"]))

    both = []
    for f in set(cb) & set(ana):
        for ln, ds in cb[f].items():
            if ln in ana[f]:
                both.append((f, ln, ds[0], ana[f][ln][0]))
    both.sort()
    print(f"\n== {len(both)} line(s) BOTH instruments flag")
    print("   Agreement is corroboration only where they fail independently.")
    print("   A shared blind spot - an unconstrained pointer parameter, an")
    print("   invariant held in a struct field - makes them agree and both")
    print("   be wrong. Worth thirty seconds each; not worth more on trust.")
    for f, ln, (fn, desc), (checker, msg) in both:
        print(f"  {f}:{ln}  ({fn})")
        print(f"      CBMC      {desc[:66]}")
        print(f"      analyser  [{checker}] {msg[:52]}")


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
    def _all_read(r):
        lns = [desc_line(d) for d in r.get("failures", [])[:2]]
        lns = [x for x in lns if x is not None]
        return bool(lns) and all(is_triaged(r["file"], x) for x in lns)

    seen_before = sum(1 for r in real if _all_read(r))
    print(f"\n== the {len(real)} worth a person's time"
          + (f", {seen_before} of them already read" if seen_before else ""))
    if seen_before:
        print("   [triaged] is in docs/security/UB_FINDINGS.md's not-a-defect")
        print("   table with the reasoning that killed it. Marked, not")
        print("   dropped - a triage that was wrong has to stay visible.")
    for r in sorted(real, key=lambda x: (x["file"], x["function"])):
        print(f"  {r['file']}:{r['function']}")
        for d in deciding_failures(r)[:2]:
            mark = (" [triaged]" if is_triaged(r["file"], desc_line(d))
                    else "")
            print(f"      {d['desc'][:100]}{mark}")

    if args.analyze:
        an = [json.loads(l) for l in Path(args.analyze).read_text().splitlines()
              if l.strip()]
        # analyze.py writes a leading _meta record naming the clang that
        # produced the run. Findings from different clangs are not
        # comparable - the analyser changes between releases - so the
        # version is printed rather than left for somebody to assume.
        metas = {r.get("analyzer") for r in an if r.get("_meta")}
        an = [r for r in an if not r.get("_meta")]
        finds = [(f, r) for r in an for f in r.get("findings", [])]
        by = collections.Counter(f["checker"] for f, _ in finds)
        sites = collections.Counter((f["where"], f["checker"]) for f, _ in finds)
        tby = collections.Counter(f["checker"] for f, r in finds
                                  if is_test_file(r["file"]))
        ntest = sum(tby.values())
        print(f"\n== clang --analyze: {len(finds)} finding(s) at "
              f"{len(sites)} distinct site(s), a DIFFERENT instrument")
        if metas:
            for m in sorted(x for x in metas if x):
                print(f"   {m}")
            if len(metas) > 1:
                print("   MORE THAN ONE ANALYSER produced this corpus; the")
                print("   totals are a sum across them, not one measurement.")
        else:
            print("   (no analyser version recorded - a sweep from before")
            print("    analyze.py started writing one. Not comparable to a")
            print("    run from a different machine.)")
        for c, n in by.most_common():
            intest = tby.get(c, 0)
            note = f"   ({intest} in test files)" if intest else ""
            print(f"  {n:4d}  {c}{note}")
        if ntest:
            print(f"\n  {ntest} of those are in test files rather than in the")
            print("  system. Not dropped - a test can have a real bug, and a")
            print("  finding nobody can see is indistinguishable from one")
            print("  that is not there - but 23% of unix.Malloc being")
            print("  lib/libc/tests changes what that number means.")
        print("\n  Path-sensitive and interprocedural where CBMC is")
        print("  exhaustive and modular, and approximate where CBMC is")
        print("  exact.")
        macro_report(sites)
        agree(recs, an)

    print("\n== before you fix anything")
    print("  Confirm it with UBSan first. Every entry in")
    print("  docs/security/UB_FINDINGS.md was, and three plausible")
    print("  findings died that way - FP_ILOGB0 is -INT_MAX and not")
    print("  INT_MIN, so -ilogb(x) cannot overflow.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
