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
                        s_significand via ilogb. Decided from CBMC's own
                        `no body for function X' and the expression
                        naming return_value_X, not from a list of names:
                        the list had cosl, sinl and tanl and not the six
                        softfloat comparison helpers whose overflowing
                        operand is spelled return_value___softfloat_*.
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


def extern_driven(rec: dict) -> bool:
    """Is every surviving failure on a value CBMC did not have a model for?

    EXTERN_DRIVEN above is a list of function NAMES, and a list of names
    falls behind by construction: it carried cosl, sinl and tanl and not
    cos, sin, cosf, sinf, tanf or the six softfloat comparison helpers,
    so thirteen records whose overflowing operand is literally spelled
    `return_value___softfloat_float64_le' were presented as the ones
    worth a person's time.

    cbmc_driver.py now records `no_body', the callees CBMC itself
    reported having no model for. A failure whose expression names one
    of those returns is the absence of a model, not a defect. The name
    list stays as the fallback for a record written before the driver
    recorded it - an older record simply does not carry the evidence, so
    it is judged the old way rather than judged wrongly.
    """
    no_body = rec.get("no_body")
    if not no_body:
        return rec["function"] in EXTERN_DRIVEN
    names = {"return_value_" + n for n in no_body}
    descs = [d.get("desc", "") for d in deciding_failures(rec)]
    if not descs:
        return rec["function"] in EXTERN_DRIVEN
    return all(any(n in d for n in names) for d in descs)



# The tree, for addr_taken() below. The report already reads
# UB_FINDINGS.md; this reads the sources that document is about.
_SRC = Path(__file__).resolve().parents[2] / "hbsd" / "src"
_ADDR_CACHE: dict = {}


def _decomment(txt: str) -> str:
    """Source with comments and string literals blanked.

    A function's own name appears in the comment above it more often
    than anywhere else, and `tone' and `rest' in sys/dev/speaker/spkr.c
    appear in English prose. Counting those as an address reference put
    eleven functions in the wrong bucket the first time this was
    measured: 69 before blanking, 55 after.
    """
    txt = re.sub(r"/\*.*?\*/", " ", txt, flags=re.S)
    txt = re.sub(r"//[^\n]*", " ", txt)
    return re.sub(r'"(\\.|[^"\\])*"', '""', txt)


def addr_taken(rec: dict) -> bool:
    """Is this static function's address stored somewhere?

    The `static' bucket defers on one claim: that the callers are all
    in the same file, so they narrow the domain CBMC explores. For a
    function whose address is taken that claim is false -- the callers
    are whoever holds the pointer. lib/libc/locale/utf8.c's
    _UTF8_mbrtowc is `l->__mbrtowc', reached from mbrtowc(3) with an
    application's bytes; lib/libc/posix1e/acl_support.c's
    _posix1e_acl_entry_compare is a qsort comparator;
    libexec/rtld-elf/rtld_lock.c's def_lock_create is a slot in a
    vtable a program may replace.

    Measured over run 33: 55 of the 254 records in this bucket, 22%,
    are one of these, and the bucket's stated reason does not hold for
    any of them.

    A name is an ADDRESS reference when what follows it is a value
    position -- a comma, semicolon, close bracket, assignment or end of
    line -- rather than the `(' of a call. If the file cannot be read
    the answer is False: absence of evidence is not evidence, and the
    old bucket is the conservative one.
    """
    key = (rec.get("file"), rec.get("function"))
    if key in _ADDR_CACHE:
        return _ADDR_CACHE[key]
    fn = rec.get("function")
    ans = False
    if fn:
        try:
            txt = _decomment((_SRC / rec["file"]).read_text(errors="replace"))
        except OSError:
            txt = None
        if txt is not None:
            ans = re.search(r"(?<![\w>.&])&?" + re.escape(fn) +
                            r"\s*[,;)}\]=\n]", txt) is not None
    _ADDR_CACHE[key] = ans
    return ans

# CBMC's own model of pipe(2). `__CPROVER_pipes[fildes].data[...]' is
# the library's array, not the tree's, and a descriptor it will not
# constrain is the same missing precondition as an unconstrained
# pointer -- 8 of the 36 index-shaped records in run 33 are this.
_CPROVER_ARRAY = re.compile(r"__CPROVER_\w+")

