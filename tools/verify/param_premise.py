#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Which of the sweep's findings rest on a parameter nobody can constrain.

tools/verify/report.py has had this rule since the CBMC runs:

    static   the signature domain is the wrong domain: the only callers
             are in the same file and they constrain it. Deferred, not
             dismissed.

The clang sweep has never had it, and the sweep is where the findings
now are: 1,569 of sweep 15's 1,638 are not written up in
docs/security/UB_FINDINGS.md, which is more than a person reads one at a
time. Sorting them by whose precondition they are is how the pile
becomes readable.

A finding is sorted by the function it is inside and by who can call it:

  EXPORTED        the callers are in other translation units. The
                  analyser cannot see them and neither can this. The
                  finding is a question about the function's contract.
  STATIC, CALLED  every caller is in this file. The analyser inlined
                  them AND analysed the function again from its own
                  entry with nothing constrained, and this report is
                  from the second pass. Reading the call sites settles
                  it.
  STATIC, TAKEN   no call site, but the name appears elsewhere in the
                  file - a DEVMETHOD table, a kobj method, a sysctl
                  handler, a callout. It is called through a pointer by
                  code that is not here, so it is exported in every way
                  that matters to a precondition.
  STATIC, UNSEEN  the name appears nowhere but its own definition.
  MACRO           the analyser named a function the source does not
                  contain - RB_GENERATE_STATIC and its family. Recorded
                  since analyze.py started asking clang for the name; a
                  record older than that lands in "attributed to no
                  function" instead.
  BRACE-ON-DECLARATOR
                  a definition that puts its `{' on the declarator line.
                  Not style(9), and not a macro either: BIND's resolver
                  under lib/libc/resolv and openzfs's Lua both do it.
  GENERATED       config(8)'s vnode_if.h and the other *_if.h interfaces,
                  written into a directory that does not outlive the run.
  HEADER          the finding is in a .h. A `static __inline' there has no
                  caller in its own file and a caller in every unit that
                  includes it, and the analyser reports it once per unit -
                  sys/sys/refcount.h:69 and sys/vm/vm_page.h:959 are two.

