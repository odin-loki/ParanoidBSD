#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Every function that exists, and how we know it exists.

"No function left untouched" is a claim about a SET, and the set has to be
built before it can be checked. The obvious failure is circular: enumerate
functions by asking the instruments which functions they saw, then report
that the instruments saw every function they saw. That number is always
100% and always meaningless.

So the universe is built from sources that do not depend on the
instruments at all, and every row carries WHICH source found it, because
the sources are not equally trustworthy:

    ledger    docs/port_plan.json, built by tools/port_plan.py from the
              tree itself. 297,212 (file, function) pairs across
              hbsd/src. This is the good source.
    textual   a brace-matching scan, in this file. It is a parser only
              in the loosest sense, it has a measured error rate (see
              --selftest), and it is used ONLY for trees the ledger does
              not cover - which today means kde/ and pbsd/'s own code.

The textual scanner's accuracy is not asserted. `--selftest` runs it over
files the ledger already knows and prints precision and recall against
the ledger's answer, over a stated sample. If those numbers are bad, the
KDE row count is bad by the same amount, and a reader should be able to
find that out in one command rather than by trusting a sentence.

What this deliberately does NOT do
----------------------------------
It does not try to decide whether a function MATTERS. A one-line static
accessor and an on-disk filesystem parser are both one row. Weighting by
risk is a real thing to want and it is not this - see confidence.py's
closing note on the same point.
"""
from __future__ import annotations

import argparse
import json
import random
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LEDGER = ROOT / "docs" / "port_plan.json"

# Where each tree lives, and what the ledger's paths are relative to.
TREES = {
    "hbsd": ROOT / "hbsd" / "src",
    "kde": ROOT / "kde",
    "pbsd": ROOT / "pbsd",
}

C_EXT = {".c"}
CXX_EXT = {".cpp", ".cc", ".cxx", ".C"}
SRC_EXT = C_EXT | CXX_EXT

# ------------------------------------------------------------------ scanner
#
# Not a parser. It finds a `)' that is followed by a `{' at the top level
# of a file, with a plausible identifier in front of the matching `(', and
# it is wrong in both directions. The keyword list is what stops `if (x) {'
# from being counted as a function called `if'.

_NOT_A_NAME = {
    "if", "for", "while", "switch", "catch", "return", "sizeof", "do",
    "else", "case", "default", "and", "or", "not", "typeof", "__typeof",
    "alignof", "_Alignof", "static_assert", "_Static_assert", "decltype",
    "throw", "new", "delete", "operator", "template", "typename",
    "constexpr", "noexcept", "explicit", "friend", "using", "namespace",
    "struct", "union", "enum", "class", "public", "private", "protected",
    "__attribute__", "__asm", "asm", "__asm__", "defined", "va_arg",
    "static_cast", "dynamic_cast", "const_cast", "reinterpret_cast",
}

# An identifier, possibly qualified (C++ Class::method, Ns::Class::method),
# possibly with a destructor tilde, immediately before an open paren.
_NAME_BEFORE_PAREN = re.compile(
    r"(~?[A-Za-z_][A-Za-z_0-9]*(?:\s*::\s*~?[A-Za-z_][A-Za-z_0-9]*)*)\s*$")

# Trailing decoration between `)' and `{' that a definition may carry.
# Matched as a PREFIX of what follows the close paren, never anchored to
# the end of the slice: `int f() const { ... }' has a body after the
# decoration, so a `$' here means `const' is never recognised and every
# const method in the tree goes missing.
_TRAILER = re.compile(
    r"(?:\s*(?:const\b|noexcept\b|override\b|final\b|volatile\b|"
    r"mutable\b|&&|&|throw\s*\([^)]*\)|"
    r"__attribute__\s*\(\(.*?\)\)|__THROW\b|_Noreturn\b|"
    r"->\s*[A-Za-z_][\w:<>,\s*&]*))*")


def _strip(text: str) -> str:
    """Blank out comments, string and character literals, in place.

    Length is preserved so offsets stay valid; the content is replaced by
    spaces so a brace inside a string cannot move the depth counter.
    """
    out = list(text)
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            while i < n and text[i] != "\n":
                out[i] = " "
                i += 1
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            out[i] = out[i + 1] = " "
            i += 2
            while i + 1 < n and not (text[i] == "*" and text[i + 1] == "/"):
                if text[i] != "\n":
                    out[i] = " "
                i += 1
            if i + 1 < n:
                out[i] = out[i + 1] = " "
                i += 2
        elif c in "\"'":
            q = c
            out[i] = " "
            i += 1
            while i < n and text[i] != q:
                if text[i] == "\\":
                    out[i] = " "
                    i += 1
                    if i < n and text[i] != "\n":
                        out[i] = " "
                        i += 1
                    continue
                if text[i] != "\n":
                    out[i] = " "
                i += 1
            if i < n:
                out[i] = " "
                i += 1
        else:
            i += 1
    return "".join(out)


def scan(text: str) -> list[str]:
    """The function names this file appears to DEFINE, in order.

    A definition is a balanced `(...)' whose next non-decoration character
    is `{', found where the brace depth is zero (a free function) or
    inside a class or namespace body (a method). Bodies are skipped, so a
    call inside a function is never mistaken for a definition.
    """
    src = _strip(text)
    names: list[str] = []
    depth = 0
    i, n = 0, len(src)
    # `pending' holds a class/namespace prefix stack so an in-class method
    # comes out as Class::method and can be told apart from a free
    # function of the same name.
    scope: list[str] = []
    scope_depth: list[int] = []

    while i < n:
        c = src[i]
        if c == "{":
            depth += 1
            i += 1
            continue
        if c == "}":
            depth -= 1
            while scope_depth and scope_depth[-1] > depth:
                scope.pop()
                scope_depth.pop()
            i += 1
            continue
        if c == ";":
            i += 1
            continue
        if c == "(":
            # Find the matching close paren.
            d, j = 1, i + 1
            while j < n and d:
                if src[j] == "(":
                    d += 1
                elif src[j] == ")":
                    d -= 1
                j += 1
            if d:
                break
            close = j
            # A definition: optional decoration, an optional C++ member
            # initialiser list, then `{'.
            k = close
            while k < n and src[k] in " \t\r\n":
                k += 1
            if k < n and src[k] == ":" and not (k + 1 < n and src[k+1] == ":"):
                # ctor init list - walk to the brace at this paren depth
                d2 = 0
                while k < n:
                    if src[k] == "(":
                        d2 += 1
                    elif src[k] == ")":
                        d2 -= 1
                    elif src[k] == "{" and d2 == 0:
                        break
                    k += 1
            else:
                k = close
                m = _TRAILER.match(src[close:close + 200])
                if m:
                    k = close + m.end()
                while k < n and src[k] in " \t\r\n":
                    k += 1
            if k < n and src[k] == "{":
                head = src[max(0, i - 400):i]
                nm = _NAME_BEFORE_PAREN.search(head)
                if nm:
                    name = re.sub(r"\s*", "", nm.group(1))
                    base = name.split("::")[-1]
                    if base not in _NOT_A_NAME and name not in _NOT_A_NAME:
                        qual = "::".join(scope + [name]) if scope else name
                        names.append(qual)
                # Skip the whole body: it contains no definitions we want.
                d3, j2 = 0, k
                while j2 < n:
                    if src[j2] == "{":
                        d3 += 1
                    elif src[j2] == "}":
                        d3 -= 1
                        if d3 == 0:
                            break
                    j2 += 1
                i = j2 + 1
                continue
            i = close
            continue
        # A class/struct/namespace opening a scope.
        m = re.match(r"\b(class|struct|namespace)\s+([A-Za-z_]\w*)"
                     r"(?:\s*:\s*[^{;]*)?\s*\{", src[i:i + 300])
        if m and (i == 0 or not (src[i - 1].isalnum() or src[i - 1] == "_")):
            scope.append(m.group(2))
            scope_depth.append(depth + 1)
            depth += 1
            i += m.end()
            continue
        i += 1
    return names


def scan_file(path: Path) -> list[str]:
    try:
        return scan(path.read_text(errors="replace"))
    except OSError:
        return []


# ------------------------------------------------------------------ universe

def ledger_rows(ledger: Path):
    """(tree, path, function, source) from the port ledger."""
    data = json.loads(ledger.read_text())
    for rec in data["records"]:
        for fn in rec.get("functions") or ():
            yield ("hbsd", rec["path"], fn, "ledger")


def textual_rows(tree: str, root: Path, skip: set[str] | None = None):
    """(tree, path, function, source) from the scanner, for a whole tree."""
    for p in sorted(root.rglob("*")):
        if p.suffix not in SRC_EXT or not p.is_file():
            continue
        rel = str(p.relative_to(root))
        if skip and rel in skip:
            continue
        for fn in scan_file(p):
            yield (tree, rel, fn, "textual")


def build(out: Path, trees: list[str], ledger: Path) -> dict:
    counts = {"ledger": 0, "textual": 0}
    seen = set()
    with out.open("w") as fh:
        fh.write(json.dumps({
            "_meta": True, "v": 1, "trees": trees,
            "ledger": str(ledger.relative_to(ROOT)),
        }) + "\n")
        if "hbsd" in trees:
            # The ledger covers hbsd/src; do not scan it textually as well
            # or the weaker source would silently add rows the good one
            # rejected.
            for row in ledger_rows(ledger):
                key = row[:3]
                if key in seen:
                    continue
                seen.add(key)
                counts["ledger"] += 1
                fh.write(json.dumps({"tree": row[0], "file": row[1],
                                     "function": row[2], "src": row[3]}) + "\n")
        for tree in trees:
            if tree == "hbsd":
                continue
            root = TREES.get(tree)
            if root is None or not root.is_dir():
                print(f"  tree {tree}: NOT PRESENT at {root}", file=sys.stderr)
                continue
            for row in textual_rows(tree, root):
                key = row[:3]
                if key in seen:
                    continue
                seen.add(key)
                counts["textual"] += 1
                fh.write(json.dumps({"tree": row[0], "file": row[1],
                                     "function": row[2], "src": row[3]}) + "\n")
    return counts


# ------------------------------------------------------------------ selftest

def selftest(sample: int, seed: int) -> int:
    """Precision and recall of the scanner, against the ledger's answer.

    The ledger is not ground truth either - it is built by a different
    method over the same files - but it is INDEPENDENT of the scanner,
    which is the property that matters. Disagreement is reported both
    ways and a handful of each is printed, because the interesting
    question is not the percentage, it is what the two disagree ABOUT.
    """
    data = json.loads(LEDGER.read_text())
    cands = [r for r in data["records"]
             if r.get("functions") and Path(r["path"]).suffix in SRC_EXT
             and (TREES["hbsd"] / r["path"]).is_file()]
    random.seed(seed)
    picked = random.sample(cands, min(sample, len(cands)))

    tp = fp = fn_ = 0
    ex_fp, ex_fn = [], []
    for rec in picked:
        want = set(rec["functions"])
        got = set(scan_file(TREES["hbsd"] / rec["path"]))
        # The scanner qualifies C++ methods; the ledger does not. Compare
        # on the bare name so the two are talking about the same thing.
        got_bare = {g.split("::")[-1] for g in got}
        tp += len(want & got_bare)
        for x in sorted(got_bare - want):
            fp += 1
            if len(ex_fp) < 12:
                ex_fp.append(f"{rec['path']}:{x}")
        for x in sorted(want - got_bare):
            fn_ += 1
            if len(ex_fn) < 12:
                ex_fn.append(f"{rec['path']}:{x}")

    prec = tp / (tp + fp) if (tp + fp) else 0.0
    rec_ = tp / (tp + fn_) if (tp + fn_) else 0.0
    print(f"== textual scanner vs the ledger, {len(picked)} files, seed {seed}")
    print(f"  agreed            {tp}")
    print(f"  scanner only (FP) {fp}")
    print(f"  ledger only (FN)  {fn_}")
    print(f"  precision {prec:.3f}   recall {rec_:.3f}")
    print("\n  scanner found, ledger did not:")
    for x in ex_fp:
        print(f"    {x}")
    print("\n  ledger found, scanner did not:")
    for x in ex_fn:
        print(f"    {x}")
    print("""
  Read this as the error bar on every row whose src is `textual' - which
  is every row in kde/ and pbsd/. It is NOT the error bar on hbsd/src,
  where the ledger is used directly.""")
    return 0


# ---------------------------------------------------------------------- main

def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--out", default="verify_universe.jsonl")
    ap.add_argument("--tree", action="append", default=[],
                    choices=sorted(TREES) + ["all"],
                    help="repeatable; default all")
    ap.add_argument("--ledger", default=str(LEDGER))
    ap.add_argument("--selftest", action="store_true",
                    help="measure the scanner against the ledger")
    ap.add_argument("--sample", type=int, default=300)
    ap.add_argument("--seed", type=int, default=20260917)
    ap.add_argument("--scan", help="scan one file and print what it finds")
    args = ap.parse_args(argv)

    if args.scan:
        for nm in scan_file(Path(args.scan)):
            print(nm)
        return 0
    if args.selftest:
        return selftest(args.sample, args.seed)

    trees = args.tree or ["all"]
    if "all" in trees:
        trees = sorted(TREES)
    out = Path(args.out)
    counts = build(out, trees, Path(args.ledger))
    total = counts["ledger"] + counts["textual"]
    print(f"== universe -> {out}")
    print(f"  from the ledger   {counts['ledger']}")
    print(f"  from the scanner  {counts['textual']}"
          "   (error bar: run --selftest)")
    print(f"  rows              {total}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