# CBMC names the array in an index check and does not in a pointer one:
#
#   array 'linux_errtbl' lower bound in linux_errtbl[(signed long int)error]
#   array.linp dynamic object upper bound in h->linp[(signed long int)nxt]
#
# against
#
#   dereference failure: pointer outside object bounds in s + len
#   pointer arithmetic: pointer outside object bounds in argv + 1l
#
# which is the distinction that matters. An unconstrained POINTER is
# modelled as pointing at a zero-size object, so any arithmetic on it
# is "outside object bounds" -- the same missing precondition as NULL,
# in a different spelling. An INDEX check fires on an object CBMC
# does have, with a subscript it does not.
_INDEX_BOUND = re.compile(r"array[ .'\w]* (?:upper|lower) bound")
_PTR_BOUND = "outside object bounds"
_NULLISH = ("pointer NULL", "pointer invalid", "deallocated dynamic object",
            "dead object", "invalid integer address", "pointer uninitialized")


def index_bound(rec: dict) -> bool:
    """Is an ARRAY SUBSCRIPT, not a pointer, what failed?

    Run 33 put 1,781 records in "pointer/memory (a missing precondition,
    not a bug)" and bucket()'s own comment admitted an array bound does
    not belong there. Splitting them:

        986  a pointer-shaped bound AND a null (the null explains it)
        542  a pointer-shaped bound only -- the same precondition
        206  region readable/writeable, leaks
         36  AN INDEX, on an object CBMC has
         11  null/invalid only

    Only the 36 are a different question, and reading them found
    libexec/talkd/print.c's three `> NTYPES' where the array wants
    `>=' -- a remote byte reaching one past a table of four.

    A record that also carries a null or a pointer-shaped bound is not
    one of them: there the unconstrained pointer explains the subscript
    too, and the answer is the precondition.
    """
    descs = [d.get("desc", "") for d in rec.get("failures", [])]
    if any(w in d for d in descs for w in _NULLISH):
        return False
    if any(_PTR_BOUND in d for d in descs):
        return False
    return any(_INDEX_BOUND.search(d) and not _CPROVER_ARRAY.search(d)
               for d in descs)


# `__pc' is the name every one of amd64's per-CPU accessors binds:
# get_pcpu(), __PCPU_PTR, __PCPU_GET, __PCPU_ADD and __PCPU_SET each
# open with `static struct pcpu __seg_gs *__pc = 0'. That 0 is the base
# of the per-CPU area in the %gs segment, not a null pointer -- but
# tools/verify/includes.py hands goto-cc `-D__seg_gs=', because its C
# parser does not know the qualifier and 245 translation units modelled
# nothing at all until it did. With the qualifier defined away CBMC
# sees a plain `struct pcpu *__pc = 0' and reports a null dereference
# at every PCPU_GET site.
#
# That is the trade docs/security/UB_FINDINGS.md states rather than
# buries, and this is its other half: the cost is bounded because the
# class is recognisable. `__pc' is a macro-local name, so a report that
# spells it is inside one of those five expansions and nowhere else.
_PCPU = re.compile(r"\b__pc->")
_AT_LINE = re.compile(r"^line (\d+) ")


def pcpu_artefact(rec: dict) -> bool:
    """Is every failing LINE one that reads the per-CPU segment base?

    Per line, not per failure, and the difference is a real record.
    sys/cddl/dev/dtrace/amd64/dtrace_subr.c's dtrace_gethrtime is
    `tsc_skew[curcpu]', and CBMC reports two things about line 346: the
    null `__pc', and an upper bound on tsc_skew. The second is
    downstream of the first - the subscript is unconstrained BECAUSE
    the value read out of the fake null pointer is - so a rule that
    asked every failure to name `__pc' would have missed it.

    Across lines that inference does not hold and is not made:
    kern_clock.c's statclock reports `td->td_proc' on line 696 and
    `__pc->pc_prvspace' on 698, and the first is a genuinely
    unconstrained parameter. It keeps the older answer. A failure whose
    desc carries no line number is its own group, so it cannot be
    carried by somebody else's.

    This only ever moves a record WITHIN the discard pile: everything
    it catches was already going to "a missing precondition, not a
    bug", and that label is the wrong reason for it. There is no
    precondition to add - `__pc' is not a pointer a caller was supposed
    to check. arc.c:arc_state_init, one of the 44 failures in the first
    full model check of OpenZFS, was triaged by hand; doing that again
    every sweep is how a known false positive turns back into a finding
    somebody reads.
    """
    fails = rec.get("failures", [])
    if not fails:
        return False
    lines, pcpu = set(), set()
    for d in fails:
        desc = d.get("desc", "")
        m = _AT_LINE.match(desc)
        at = m.group(1) if m else desc
        lines.add(at)
        if _PCPU.search(desc):
            pcpu.add(at)
    return lines == pcpu


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
        if pcpu_artefact(rec):
            return ("the __seg_gs pcpu artefact (the model's null, not "
                    "the code's)")
        if index_bound(rec):
            return "an INDEX out of its array's range - READ THESE"
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
        if addr_taken(rec):
            return "STATIC but its address is taken - READ THESE"
        return "static (callers constrain the domain - deferred)"
    if extern_driven(rec):
        return "extern-driven (an unmodelled return, unconstrained)"
    return "EXPORTED, arithmetic - READ THESE"


