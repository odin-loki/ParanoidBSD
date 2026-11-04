#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""AW_CLK_FACTOR_ZERO_BASED on a factor that divides.

Fifteen core.DivideZero findings under sys/dev/clk/allwinner/ are one
flag.  A clock factor is a register field plus a rule for reading it, and
aw_clk.h gives one rule a minimum of zero:

    static inline uint32_t
    aw_clk_factor_get_min(struct aw_clk_factor *factor)
    {
            ...
            else if (factor->flags & AW_CLK_FACTOR_ZERO_BASED)
                    min = 0;                        (aw_clk.h:151)

so a factor carrying that flag can be zero, and the frequency searches
divide by factors:

    cur = fparent / n / m;                          (aw_clk_nm.c:150)
    *freq = *freq / prediv / div;                   (aw_clk_prediv_mux.c:121)

A kernel division by zero on arm or arm64 is a panic, not a signal.

The fifteen are false, and what makes them false is not in the code.  It
is in the driver DATA: AW_CLK_FACTOR_ZERO_BASED appears thirteen times in
the whole tree, three in ccu_a10.c and ten in ccu_a83t.c, and every one
of them is the `n' factor of an NKMP_CLK.  In NKMP `n' multiplies -

    cur = (fparent * n * k) / (m * p);              (aw_clk_nkmp.c:155)

- so n == 0 gives a candidate frequency of zero, which the search
rejects, and never a division.  A fourteenth line putting the flag on an
`m', `p', `div' or `prediv' field would make all fifteen live at once,
and nothing in the tree would say so.

That is what this checks.  For each clock-definition macro it reads which
positional argument sets which factor's flags; for each driver it reads
which factors appear in a DENOMINATOR of the frequency arithmetic; and
then it reads every macro invocation in the tree and reports any
zero-based factor that the driver divides by.

The invariant is per-driver, not per-name: `n' multiplies in NKMP, NP,
NMM and MIPI, and DIVIDES in NM (aw_clk_nm.c:150).  A rule about the
letter would be wrong for one of the two.

    python3 tools/verify/aw_clk_zero_based.py           # report
    python3 tools/verify/aw_clk_zero_based.py --gate    # exit 1 on a fault
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys

SRC = pathlib.Path(__file__).resolve().parents[2] / "hbsd" / "src"
CLKDIR = "sys/dev/clk/allwinner"
HEADER = f"{CLKDIR}/aw_clk.h"

FLAG = "AW_CLK_FACTOR_ZERO_BASED"

# The two inputs to every frequency expression.  Anything else in one that
# the reader cannot map to a factor field is a fault, not a shrug.
NOT_A_FACTOR = frozenset({"fparent", "freq", "fout", "best", "cur"})

_MACRO = re.compile(r"^#define\s+([A-Z0-9_]+)\(", re.M)
_DEFSTRUCT = re.compile(r"struct\s+aw_clk_([a-z0-9_]+)_def\b")
_FLAGS_FIELD = re.compile(r"\.\s*([a-z0-9_]+)\s*\.\s*flags\s*=\s*(_[a-z0-9_]+)\s*,")
_LOCAL = re.compile(
    r"\b([A-Za-z_]\w*)\s*=\s*aw_clk_(?:get_factor|factor_get_min)\s*\(\s*"
    r"(?:[A-Za-z_]\w*\s*,\s*)?&\s*sc\s*->\s*([A-Za-z_]\w*)\s*\)"
)
_FREQ_EXPR = re.compile(r"^\s*(?:cur|\*\s*freq)\s*=\s*(.+?);\s*$", re.M)
_TOKEN = re.compile(r"[A-Za-z_]\w*|[()*/]")


class Fault(Exception):
    """Something the reader needs and cannot get.  Never a silent skip."""


def _read(rel: str) -> str:
    p = SRC / rel
    if not p.is_file():
        raise Fault(f"{rel}: not in the tree")
    return p.read_text(errors="replace")


def _join(text: str) -> str:
    return text.replace("\\\n", " ")


def denominator_names(expr: str) -> set[str]:
    """The identifiers an odd number of `/' operators govern.

    `(fparent * n * k) / (m * p)' puts m and p in the denominator and
    nothing else; `fparent / n / m' puts both n and m there.  A `(' seen
    just after a `/' inverts everything inside it, which is the only
    reason this needs a stack rather than a boolean.
    """
    ctx = [False]
    div = False
    out: set[str] = set()
    for tok in _TOKEN.findall(expr):
        if tok == "/":
            div = True
        elif tok == "*":
            div = False
        elif tok == "(":
            ctx.append(ctx[-1] ^ div)
            div = False
        elif tok == ")":
            if len(ctx) > 1:
                ctx.pop()
            div = False
        else:
            if ctx[-1] ^ div:
                out.add(tok)
            div = False
    return out


def macros(header: str) -> dict[str, tuple[str, dict[int, str]]]:
    """macro name -> (driver kind, {argument index: factor field}).

    The macros are positional, so which argument carries a factor's flags
    is only knowable from the macro body's `.n.flags = _n_flags,'.
    """
    text = _join(header)
    out: dict[str, tuple[str, dict[int, str]]] = {}
    for m in _MACRO.finditer(text):
        name = m.group(1)
        i = text.index("(", m.start())
        depth, j = 0, i
        while j < len(text):
            if text[j] == "(":
                depth += 1
            elif text[j] == ")":
                depth -= 1
                if depth == 0:
                    break
            j += 1
        params = [p.strip() for p in text[i + 1 : j].split(",")]
        body = text[j + 1 : text.index("\n", j) if "\n" in text[j:] else len(text)]
        # The body runs to the end of the (joined) logical line.
        body = text[j + 1 :].split("\n", 1)[0]

        kind = _DEFSTRUCT.search(body)
        if kind is None:
            continue                      # not a clock-definition macro
        index = {p: n for n, p in enumerate(params)}
        fields: dict[int, str] = {}
        for field, param in _FLAGS_FIELD.findall(body):
            if param not in index:
                raise Fault(f"{name}: .{field}.flags is set from {param}, "
                            "which is not one of its parameters")
            fields[index[param]] = field
        if not fields:
            raise Fault(f"{name}: builds an aw_clk_{kind.group(1)}_def "
                        "but sets no factor flags")
        out[name] = (kind.group(1), fields)
    if not out:
        raise Fault(f"{HEADER}: no clock-definition macros found")
    return out


def divisors(kind: str) -> set[str]:
    """The factor fields aw_clk_<kind>.c divides a frequency by."""
    rel = f"{CLKDIR}/aw_clk_{kind}.c"
    text = _read(rel)
    local = dict(_LOCAL.findall(text))
    exprs = _FREQ_EXPR.findall(text)
    if not exprs:
        raise Fault(f"{rel}: no `cur =' or `*freq =' frequency expression")

    out: set[str] = set()
    for e in exprs:
        for name in denominator_names(e):
            if name in NOT_A_FACTOR:
                continue
            if name not in local:
                raise Fault(
                    f"{rel}: `{e}' divides by `{name}', which is not read "
                    "from any sc-> factor - the reader cannot say which "
                    "factor that is"
                )
            out.add(local[name])

    # Every one of these drivers divides a frequency by at least one
    # factor - that is what makes the DivideZero findings reachable in
    # the first place. A kind that comes back with none has not been
    # proved safe; the reader has failed to find the arithmetic, and
    # would then wave through a zero-based flag on any of its factors.
    if not out:
        raise Fault(
            f"{rel}: no factor appears in a denominator of "
            + "; ".join(f"`{e}'" for e in exprs)
            + " - if the division moved into a helper, this reader now "
            "passes everything in this driver"
        )
    return out


def split_args(text: str, start: int) -> list[tuple[str, int]] | None:
    """The top-level arguments of a macro invocation, each with its offset.

    The offset is kept because these definitions run to twelve lines and
    the answer wanted is which LINE carries the flag, not which line the
    macro name is on.
    """
    i = text.find("(", start)
    if i < 0:
        return None
    depth, j, args, cur, at = 0, i, [], [], i + 1
    while j < len(text):
        c = text[j]
        if c == "(":
            depth += 1
            if depth == 1:
                j += 1
                at = j
                continue
        elif c == ")":
            depth -= 1
            if depth == 0:
                args.append(("".join(cur), at))
                return [(a.strip(), o) for a, o in args]
        elif c == "," and depth == 1:
            args.append(("".join(cur), at))
            cur = []
            j += 1
            at = j
            continue
        cur.append(c)
        j += 1
    return None


def strip_comments(text: str) -> str:
    """Blank the comments and KEEP the line count.

    The clock definitions carry a `/* n factor */' on every line, so the
    flag has to be read out of code rather than comments - but collapsing
    a multi-line comment to one space moves every line after it. The
    first draft did that and reported ccu_a83t.c:368 for a definition at
    :397, which is a wrong answer that looks like a right one.
    """
    return re.sub(
        r"/\*.*?\*/", lambda m: " " + "\n" * m.group(0).count("\n"),
        text, flags=re.S)


def check() -> tuple[list[str], list[str]]:
    header = _read(HEADER)
    if f"#define\t{FLAG}" not in header and f"#define {FLAG}" not in header:
        raise Fault(f"{HEADER}: {FLAG} is not defined")
    # The whole check rests on this arm.  If the header stops giving
    # zero-based factors a minimum of zero, the invariant has changed and
    # saying nothing would be the wrong answer.
    if not re.search(
        r"factor->flags\s*&\s*" + FLAG + r"\s*\)\s*\n?\s*min\s*=\s*0\s*;", header
    ):
        raise Fault(
            f"{HEADER}: aw_clk_factor_get_min() no longer returns 0 for {FLAG} "
            "- the premise this check enforces has changed"
        )

    macs = macros(header)
    div_cache: dict[str, set[str]] = {}
    faults: list[str] = []
    lines: list[str] = []

    for name, (kind, fields) in sorted(macs.items()):
        if kind not in div_cache:
            div_cache[kind] = divisors(kind)
        d = sorted(div_cache[kind])
        lines.append(
            f"{name:<16} aw_clk_{kind}.c divides by "
            + (", ".join(d) if d else "(nothing)")
        )
    lines.append("")

    seen = 0
    files = sorted(p.relative_to(SRC).as_posix()
                   for p in (SRC / CLKDIR).glob("*.c"))
    if not files:
        raise Fault(f"{CLKDIR}: no .c files")
    for rel in files:
        # Through _read, like everything else: one reader, so a test that
        # redirects it redirects all of it.
        text = strip_comments(_read(rel))
        if FLAG not in text:
            continue
        for name, (kind, fields) in macs.items():
            for m in re.finditer(r"\b" + name + r"\s*\(", text):
                args = split_args(text, m.start())
                if args is None:
                    raise Fault(f"{rel}: unterminated {name}(...) invocation")
                for idx, (arg, off) in enumerate(args):
                    if FLAG not in arg:
                        continue
                    seen += 1
                    field = fields.get(idx)
                    line = text[: off + arg.index(FLAG)].count("\n") + 1
                    if field is None:
                        raise Fault(
                            f"{rel}:{line}: {name} argument {idx} carries "
                            f"{FLAG}, but that argument sets no factor's flags"
                        )
                    where = f"{rel}:{line} {name} {field} factor"
                    if field in div_cache[kind]:
                        faults.append(
                            f"{where}: aw_clk_{kind}.c DIVIDES by {field}, and "
                            f"{FLAG} gives it a minimum of 0"
                        )
                        lines.append(f"!! {where}")
                    else:
                        lines.append(f"   {where}  (multiplies)")

    if seen == 0:
        raise Fault(
            f"no {FLAG} argument found in any clock definition under {CLKDIR} "
            "- either the flag is gone or this reader stopped finding it"
        )
    return faults, lines + ["", f"{seen} zero-based factor(s) in the tree"]


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--gate", action="store_true", help="exit 1 on any fault")
    args = ap.parse_args(argv)

    try:
        faults, lines = check()
    except Fault as exc:
        print(f"aw_clk_zero_based: cannot check: {exc}", file=sys.stderr)
        return 1

    print("\n".join(lines))
    print()
    if faults:
        print(f"{len(faults)} zero-based factor(s) on a divisor:")
        for f in faults:
            print(f"  {f}")
        return 1 if args.gate else 0
    print("no zero-based factor is divided by; the 15 core.DivideZero "
          "findings under sys/dev/clk/allwinner/ stay unreachable.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