and by whether the finding NAMES a parameter of that function:

  named    the analyser's message says `loaded from variable 'x'' or
           `pointed to by 'x'', and x is a parameter. Exact.
  online   the message names no variable, and a parameter appears on the
           finding's own source line. Suggestive; it is a different
           column and is never added to the first.

The function extents come from style(9): a definition's opening brace is
alone at column 0. tools/verify/test_param_premise.py checks that rule
against clang's own issue_context on a sample, because a rule about where
functions start that is a little bit wrong produces a table that is
entirely wrong and looks fine.

Where the rule cannot answer it says so rather than guessing, and over
sixty files sampled against the analyser every disagreement was of that
kind - never a wrong function. Two things it declines: code a macro
generated at file scope (`RB_GENERATE_STATIC', `NSS_MP_CACHE_HANDLING'),
and a definition that puts its brace on the declarator line, which is
every non-BSD file in the tree - openzfs's Lua is the example. 52 of
sweep 15's 1,554 come out unattributed, five of them in sys/contrib, and
a rule for the one-line brace was considered and left out: it would
recover a handful and its failure mode is silent misattribution, which
is the thing this whole file is arranged not to do.
"""
from __future__ import annotations

import argparse
import collections
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import report  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd/src"

# The message forms that name an OBJECT. `The left operand of '!=' is a
# garbage value' also has a quoted string in it and it is an operator, and
# `Access to field 'foo'' names a field of the object rather than the
# object, so the patterns are listed rather than a general quote scrape.
NAMES_OBJECT = (
    re.compile(r"loaded from variable '(\w+)'"),
    re.compile(r"pointed to by '(\w+)'"),
    re.compile(r"Array access \(from variable '(\w+)'\)"),
)

IDENT = re.compile(r"\b[A-Za-z_]\w*\b")


def definitions(lines: list[str]) -> list[tuple[int, int, int]]:
    """[(brace, head, end)] 0-based, for every column-0 brace on its own line.

    `head` is the first line of the declarator above it: walk up while the
    lines are non-blank and start neither a closing brace nor a directive.
    `end` is the column-0 `}' that closes it.

    The end matters as much as the start. A finding can sit at file scope
    BETWEEN two functions, inside a macro that expands to code -
    tcp_log_buf.c:357 is `RB_GENERATE_STATIC(tcp_log_id_tree, ...)' - and
    without the bound the nearest preceding function swallows it. Eight of
    the first thirty files sampled against clang disagreed, and all eight
    were this.
    """
    # A `#define' body can put a `}' at column 0 INSIDE a function -
    # lib/libc/stdio/vfprintf.c:375-382 defines PRINTANDPAD and FLUSH that
    # way, in the middle of __vfprintf - and the first version of this took
    # that for the end of the function and dropped every finding after it.
    cpp = set()
    cont = False
    for i, ln in enumerate(lines):
        if cont or ln.lstrip()[:1] == "#":
            cpp.add(i)
            cont = ln.rstrip().endswith("\\")
        else:
            cont = False

    out = []
    for i, ln in enumerate(lines):
        if i in cpp:
            continue
        if ln[:1] == "{" and ln.strip() == "{":
            j = i - 1
            while (j >= 0 and lines[j].strip()
                   and not lines[j].startswith(("}", "#"))):
                j -= 1
            end = len(lines) - 1
            for k in range(i + 1, len(lines)):
                if lines[k][:1] == "}" and k not in cpp:
                    end = k
                    break
            out.append((i, j + 1, end))
    return out


def parameters(sig: str) -> tuple[str, tuple[str, ...]]:
    """(function name, parameter names) from a joined declarator."""
    i = sig.find("(")
    if i < 0:
        return "", ()
    name = ""
    m = list(IDENT.finditer(sig[:i]))
    if m:
        name = m[-1].group(0)
    # The matching close paren, so a parameter that is itself a function
    # pointer does not truncate the list.
    depth, j = 0, i
    for j in range(i, len(sig)):
        if sig[j] == "(":
            depth += 1
        elif sig[j] == ")":
            depth -= 1
            if depth == 0:
                break
    params = []
    for part in _split(sig[i + 1:j]):
        part = part.strip()
        if not part or part == "void":
            continue
        ids = IDENT.findall(part)
        # struct foo *bar -> bar; int (*fn)(void) -> fn; unnamed -> nothing
        if part.endswith(("*", "]")) or len(ids) < 2:
            if "(*" in part and len(ids) >= 2:
                params.append(re.search(r"\(\s*\*\s*(\w+)", part).group(1))
            continue
        if "(*" in part:
            m2 = re.search(r"\(\s*\*\s*(\w+)", part)
            if m2:
                params.append(m2.group(1))
            continue
        params.append(ids[-1])
    return name, tuple(params)


def _split(s: str) -> list[str]:
    """Split on commas that are not inside parentheses or brackets."""
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    out.append(cur)
    return out


class File:
    def __init__(self, path: Path):
        self.lines = path.read_text(errors="replace").splitlines()
        self.defs = definitions(self.lines)
        self._uses: dict[str, int] | None = None

    def one_line_brace(self, name: str) -> bool:
        """Is `name' defined with its brace on the declarator line?

        style(9) does not do this and most of the tree does not either,
        but BIND's resolver under lib/libc/resolv and openzfs's Lua both
        write `free_nsrr(rrset_ns *nsrrsp, rr_ns *nsrr) {'. Worth telling
        apart from a name a macro generated, because one is a function a
        reader can open and the other is not.
        """
        pat = re.compile(r"^\s*(?:\w[\w \t*]*\s)?\**" + re.escape(name)
                         + r"\s*\(")
        for i, ln in enumerate(self.lines):
            if not pat.match(ln):
                continue
            j, depth = i, 0
            while j < len(self.lines) and j < i + 8:
                depth += self.lines[j].count("(") - self.lines[j].count(")")
                if depth <= 0 and self.lines[j].rstrip().endswith("{"):
                    return True
                if depth <= 0:
                    break
                j += 1
        return False

    def enclosing(self, line: int):
        """(name, params, is_static, brace, head) around a 1-based line.

        None when the line is at file scope - between two definitions, or
        before the first.
        """
        best = None
        for brace, head, end in self.defs:
            if brace <= line - 1 <= end:
                best = (brace, head)
                break
            if brace > line - 1:
                break
        if best is None:
            return None
        brace, head = best
        sig = " ".join(self.lines[head:brace])
        # The walk up stops at a blank line, and style(9) puts the comment
        # block immediately above the declarator with no blank line between,
        # so sig routinely BEGINS with `/*'. Anchoring `static' to the start
        # of it called two thirds of this tree's static functions exported.
        sig = re.sub(r"/\*.*?\*/", " ", sig)
        name, params = parameters(sig)
        static = re.match(r"\s*static\b", sig) is not None
        return name, params, static, brace, head

    def uses(self, name: str) -> tuple[int, int]:
        """(call sites, other mentions) of `name`, outside its definition.

        A mention that is not a call is the address being taken - a
        DEVMETHOD entry, a kobj method table, a sysctl handler, a callout.
        """
        calls = others = 0
        own = set()
        for brace, head, _end in self.defs:
            own.update(range(head, brace + 1))
        decl = re.compile(r"\b" + re.escape(name) + r"\b")
        for i, ln in enumerate(self.lines):
            if i in own:
                continue
            for m in decl.finditer(ln):
                if not ln[m.end():].lstrip().startswith("("):
                    others += 1        # the address, not the call
                    continue
                # A prototype is `[static] TYPE name(...);' - an identifier
                # sits immediately before the name, and a semicolon follows
                # the argument list. A call has an operator, a comma or
                # nothing before it: `error = foo(x);', `return (foo(x));'.
                # The semicolon is looked for after the CLOSING paren rather
                # than at the end of the line, because a prototype in this
                # tree routinely carries a trailing comment after it.
                before = ln[:m.start()]
                if not re.search(r"[A-Za-z_]\w*[\s*]+$", before):
                    calls += 1
                    continue
                k = ln.index("(", m.end() - 1)
                depth = 0
                for k in range(k, len(ln)):
                    if ln[k] == "(":
                        depth += 1
                    elif ln[k] == ")":
                        depth -= 1
                        if depth == 0:
                            break
                if ln[k + 1:].lstrip()[:1] == ";":
                    continue
                calls += 1
        return calls, others


def named_objects(msg: str) -> set[str]:
    out: set[str] = set()
    for pat in NAMES_OBJECT:
        out.update(pat.findall(msg))
    return out


GENERATED = re.compile(r"^/tmp/pbsd_\w+_[A-Za-z0-9_]+/")


def collect(shards) -> dict:
    """{(file, line, checker): (message, function or None)} over a sweep.

    The function is what the analyser itself said - analyze.py records
    clang's issue_context since the plist-multi-file switch - and is
    absent from a record older than that.

    Keyed by the PLACE, so a finding in a generated header that every unit
    including it reports is counted once. sweep_report.py keys by the
    translation unit as well, which is what its per-sweep totals mean, so
    the two counts are not the same number and are not meant to be.

    The generated header's directory is a fresh mkdtemp every run, and is
    normalised the same way sweep_report.py normalises it - otherwise the
    same seven findings have a different name in every sweep.
    """
    seen: dict[tuple, tuple[str, str | None]] = {}
    for sh in shards:
        for ln in open(sh):
            rec = json.loads(ln)
            if rec.get("_meta"):
                continue
            for f in rec.get("findings", []):
                path, _, line = f["where"].rpartition(":")
                path = GENERATED.sub("<generated>/", path)
                seen[(path, int(line), f["checker"])] = (f["msg"], f.get("fn"))
    return seen


def audit(seen: dict) -> int:
    """Citations in UB_FINDINGS.md that no finding in this sweep matches.

    A citation is `file.c:601` and report.is_triaged() reads it by
    basename, so this reads it the same way. Not every one is a mistake:
    the document cites call sites, guards and macro definitions in prose
    too, and a finding that has been FIXED is cited by a section that
    describes fixing it. What it catches is the citation written from the
    prose rather than from the sweep - six lines of ext2_lookup.c were
    cited at the `dd_ino != NULL` returns the paragraph names instead of
    at the reports, which counts as triaged and reads as read.
    """
    doc = ROOT / "docs/security/UB_FINDINGS.md"
    if not doc.is_file():
        print("no docs/security/UB_FINDINGS.md here")
        return 0
    have: dict[str, set[int]] = collections.defaultdict(set)
    for path, line, _ in seen:
        have[path.rsplit("/", 1)[-1]].add(line)
    cited: dict[str, set[int]] = collections.defaultdict(set)
    for name, lines in report._TRIAGED.findall(doc.read_text()):
        base = name.rsplit("/", 1)[-1]
        for n in lines.split(","):
            cited[base].add(int(n.strip()))
    miss = {b: sorted(ls - have.get(b, set()))
            for b, ls in cited.items() if ls - have.get(b, set())}
    total = sum(len(v) for v in cited.values())
    n = sum(len(v) for v in miss.values())
    print(f"{total:,} citations in {len(cited)} files; "
          f"{n} match no finding in this sweep")
    for b in sorted(miss, key=lambda x: -len(miss[x])):
        print(f"  {len(miss[b]):3d}  {b}: "
              + ", ".join(str(x) for x in miss[b][:12])
              + (" ..." if len(miss[b]) > 12 else ""))
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("shards", nargs="+", help="a sweep's .jsonl files")
    ap.add_argument("--all", action="store_true",
                    help="include findings already written up")
    ap.add_argument("--audit", action="store_true",
                    help="list the citations in UB_FINDINGS.md that match "
                         "no finding in this sweep")
    ap.add_argument("--list", default="",
                    help="print every finding in this class, e.g. "
                         "'static-called' or 'exported'")
    args = ap.parse_args()

    seen = collect(args.shards)
    if args.audit:
        return audit(seen)
    keys = [k for k in seen
            if args.all or not report.is_triaged(k[0], str(k[1]))]
    # report.is_triaged() wants the finding cited as `file.c:601'. The
    # document also writes `:601' inside a section whose heading already
    # named the file - the ufs_lookup.c and ext2_lookup.c section is twelve
    # findings written up that way - so "not cited" overstates "not read".
    # Naming the file is a weaker signal than citing the line and is
    # reported as its own number, never folded into the first.
    discussed = 0
    if not args.all:
        doc = (ROOT / "docs/security/UB_FINDINGS.md")
        base = set()
        if doc.is_file():
            base = {n.rsplit("/", 1)[-1] for n in
                    re.findall(r"[\w./-]+\.(?:c|h|cpp)", doc.read_text())}
        discussed = sum(1 for k in keys if k[0].rsplit("/", 1)[-1] in base)
    byfile: dict[str, list] = collections.defaultdict(list)
    for k in keys:
        byfile[k[0]].append(k)

    tally: dict[str, list[int]] = collections.defaultdict(
        lambda: [0, 0, 0])   # findings, names a parameter, parameter on line
    rows: dict[str, list[str]] = collections.defaultdict(list)
    unattributed = 0
    unreadable: set[str] = set()

    for path in sorted(byfile):
        if path.startswith("<generated>/"):
            # config(8)'s vnode_if.h and the *_if.h interfaces. The file is
            # written into a temporary directory that does not outlive the
            # run, so there is nothing here to read the function out of.
            tally["generated"][0] += len(byfile[path])
            continue
        p = SRC / path
        if not p.is_file():
            unreadable.add(path)
            unattributed += len(byfile[path])
            continue
        f = File(p)
        for k in sorted(byfile[path]):
            _, line, checker = k
            msg, said = seen[k]
            e = f.enclosing(line)
            if e is None or not e[0]:
                if said:
                    # At file scope as far as the text goes, and clang has
                    # a name for it: either a macro generated the function
                    # (RB_GENERATE_STATIC and friends) or the file writes
                    # its brace on the declarator line, which is not
                    # style(9) but is what BIND's resolver and Lua do.
                    cls = ("brace-on-declarator" if f.one_line_brace(said)
                           else "macro")
                    tally[cls][0] += 1
                    if args.list == cls:
                        rows[cls].append(
                            f"  {path}:{line}  {said}()"
                            f"\n      {checker}: {msg}")
                else:
                    unattributed += 1
                continue
            name, params, static, brace, _head = e
            if said and said != name:
                # clang named a different function: the brace rule found
                # the definition this line is textually inside, and the
                # analyser is reporting code a macro generated there. Its
                # answer wins, and it has no parameters to read.
                tally["macro"][0] += 1
                if args.list == "macro":
                    rows["macro"].append(
                        f"  {path}:{line}  {said}()  [textually in "
                        f"{name}()]\n      {checker}: {msg}")
                continue
            if path.endswith((".h", ".hh", ".hpp")):
                # A `static __inline' in a header has no callers in its own
                # file and every including translation unit's callers
                # elsewhere. sys/sys/refcount.h:69, sys/vm/vm_page.h:959,
                # sys/sys/time.h:87 - the analyser starts at the inline
                # function with its parameters unconstrained, in every unit
                # that includes it, and reports the same thing each time.
                cls = "header"
            elif not static:
                cls = "exported"
            else:
                calls, others = f.uses(name)
                cls = ("static-called" if calls else
                       "static-taken" if others else "static-unseen")
            t = tally[cls]
            t[0] += 1
            named = named_objects(msg) & set(params)
            if named:
                t[1] += 1
            elif params and line - 1 < len(f.lines) and set(
                    IDENT.findall(f.lines[line - 1])) & set(params):
                t[2] += 1
            if args.list == cls:
                rows[cls].append(
                    f"  {path}:{line}  {name}()"
                    f"{'  [' + ', '.join(sorted(named)) + ']' if named else ''}"
                    f"\n      {checker}: {msg}")

    order = ["exported", "static-taken", "static-called",
             "static-unseen", "header", "generated", "macro",
             "brace-on-declarator"]
    print(f"{len(keys):,} findings"
          f"{'' if args.all else ' not cited by line in UB_FINDINGS.md'}"
          f" in {len(byfile):,} files")
    if discussed:
        print(f"  ({discussed:,} of them are in a file the document does "
              f"discuss, which is a weaker claim than a citation)")
    print()
    print(f"  {'':<20}{'findings':>9}{'names a param':>15}"
          f"{'param on the line':>19}")
    for cls in order:
        if cls not in tally:
            continue
        n, named, online = tally[cls]
        print(f"  {cls:<20}{n:>9,}{named:>15,}{online:>19,}")
    tot = [sum(tally[c][i] for c in tally) for i in range(3)]
    print(f"  {'':-<20}{'':->9}{'':->15}{'':->19}")
    print(f"  {'total':<20}{tot[0]:>9,}{tot[1]:>15,}{tot[2]:>19,}")
    if unattributed:
        print(f"\n  {unattributed} finding(s) attributed to no function"
              + (f", {len(unreadable)} of them in a source that is not here"
                 if unreadable else ""))
        for u in sorted(unreadable)[:6]:
            print(f"      {u}")
    for line in rows.get(args.list, []):
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