ROOT = Path(__file__).resolve().parents[2]
DOC = ROOT / "docs/security/UB_FINDINGS.md"
# A citation in the table: `path/to/file.c:123` or `file.c:12, 34`, and
# the CONTINUATION form the document uses when a row names several lines
# in one file -- `dis_tables.c:6419`, `:6447`. The path group is optional
# for exactly that: 64 bare `:NNN` across 31 of the table's rows had
# never matched anything, so a third of the rows were carrying lines the
# marker could not see. Those are findings somebody did read and write
# up, silently unmarked -- the harmless direction of the same failure the
# section boundary above was the harmful one of.
_TRIAGED = re.compile(
    r"`(?:([\w./-]+\.(?:c|cpp|h))|):(\d+(?:\s*,\s*\d+)*)`")


@functools.lru_cache(maxsize=1)
def triaged() -> frozenset:
    """(path suffix, line) pairs already read and found not to be defects.

    docs/security/UB_FINDINGS.md has a table of findings that looked
    like defects and were not, each with the reasoning that killed it.
    Re-listing them every sweep buries the ones nobody has read yet.

    They are MARKED and still printed, never dropped. A finding whose
    triage was wrong has to stay visible for that to be discoverable, and
    the whole point of writing the reasoning down was that the reasoning
    can be checked.

    Matching is on the path SUFFIX because that is what the document
    contains - `fread.c:129` for a libc path it names once and
    `sys/x86/isa/clock.c:200` for a kernel one it wants to disambiguate.

    ONLY THE TABLE. This used to read from the table's heading to the
    end of the file, on the premise -- which the docstring stated --
    that the table was the last section. It has not been for a long
    time: 19,500 lines of commit-by-commit writeup sit after it, every
    one of them citing `file.c:line` in backticks, and all of it was
    being read as triage. 1049 pairs, of which 148 were the table.

    The 901 were not a harmless superset. The citation that marked
    `citrus_mapper.c:188` as read is the sentence "Twelve more findings
    of the same checker are NOT COVERED HERE", and the others include
    defects that were found and fixed -- whose line numbers have since
    moved onto something else. A marker that says somebody read this
    when the document says nobody did is the one failure this whole
    document set exists to prevent, and it is the direction
    test_report_triage.py says matters.

    So the scan stops at the next `## ` heading. Prose is prose; a
    conclusion that belongs in the record goes in the table.
    """
    if not DOC.is_file():
        return frozenset()
    text = DOC.read_text(errors="replace")
    head = text.find("## Not defects, and why they looked like defects")
    if head < 0:
        return frozenset()
    end = text.find("\n## ", head + 1)
    table = text[head:] if end < 0 else text[head:end]
    out = set()
    for row in table.split("\n"):
        # Row by row, so a bare `:NNN` attaches to the path named to its
        # LEFT IN THE SAME ROW and can never reach across into another.
        if not row.lstrip().startswith("|"):
            continue
        here = None
        for m in _TRIAGED.finditer(row):
            name, lines = m.group(1), m.group(2)
            if name:
                here = name
            elif here is None:
                continue        # a continuation with nothing to continue
            for ln in lines.split(","):
                out.add((here, ln.strip()))
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
    ap.add_argument("--expect", action="append", default=[],
                    help="basename of a model-check shard this report "
                         "SHOULD have been given (repeatable). One that is "
                         "not among `results' is named as missing rather "
                         "than silently left out of the totals.")
    args = ap.parse_args()

    recs = []
    per_input = []
    for p in args.results:
        n = len(recs)
        for line in Path(p).read_text().splitlines():
            if line.strip():
                recs.append(json.loads(line))
        per_input.append((Path(p).name, len(recs) - n))

    # A shard whose job died contributes no verdicts, and every number
    # below is then over what remains. Five runs, 27 through 31, each
    # lost the whole userland model check to an OOMed runner and every
    # one still printed a report that read like a complete one: 1460
    # checked, no lib/libc anywhere, and nothing on the page to say a
    # third of the corpus was absent.
    # The absence of a measurement is not a clean result, and a report
    # that cannot tell the two apart is the same failure this repository
    # keeps finding in the code it reads.
    print("== the model-check shards this report is built from")
    for name, n in sorted(per_input):
        print(f"  {n:6d}  {name}")
    missing = [e for e in args.expect
               if e not in {name for name, _ in per_input}]
    if missing:
        print(f"\n  MISSING, {len(missing)} of "
              f"{len(args.expect)} expected: {', '.join(sorted(missing))}")
        print("  Those shards produced no verdicts at all. Every count")
        print("  below is over the shards that are here, and says NOTHING")
        print("  about the scopes that are not - not that they are clean.")
    elif args.expect:
        print(f"\n  All {len(args.expect)} expected shards are present.")

    status = collections.Counter(r["status"] for r in recs)
    print("\n== what the model checker concluded")
    order = ["PROVED", "PROVED-ASSUMING", "BOUNDED", "FAILED",
             "TIMEOUT", "ERROR", "NOFUNC"]
    for k in order + [k for k in status if k not in order]:
        if status.get(k):
            print(f"  {k:16s} {status[k]}")
    print(f"  {'':16s} {sum(status.values())} checked")
    # An ERROR is not a clean result and not a finding: it is CBMC
    # declining to answer. 60 of the 918 functions in one userland run
    # are CBMC 5.95.1 aborting on its own invariant, mostly on _Complex
    # arithmetic in lib/msun, which means those functions have never
    # been checked at all - a state indistinguishable, in a count, from
    # one that came back clean. So the reasons are named.
    errs = [r for r in recs if r["status"] in ("ERROR", "NOFUNC")]
    if errs:
        why = collections.Counter()
        for r in errs:
            d = (r.get("detail") or "").strip()
            lines = [x.strip() for x in d.splitlines()[:4] if x.strip()]
            # Only a line the driver put there on purpose counts. A
            # record written before it did carries an arbitrary
            # 600-character window, and the fragment at the top of that
            # window is not a reason - calling it one would be the same
            # mistake in smaller print.
            # "Reason:" is the line that says what CBMC could not do;
            # "Invariant check failed" above it only says that it could
            # not, so prefer the one that carries information.
            first = next((x for x in lines if x.startswith("Reason:")), "")
            if not first:
                first = next(
                    (x for x in lines
                     if x.startswith(("Invariant check failed",
                                      "CONVERSION ERROR"))
                     or "error:" in x.lower()),
                    "")
            # The single most common thing CBMC says in this tree, and
            # it carries no "Reason:". Run 33: 193 "SAT checker ran out
            # of memory" and 27 "Out of memory" out of 424 ERRORs --
            # the --mem-mb bound converting a runner kill into one
            # function's non-answer. Counting those as "no reason
            # recorded" says we do not know when we do.
            #
            # Searched over the WHOLE detail rather than its first four
            # lines, unlike the tests above. Those look for a line the
            # driver hoisted on purpose; this one is a fact about the
            # record whoever wrote it, and every record from before
            # cbmc_driver.py learned the phrase still carries it
            # somewhere in the 600-character window. Retroactive by
            # construction, which is the point: run 33's artifact is
            # what proved the number.
            if not first:
                first = next(
                    (x.strip() for x in d.splitlines()
                     if "out of memory" in x.lower()),
                    "")
            # And the second most common, for the same reason and in
            # the same retroactive way. cbmc_driver.py now hoists this
            # line, but every record written before it did still
            # carries the sentence somewhere in its window:
            #
            #   too many addressed objects: maximum number of objects
            #   is set to 2^n=256 (with n=8); use the `--object-bits n'
            #   option to increase the maximum number
            #
            # Run 33: 135 of 424, 32%, and unlike the memory bound this
            # one names a knob. Counting it as "no reason recorded"
            # says we do not know when we do, and hides the fact that a
            # third of the non-answers have a remedy.
            if not first:
                first = next(
                    (x.strip() for x in d.splitlines()
                     if "too many addressed objects" in x.lower()),
                    "")
            if not first:
                first = "(CBMC printed no reason)"
            why[first[:72]] += 1
        print(f"\n  what the {len(errs)} ERROR/NOFUNC records say:")
        for k, n in why.most_common(6):
            print(f"    {n:4d}  {k}")
        rest = len(errs) - sum(n for _, n in why.most_common(6))
        if rest:
            print(f"    {rest:4d}  (the rest)")
        print("  An ERROR is CBMC declining to answer. Those functions")
        print("  are UNCHECKED, which a count cannot tell from clean.")

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
