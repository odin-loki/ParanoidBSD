"""One function validates a parameter and the one next to it does not.

This is the shape behind most of a night's defects in this tree, and it is
worth naming precisely because it is invisible in review: both functions
look correct on their own. Only the pair is wrong.

    arswitch_readphy_internal()   bounds phy and reg
    arswitch_writephy_internal()  bounded reg

    dmc620_pmc_register(unit)     bounds unit against DMC620_UNIT_MAX
    dmc620_pmc_unregister(unit)   did not

    cmn600_pmc_register(unit)     the same file, character for character
    cmn600_pmc_unregister(unit)   the same omission

    isa_dma*(chan)                six of eleven test chan, every one of
                                  them only under #ifdef DIAGNOSTIC, so a
                                  production kernel has no test at all

    g_uzip_taste()                two checks on sc->blksz one line apart;
                                  the first goto e4's, the second printf'd

Those five are two shapes, so this has two detectors:

  SIBLING   functions in one file sharing a parameter NAME AND TYPE, where
            at least one puts a CEILING on it near entry and at least one
            never tests it anywhere. The asymmetry is the signal: a
            parameter nobody checks is a design decision, a parameter half
            the file checks is an oversight.

  ADJACENT  two `if's at the same brace depth in one function, leading with
            the same lvalue, both shaped like rejections, where the first
            transfers control and the second only prints. That is g_uzip:
            the author wrote the check and forgot the bail-out, one line
            below a check that has it.

  WEAK      the SIBLING analysis where every test in the group is one a
            production kernel does not run. That is isa_dma.c, and it is
            not a lesser finding than a missing check - it is a file that
            reads as validated and compiles to one that is not.

A KNOWN FALSE POSITIVE, NAMED

A guard here must be REJECTION-SHAPED -- `if (bad) return' -- and a
condition that WRAPS the dangerous use is not recognised:

    if (mixer_idx >= 0 && mixer_idx < NUM_MIXERS) {
            sc->mixer_volcache[mixer_idx] = volume;      /* <- guarded */
            emumix_set_fxvol(sc, sc->mixer_gpr[mixer_idx], volume);
    }

That is every bit as safe as returning early, and this tool reports it
anyway. sys/dev/sound/pci/emu10kx.c:2605 is the live example: it was a
REAL finding before the `mixer_idx >= 0' half was added, and it is a
false positive afterwards, because the fix kept the file's wrapping
style instead of adopting a rejection guard.

Deciding it properly needs more than the condition: it needs every use
of the parameter in the function to fall inside that `if' body. The
machinery is here -- guards_of() already has the consequent's span --
but it is not wired up, and wiring it up risks a FALSE NEGATIVE in a
tool that is about to gate. Recorded rather than guessed at; fixing it
is a separate measured change.

The source is not reshaped to suit the tool. A lint that makes people
rewrite correct code to silence it is worse than one that is wrong in a
documented way.

WHAT COUNTS AS A GUARD

An `if' whose consequent is return / goto / break / continue / panic(), no
`else', and whose condition tests the parameter. `Tests' is narrow on
purpose, and each narrowing below cost real false positives to find:

  * the parameter must be an OPERAND, not a subscript inside one.
    `chan & ~VALID_DMA_MASK' tests chan; `dma_inuse & (1 << chan)' and
    `dma_bouncebuf[chan] == NULL' test dma_inuse and dma_bouncebuf and USE
    chan unchecked. Reading those as bounds made isa_dma.c look validated.
  * a mask test needs a complement. `chan & ~MASK' asks whether any bit
    outside the legal set is set. `fflags & FREAD' asks which mode we are
    in; taking it for a check reported every USB input driver's close()
    against its open().
  * equality is not validation for an integer. `reg == RL_GMEDIASTAT',
    `sc->sc_phyno != phy', `scsi_id == ahd->our_id' are special cases, not
    bounds. Equality counts only for a pointer, where `== NULL' is the
    whole idiom.
  * at least one member of the group needs a CEILING - a relational test
    against something that is not 0, 1 or -1. `if (cnt < 1) return' is an
    emptiness test and a sibling without one is normally fine.

Two things that look like guards and are counted separately rather than
believed: a guard inside #ifdef DIAGNOSTIC / INVARIANTS / DEBUG, and
KASSERT / MPASS / HALASSERT. GENERIC defines none of those symbols, so on
the kernel anyone runs, the check is not there.

WHAT THIS DELIBERATELY DOES NOT CATCH, AND WHY

  * pointer parameters, unless --include-pointers. `if (p == NULL) return'
    is asymmetric between an internal helper and a public entry point for
    good reasons. Integer parameters - indices, units, channels, register
    numbers - are where the array write is.
  * a bound enforced one level down. If the unguarded sibling passes the
    parameter straight to a guarded one IN THE SAME FILE, in the same
    argument position, that is delegation and it is suppressed. Two levels
    down is not seen (lbggpiocm_pin_setflags -> pin2cpin -> getpad is a
    false positive for exactly this reason), and neither is a bound in
    another translation unit - gpiobus_pin_get() bounds every GPIO pin
    number before the controller method ever sees it.
  * a static helper that only its own guarded caller reaches.
    ice_save_vsi_ctx() writes hw->vsi_ctx[vsi_handle] with no test and is
    called from ice_add_vsi() two lines after ice_add_vsi() tests it. This
    reports it. Fixing that needs a call graph, not a scan.
  * whether a finding is REACHABLE. This says two functions disagree about
    a parameter; it cannot say whether anything can reach the one that
    does not check. Of eight high-confidence findings in sys/dev, five
    were real asymmetries and four of those five were unreachable today
    because every caller passes a constant. That judgement is the
    reviewer's and the tool does not pretend to make it.
  * anything needing types. There is no C parser in this environment
    (pycparser, libclang and tree-sitter are all absent), so parameter
    types are matched on their spelling and the numeric ones are an
    ALLOWLIST. `device_t', `caddr_t' and `bus_space_tag_t' are spelled
    like scalars and are pointers; a `*_t' pattern put a dozen of them in
    the integer set and every one produced a false positive.
  * macro-generated functions. Preprocessor lines are blanked before the
    brace scan, so a function whose header or body comes out of a macro is
    not seen at all.
  * the case where BOTH siblings are missing the check. Nothing to compare
    against; that is a model checker's job, not a lint's.
  * a parameter the unguarded function only prints, or only compares. If
    it never reaches a subscript, a shift, arithmetic, a bit operation or
    a call, nothing can be corrupted by it.

WHAT IT COSTS

Measured, not estimated. Over the pre-fix versions of the five files above
it finds all five. Over the tree as it stands:

    sys/dev   2633 files   8 high   525 medium   49 weak    249 s
    sys/kern   230 files   0 high    95 medium   26 weak     21 s
    sys/x86     73 files   0 high    14 medium   10 weak      2 s
    sys/geom    78 files   0 high     5 medium    2 weak      3 s

All eight high-confidence findings in sys/dev were read against the source.
Five are real asymmetries (pcf8574, emu10kx, tpm_spibus, mthca, cxgbe
t4_hw) and three are false - the `guard' is a write-protection
(vmbus_pcib, superio) or the unguarded sibling is safe by other means
(mlx5_fs_chains searches and bails). That is a 37% false-positive rate on
the tier --gate uses.

The medium tier is the same analysis without the name-pair corroboration.
A 22-finding sample of it over sys/dev came out about nine in ten false,
so it is off by default and is for reading one driver, not a subtree.
"""
from __future__ import annotations

import argparse
import bisect
import os
import re
import sys

# ---------------------------------------------------------------------------
# Lexical preparation
# ---------------------------------------------------------------------------

# Symbols whose #ifdef makes the code inside it absent from a production
# kernel. GENERIC defines none of these. KTR and BOOTVERBOSE are here
# because a check written under them is decoration, same as the rest.
DEBUG_SYMS = {
    "DIAGNOSTIC", "INVARIANTS", "INVARIANT_SUPPORT", "DEBUG", "KDB",
    "KTR", "BOOTVERBOSE", "WITNESS", "MALLOC_DEBUG", "_KERNEL_DEBUG",
}

# Calls that compile to nothing without INVARIANTS. A KASSERT is an
# assertion, not a bound.
# HALASSERT is the ath HAL's, spelled without the underscore, and it is
# `#ifdef AH_ASSERT' - ar5211StartTxDma() asserting `q < HAL_NUM_TX_QUEUES'
# and then writing `1 << q' is the isa_dma shape in a different driver.
WEAK_CALLS = re.compile(
    r"\b(KASSERT|MPASS|VNASSERT|VNPASS|[A-Z][A-Z0-9_]*ASSERT[A-Z0-9_]*)\s*\(")

# Diagnostic output. A consequent made only of these is not a guard, and a
# parameter mentioned only inside one of these is not at risk.
DIAG_CALL = re.compile(
    r"\b(printf|uprintf|log|device_printf|if_printf|kprintf|tprintf|"
    r"DPRINTF|DPRINTF\d|DBG|dprintf|debugf|CTR\d|ktr_\w+|"
    r"[A-Z][A-Z0-9_]*(?:PRINTF|_DBG|_PRINT|_LOG|_WARN|_DEBUG))\s*\(")


def blank_comments_and_literals(text: str) -> str:
    """Comments and the insides of literals become spaces, same length.

    Length has to be preserved: every offset this tool computes is turned
    back into a line number with a bisect over the ORIGINAL text, and the
    reported line number is the only thing a reviewer can act on.
    """
    out = list(text)
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                if out[k] != "\n":
                    out[k] = " "
            i = j
        elif c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            j = n if j < 0 else j
            for k in range(i, j):
                out[k] = " "
            i = j
        elif c in "\"'":
            q, j = c, i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == q or text[j] == "\n":
                    break
                j += 1
            for k in range(i + 1, min(j, n)):
                if out[k] != "\n":
                    out[k] = " "
            i = min(j + 1, n)
        else:
            i += 1
    return "".join(out)


def pp_context(text: str) -> list[tuple[str, ...]]:
    """Per line (0-based), the #if conditions in force, as bare symbols.

    A symbol appears only when the code is inside its POSITIVE branch, so
    `#ifndef DIAGNOSTIC' does not mark its body as diagnostic-only and
    `#else' after `#ifdef DIAGNOSTIC' correctly stops marking it.
    """
    ctx: list[tuple[str, ...]] = []
    stack: list[tuple[str | None, bool]] = []
    cont = False
    for line in text.split("\n"):
        ctx.append(tuple(s for s, neg in stack if s and not neg))
        s = line.strip()
        if cont:
            cont = line.rstrip().endswith("\\")
            continue
        if not s.startswith("#"):
            continue
        cont = line.rstrip().endswith("\\")
        d = s[1:].strip()
        m = re.match(r"ifdef\s+([A-Za-z_]\w*)", d)
        if m:
            stack.append((m.group(1), False))
            continue
        m = re.match(r"ifndef\s+([A-Za-z_]\w*)", d)
        if m:
            stack.append((m.group(1), True))
            continue
        m = re.match(r"if\s+(.*)", d)
        if m:
            stack.append(_parse_if(m.group(1)))
            continue
        m = re.match(r"elif\s+(.*)", d)
        if m:
            if stack:
                stack[-1] = _parse_if(m.group(1))
            continue
        if d.startswith("else"):
            if stack:
                s0, neg = stack[-1]
                stack[-1] = (s0, not neg)
            continue
        if d.startswith("endif"):
            if stack:
                stack.pop()
    return ctx


def _parse_if(cond: str) -> tuple[str | None, bool]:
    """`defined(X)' and `!defined(X)' only. Anything else is opaque."""
    cond = cond.strip()
    neg = False
    if cond.startswith("!"):
        neg, cond = True, cond[1:].strip()
    m = re.fullmatch(r"defined\s*\(\s*([A-Za-z_]\w*)\s*\)", cond)
    if not m:
        m = re.fullmatch(r"defined\s+([A-Za-z_]\w*)", cond)
    return (m.group(1), neg) if m else (None, False)


def blank_pp_lines(text: str) -> str:
    """Preprocessor lines become spaces before the brace scan.

    `#define FOO(x) { ... }' and the half-dozen kernel macros that open a
    brace they never close would otherwise put the whole-file brace depth
    permanently out of step, and then no function after the first one is
    found. This is also why a macro-generated function is invisible here.
    """
    out = []
    cont = False
    for line in text.split("\n"):
        s = line.lstrip()
        if cont or s.startswith("#"):
            cont = line.rstrip().endswith("\\")
            out.append(" " * len(line))
        else:
            out.append(line)
    return "\n".join(out)


# ---------------------------------------------------------------------------
# Function extraction
# ---------------------------------------------------------------------------

IDENT = re.compile(r"[A-Za-z_]\w*$")
CTRL = {"if", "for", "while", "switch", "do", "else", "return", "sizeof"}


class Func:
    __slots__ = ("name", "params", "body", "body_off", "hdr_off", "line",
                 "ret", "nodiag")

    def __init__(self, name, params, body, body_off, hdr_off, line, ret):
        self.nodiag = None
        self.name = name
        self.params = params            # list of (type, pname)
        self.body = body                # blanked text of the body, no braces
        self.body_off = body_off        # offset of body[0] in the file
        self.hdr_off = hdr_off
        self.line = line                # 1-based line of the function name
        self.ret = ret


def match_back(text: str, close: int) -> int:
    """Offset of the `(' matching the `)' at `close', or -1."""
    depth = 0
    i = close
    while i >= 0:
        if text[i] == ")":
            depth += 1
        elif text[i] == "(":
            depth -= 1
            if depth == 0:
                return i
        i -= 1
    return -1


def split_params(s: str) -> list[tuple[str, str]]:
    """(type, name) for each parameter, skipping void and ... .

    A function-pointer parameter `int (*fn)(void)' is named by the
    identifier inside the first parenthesised `*', which is the only form
    of it the kernel actually uses.
    """
    parts, depth, cur = [], 0, []
    for c in s:
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        if c == "," and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(c)
    parts.append("".join(cur))

    out = []
    for p in parts:
        p = " ".join(p.split())
        if not p or p == "void" or p == "...":
            continue
        m = re.search(r"\(\s*\*+\s*([A-Za-z_]\w*)\s*\)", p)
        if m:                                   # function pointer
            out.append((p[:m.start()].strip() + " (*)()", m.group(1)))
            continue
        arr = ""
        while p.endswith("]"):
            j = p.rfind("[")
            if j < 0:
                break
            arr = p[j:] + arr
            p = p[:j].strip()
        m = IDENT.search(p)
        if not m:
            continue                            # unnamed parameter
        name = m.group(0)
        if name in ("void", "int", "char", "long", "short", "unsigned",
                    "signed", "float", "double", "struct", "union", "enum",
                    "const", "volatile"):
            continue                            # a type, not a name
        ty = " ".join(p[:m.start()].split()) + arr
        out.append((ty.strip(), name))
    return out


def find_functions(blank: str, raw: str) -> list[Func]:
    """Every function DEFINITION in the file, by brace matching.

    The rule is: a `{' that takes the depth from 0 to 1, preceded (past
    whitespace and any __attribute__/__unused tail) by a `)' whose matching
    `(' is preceded by an identifier that is not a control keyword. A
    struct initialiser fails it on the `=' before the brace, a prototype
    never opens a brace at all, and a K&R definition - none left in sys/ -
    fails it on the declaration list between `)' and `{'.
    """
    funcs = []
    depth = 0
    i, n = 0, len(blank)
    starts = [0]
    for k, c in enumerate(raw):
        if c == "\n":
            starts.append(k + 1)

    def lineof(off: int) -> int:
        lo, hi = 0, len(starts) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if starts[mid] <= off:
                lo = mid
            else:
                hi = mid - 1
        return lo + 1

    while i < n:
        c = blank[i]
        if c == "{":
            if depth == 0:
                f = _try_header(blank, i, lineof)
                if f:
                    # Body runs to the matching close brace.
                    d, j = 0, i
                    while j < n:
                        if blank[j] == "{":
                            d += 1
                        elif blank[j] == "}":
                            d -= 1
                            if d == 0:
                                break
                        j += 1
                    f.body = blank[i + 1:j]
                    f.body_off = i + 1
                    funcs.append(f)
                    depth = 0
                    i = j + 1
                    continue
            depth += 1
        elif c == "}":
            depth = max(0, depth - 1)
        i += 1
    return funcs


def _try_header(blank: str, brace: int, lineof) -> Func | None:
    j = brace - 1
    # Skip whitespace and trailing attributes: __unused, __printflike(...),
    # __aligned(64) and friends sit between the `)' and the `{'.
    while j >= 0:
        while j >= 0 and blank[j].isspace():
            j -= 1
        if j >= 0 and blank[j] == ")":
            o = match_back(blank, j)
            if o < 0:
                return None
            k = o - 1
            while k >= 0 and blank[k].isspace():
                k -= 1
            m = IDENT.search(blank[:k + 1])
            if m and m.group(0).startswith("__"):
                j = m.start() - 1          # an attribute; keep walking left
                continue
        break
    if j < 0 or blank[j] != ")":
        return None
    open_p = match_back(blank, j)
    if open_p < 0:
        return None
    k = open_p - 1
    while k >= 0 and blank[k].isspace():
        k -= 1
    m = IDENT.search(blank[:k + 1])
    if not m:
        return None
    name = m.group(0)
    if name in CTRL:
        return None
    ret = blank[max(0, m.start() - 120):m.start()]
    # A `)' two tokens back means this was a declarator like
    # `(*fn)(args) {' or a cast; and a `;' or `}' bounds the header.
    tail = ret.rsplit(";", 1)[-1].rsplit("}", 1)[-1]
    if "#" in tail or "=" in tail:
        return None
    params = split_params(blank[open_p + 1:j])
    return Func(name, params, "", 0, m.start(), lineof(m.start()),
                " ".join(tail.split()))


# ---------------------------------------------------------------------------
# Guards
# ---------------------------------------------------------------------------

class Guard:
    __slots__ = ("cond", "kind", "off", "line", "stmt", "depth", "weak",
                 "why_weak", "body")

    def __init__(self, cond, kind, off, line, stmt, depth, weak, why_weak,
                 body):
        self.cond = cond
        self.kind = kind          # return / goto / break / continue / panic
        self.off = off
        self.line = line
        self.stmt = stmt          # ordinal among depth-1 statements
        self.depth = depth
        self.weak = weak          # compiled out of a production kernel
        self.why_weak = why_weak
        self.body = body


TRANSFER = re.compile(r"\b(return|goto|break|continue|panic|vpanic|"
                      r"kassert_panic)\b")


def match_fwd(text: str, open_i: int) -> int:
    depth = 0
    for j in range(open_i, len(text)):
        if text[j] == "(":
            depth += 1
        elif text[j] == ")":
            depth -= 1
            if depth == 0:
                return j
    return -1


def consequent(body: str, start: int) -> tuple[str, int]:
    """The text controlled by an `if', and where it ends.

    Either a braced block or one statement. `else' is not followed: an
    if/else is a dispatch, not a guard, and treating its then-branch as a
    guard was the single largest source of noise in the first version.
    """
    i = start
    while i < len(body) and body[i].isspace():
        i += 1
    if i < len(body) and body[i] == "{":
        d, j = 0, i
        while j < len(body):
            if body[j] == "{":
                d += 1
            elif body[j] == "}":
                d -= 1
                if d == 0:
                    break
            j += 1
        return body[i + 1:j], j + 1
    j = i
    d = 0
    while j < len(body):
        if body[j] in "([":
            d += 1
        elif body[j] in ")]":
            d -= 1
        elif body[j] == ";" and d == 0:
            break
        j += 1
    return body[i:j], j + 1


def has_else(body: str, after: int) -> bool:
    m = re.match(r"\s*else\b", body[after:])
    return m is not None


class Positions:
    """Brace and semicolon offsets in a body, for O(log n) depth queries.

    Counting `{' minus `}' from the start of the body for every `if' in it
    is quadratic, and sys/dev/bxe/bxe.c is half a megabyte with several
    thousand `if's - it alone took eleven seconds. Binary search over three
    sorted lists takes the whole of sys/dev from minutes to under one.
    """
    __slots__ = ("op", "cl", "semi")

    def __init__(self, body: str):
        self.op = [i for i, c in enumerate(body) if c == "{"]
        self.cl = [i for i, c in enumerate(body) if c == "}"]
        self.semi = [i for i, c in enumerate(body) if c == ";"]

    def depth(self, off: int) -> int:
        return bisect.bisect_left(self.op, off) - \
            bisect.bisect_left(self.cl, off)

    def stmts(self, off: int) -> int:
        return bisect.bisect_left(self.semi, off)


def guards_of(f: Func, ctx: list[tuple[str, ...]], lineof) -> list[Guard]:
    """Every guard in the function, with where and how strong it is."""
    out = []
    body = f.body
    pos = Positions(body)
    for m in re.finditer(r"\bif\s*\(", body):
        op = m.end() - 1
        cp = match_fwd(body, op)
        if cp < 0:
            continue
        cond = body[op + 1:cp]
        cons, end = consequent(body, cp + 1)
        if has_else(body, end):
            continue
        t = TRANSFER.search(cons)
        if not t:
            continue
        # A `break' inside a switch case is a case arm, not a guard, and a
        # `break'/`continue' inside a loop over something else is loop
        # control. Both are still transfers; only their siblings decide.
        d = pos.depth(m.start())
        off = f.body_off + m.start()
        line = lineof(off)
        act = ctx[line - 1] if line - 1 < len(ctx) else ()
        dbg = [s for s in act if s in DEBUG_SYMS]
        weak, why = (True, "#ifdef " + dbg[0]) if dbg else (False, "")
        # Statement ordinal at depth 1: `;' plus closed blocks before it.
        stmt = pos.stmts(m.start()) if d == 0 else 0
        out.append(Guard(" ".join(cond.split()), t.group(1), off, line,
                         stmt, d, weak, why, " ".join(cons.split())))
    # KASSERT and friends: recorded as weak guards so a file that only
    # asserts is reported as such rather than reported as guarded.
    for m in WEAK_CALLS.finditer(body):
        op = m.end() - 1
        cp = match_fwd(body, op)
        if cp < 0:
            continue
        cond = body[op + 1:cp].split(",")[0]
        d = pos.depth(m.start())
        off = f.body_off + m.start()
        line = lineof(off)
        stmt = pos.stmts(m.start()) if d == 0 else 0
        out.append(Guard(" ".join(cond.split()), m.group(1), off, line, stmt,
                         d, True, m.group(1) + "()", ""))
    out.sort(key=lambda g: g.off)
    return out


# ---------------------------------------------------------------------------
# Is the parameter the SUBJECT of a test, or just a subscript inside one?
#
# This distinction is the whole precision of the tool, and getting it wrong
# in either direction ruins it. Two real conditions from isa_dma.c:
#
#     if (chan & ~VALID_DMA_MASK)                 <- a test of chan
#     if (dma_inuse & (1 << chan))                <- a test of dma_inuse
#
# and two from isa_dmastart():
#
#     if (dma_bouncebuf[chan] == NULL)            <- a test of dma_bouncebuf
#     if (chan < 4 && nbytes > (1 << 16))         <- a test of chan, and of
#                                                    nbytes
#
# The second of each pair USES chan without checking it - it is an instance
# of the very defect, not a defence against it. Reading them as bounds made
# isa_dma.c look like a file whose channel numbers are validated, when the
# only checks it has are the six #ifdef DIAGNOSTIC ones.
# ---------------------------------------------------------------------------

CAST = re.compile(r"^\(\s*(?:const\s+|volatile\s+|struct\s+|unsigned\s+|"
                  r"signed\s+)*[A-Za-z_]\w*(?:\s*\*)*\s*\)")
CMP = re.compile(r"<=|>=|==|!=|<<|>>|[<>]")


def _split_top(s: str, ops: tuple[str, ...]) -> list[str]:
    parts, depth, i, last = [], 0, 0, 0
    while i < len(s):
        c = s[i]
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        elif depth == 0:
            for o in ops:
                # `pi->dev == NULL' is not a `>' comparison between `pi-'
                # and `dev == NULL'. Reading it as one made every `dev'
                # parameter in the tree look tested by every `->dev' in a
                # condition, which is where a dozen of the first run's
                # high-confidence findings came from.
                if o[0] == ">" and i and s[i - 1] == "-":
                    continue
                if s.startswith(o, i):
                    parts.append(s[last:i])
                    i += len(o)
                    last = i
                    break
            else:
                i += 1
                continue
            continue
        i += 1
    parts.append(s[last:])
    return [x.strip() for x in parts if x.strip()]


def _peel(e: str) -> str:
    """Strip outer parens, unary operators and casts."""
    prev = None
    while prev != e:
        prev = e
        e = e.strip()
        while e[:1] in ("!", "~", "-", "+", "*", "&") and e[:2] != "&&":
            e = e[1:].strip()
        m = CAST.match(e)
        if m and not e[m.end():].strip().startswith("("):
            e = e[m.end():].strip()
            continue
        if e.startswith("(") and match_fwd(e, 0) == len(e) - 1:
            e = e[1:-1]
    return e.strip()


def primary_var(e: str) -> str | None:
    """What the expression is ABOUT: its leading lvalue.

    `dma_inuse & (1 << chan)' is about dma_inuse. `1 << chan' is about
    chan, because nothing else in it is a variable. An ALL-CAPS call is
    looked inside - VALID_CHAN(chan) is a predicate on chan and the kernel
    writes real checks that way.
    """
    e = _peel(e)
    m = re.match(r"([A-Za-z_]\w*)\s*\(", e)
    if m and m.group(1).isupper() and match_fwd(e, m.end() - 1) >= 0:
        inner = e[m.end():match_fwd(e, m.end() - 1)]
        return primary_var(inner.split(",")[0]) if inner.strip() else None
    m = re.match(r"[A-Za-z_]\w*", e)
    if m:
        return m.group(0)
    m = re.search(r"[A-Za-z_]\w*", e)     # `1 << chan', `0 > chan'
    return m.group(0) if m else None


REL = ("<=", ">=", "<", ">")
EQ = ("==", "!=")


def tests(cond: str, p: str, allow_eq: bool = False,
          masks: set[str] | None = None) -> bool:
    """Is `p' the subject of something in this condition that can reject it?

    Three deliberate refusals, each of which was a measured fistful of
    false positives before it went in:

    `if (chan)' is not a test. It is a mode flag far more often than a
    check, and counting it made every driver with an `if (flags)' in it
    look like it validated flags.

    `if (fflags & FREAD) return' is not a test either - it is a dispatch on
    which direction a device was opened, and every USB input driver has one
    in its open() and none in its close(). A MASK test only counts with a
    complement in it: `chan & ~VALID_DMA_MASK' asks whether any bit outside
    the legal set is set, which is a validity question. `x & FLAG' asks
    which mode we are in, which is not.

    `if (reg == RL_GMEDIASTAT) return CSR_READ_1(...)' is not a test: it is
    a special case, and so are `if (sc->sc_phyno != phy) return', `if
    (scsi_id == ahd->our_id) return' and `if (sc->owner != dev) panic'. A
    bound on an integer is written with a relational operator essentially
    always, so equality is only accepted for a pointer, where `== NULL' is
    the whole idiom.
    """
    cond = " ".join(cond.split())
    if not re.search(rf"\b{re.escape(p)}\b", cond):
        return False
    ops = REL + EQ if allow_eq else REL
    for conj in _split_top(cond, ("&&", "||")):
        conj = _peel(conj)
        sides = _split_top(conj, ops)
        if len(sides) > 1:
            # A comparison. The parameter has to BE one of the operands.
            if any(primary_var(s) == p for s in sides):
                return True
            continue
        if primary_var(conj) != p:
            continue
        # `x & ~MASK' - is any bit outside the legal set set? - is a
        # validity test. `x & FLAG' is a mode test. The complement is the
        # whole difference between them.
        if "~" in conj and re.search(r"[&|^]", conj):
            return True
        # ...and the complement is allowed to be one line further up:
        #     mask = (unsigned short)~0x1f;
        #     if (reg_num & mask) return EINVAL;
        # is qat_hal_put_rel_wr_xfer(), which this called unguarded - and
        # reported against its rd_xfer twin - until `masks' was passed in.
        if masks and re.search(r"[&|^]", conj):
            for v in re.findall(r"[A-Za-z_]\w*", conj):
                if v != p and v in masks:
                    return True
        if allow_eq and re.search(r"\bNULL\b", conj):
            return True
        # An ALL-CAPS predicate on the parameter - VALID_CHAN(chan) - USED
        # to count. It was dropped: the only thing it ever matched in four
        # scopes was `RACCT_IS_DENIABLE(resource)', which is a per-resource
        # policy bit and not a bound, and it reported racct_set_unlocked()
        # against racct_set_locked() for having no copy of it.
    return False


def complement_locals(body: str) -> set[str]:
    """Locals in this body assigned an expression containing `~'."""
    out = set()
    for m in re.finditer(r"\b([a-z_]\w*)\s*=\s*([^;=][^;]{0,80});", body):
        if "~" in m.group(2):
            out.add(m.group(1))
    return out


TRIVIAL = {"0", "1", "-1", "0x0", "0x1"}


def bounds_like(cond: str, p: str) -> bool:
    """Does this condition put an actual CEILING on `p'?

    `if (cnt < 1) return' and `if (idx < 0) return' do not. They are
    emptiness and sign tests, and a sibling that omits one is usually fine
    - ixl_del_hw_filters() with cnt == 0 mallocs nothing and runs its loop
    zero times, which is why reporting it against ixl_add_hw_filters() was
    noise. A ceiling is different: the value indexes something, and the
    sibling without the ceiling indexes it out of range.

    At least one member of a group must have a ceiling before the group is
    reported at all. That one condition removed a quarter of the
    high-confidence findings and none of the true ones.
    """
    cond = " ".join(cond.split())
    for conj in _split_top(cond, ("&&", "||")):
        conj = _peel(conj)
        if "~" in conj and re.search(r"[&|^]", conj) and \
                primary_var(conj) == p:
            return True
        sides = _split_top(conj, REL)
        if len(sides) != 2:
            continue
        for i, s in enumerate(sides):
            if primary_var(s) == p and sides[1 - i].strip() not in TRIVIAL:
                return True
    return False


# What a parameter has to DO before a missing bound on it can hurt. Being
# handed to another translation unit does not count: arswitch's `external'
# PHY accessors pass phy and reg straight to MDIO_READREG() and nothing in
# this file can be corrupted by either, which is why the tree bounded the
# `internal' pair and left the external one alone. Requiring a subscript,
# a shift or arithmetic is the difference between reporting that pair and
# reporting all four.
RISK = [
    (r"\[[^][]*\b{p}\b[^][]*\]", "array subscript"),
    (r"\b{p}\b\s*(<<|>>)|(<<|>>)\s*\b{p}\b", "shift"),
    (r"\b{p}\b\s*[-+*/%]|[-+*/%]\s*\b{p}\b", "arithmetic"),
    (r"\b{p}\b\s*[&|^]|[&|^]\s*\b{p}\b", "bit operation"),
]


def strip_diag(body: str) -> str:
    """Body with every printf-family call removed, arguments and all."""
    out, i = [], 0
    for m in DIAG_CALL.finditer(body):
        if m.start() < i:
            continue
        cp = match_fwd(body, m.end() - 1)
        if cp < 0:
            continue
        out.append(body[i:m.start()])
        i = cp + 1
    out.append(body[i:])
    return "".join(out)


def risky_use(f: "Func", p: str, local_funcs: set[str]) -> str | None:
    """How the parameter could hurt, or None if it could not."""
    if f.nodiag is None:
        f.nodiag = strip_diag(f.body)
    b = f.nodiag
    if not re.search(rf"\b{re.escape(p)}\b", b):
        return None                      # printed and nothing else
    for pat, why in RISK:
        if re.search(pat.replace("{p}", re.escape(p)), b):
            return why
    # Handed to a function defined in THIS file: the risk is real, it is
    # just one level down, and this tool can see that level.
    for m in re.finditer(r"\b([a-z_]\w*)\s*\(", b):
        if m.group(1) not in local_funcs:
            continue
        cp = match_fwd(b, m.end() - 1)
        if cp < 0:
            continue
        if p in [a.strip() for a in re.split(r",(?![^()]*\))",
                                             b[m.end():cp])]:
            return f"passed to {m.group(1)}()"
    return None


def delegates(f: Func, p: str, guarded_names: set[str]) -> str | None:
    """Is the parameter handed straight to a guarded sibling?

    `isa_dmastatus(chan)' calling `isa_dmastatus_locked(chan)' is the
    shape. Only an exact argument match counts - `p' alone, in any
    position - because `p + 1' or `p & 3' is a different value and the
    callee's bound does not cover the caller's.
    """
    for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", f.body):
        callee = m.group(1)
        if callee not in guarded_names or callee == f.name:
            continue
        cp = match_fwd(f.body, m.end() - 1)
        if cp < 0:
            continue
        args = [a.strip() for a in
                re.split(r",(?![^()]*\))", f.body[m.end():cp])]
        if p in args:
            return callee
    return None


# ---------------------------------------------------------------------------
# Name pairs
# ---------------------------------------------------------------------------

# Opposed verbs. LOOSE ones may carry a suffix - `readphy'/`writephy' is
# the arswitch case and the verb is not its own word there. SHORT ones must
# be, because `set' inside `setup' and `in' inside `init' pair everything
# with everything.
LOOSE = [
    ("read", "write"), ("register", "unregister"), ("alloc", "free"),
    ("acquire", "release"), ("attach", "detach"), ("enable", "disable"),
    ("lock", "unlock"), ("create", "destroy"), ("load", "unload"),
    ("setup", "teardown"), ("send", "recv"), ("insert", "remove"),
    ("encode", "decode"), ("compress", "decompress"),
]
SHORT = [
    ("init", "fini"), ("get", "set"), ("get", "put"), ("open", "close"),
    ("start", "stop"), ("add", "remove"), ("add", "del"), ("map", "unmap"),
    ("in", "out"), ("rd", "wr"), ("push", "pop"), ("hold", "drop"),
    ("ref", "unref"), ("tx", "rx"), ("up", "down"), ("on", "off"),
]


def pair_of(a: str, b: str) -> str | None:
    """Do these two names differ only by an opposed verb?

    Substitute the verb out of each and compare what is left. If the
    remainders are identical the two names ARE each other's opposite, and
    that is the strongest evidence this tool has that the pair was meant to
    be symmetrical - which is exactly the claim a missing check breaks.
    """
    la, lb = a.lower(), b.lower()
    for table, tail in ((LOOSE, r""), (SHORT, r"(?![a-z])")):
        for x, y in table:
            for u, v in ((x, y), (y, x)):
                ra = re.sub(rf"(?<![a-z]){re.escape(u)}{tail}", "\x00", la, 1)
                rb = re.sub(rf"(?<![a-z]){re.escape(v)}{tail}", "\x00", lb, 1)
                if ra == rb and "\x00" in ra and ra != la:
                    return f"{u}/{v}"
    return None


# ---------------------------------------------------------------------------
# Detector A: sibling functions
# ---------------------------------------------------------------------------

# Parameter types this groups on by default. It is an allowlist, not a
# pattern, because there is no C parser here and `device_t', `caddr_t' and
# `bus_space_tag_t' are all spelled like scalars and are all pointers. A
# pattern over `*_t' put a dozen pointer parameters into the integer set
# and every one of them produced a false positive - the `guard' found was
# an ownership test (`sc->owner != dev') or a NULL test on a different
# object (`pi->dev == NULL'), never a bound.
NUMERIC = {
    "int", "unsigned", "unsigned int", "signed int", "long", "unsigned long",
    "long long", "unsigned long long", "short", "unsigned short", "char",
    "unsigned char", "signed char",
    "u_int", "u_long", "u_short", "u_char", "uint", "ulong", "ushort",
    "size_t", "ssize_t", "off_t", "daddr_t", "lba_t", "blkcnt_t",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "u_int8_t", "u_int16_t", "u_int32_t", "u_int64_t",
    "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64",
    "__u8", "__u16", "__u32", "__u64", "__s8", "__s16", "__s32", "__s64",
    "bus_size_t", "bus_addr_t", "bus_space_handle_t",
    "vm_offset_t", "vm_paddr_t", "vm_size_t", "vm_pindex_t", "vm_prot_t",
    "pid_t", "uid_t", "gid_t", "mode_t", "nlink_t", "ino_t", "dev_t",
    "time_t", "clock_t", "register_t", "intptr_t", "uintptr_t",
    "quad_t", "u_quad_t", "sbintime_t", "lwpid_t", "cpuid_t", "cap_rights_t",
    "bool", "boolean_t", "sector_t", "pc_t", "rman_res_t",
}

NOT_SCALAR = re.compile(r"\*|\[")


def scalar_param(ty: str) -> bool:
    if NOT_SCALAR.search(ty):
        return False
    t = " ".join(ty.split())
    for q in ("const ", "volatile ", "register ", "_Atomic "):
        while t.startswith(q):
            t = t[len(q):]
    return t in NUMERIC


class Finding:
    def __init__(self, path, kind, sev, param, ty, guarded, unguarded,
                 detail, pair, line):
        self.path = path
        self.kind = kind
        self.sev = sev          # "high" / "medium" / "weak"
        self.param = param
        self.ty = ty
        self.guarded = guarded
        self.unguarded = unguarded
        self.detail = detail
        self.pair = pair
        self.line = line


def scan_siblings(path, rel, funcs, ctx, lineof, args) -> list[Finding]:
    out = []
    local_funcs = {f.name for f in funcs}
    # One finding per function PAIR, not per parameter: mlx5_chains_get_table
    # and mlx5_chains_put_table differ on chain, prio AND level, and printing
    # that three times says the same thing three times.
    pairs: dict[tuple[str, str], list] = {}
    # (parameter name, parameter type as spelled) -> members of the group
    groups: dict[tuple[str, str], list] = {}
    for f in funcs:
        f_guards = guards_of(f, ctx, lineof)
        masks = complement_locals(f.body)
        for ty, p in f.params:
            if not scalar_param(ty) and not (
                    args.include_pointers and NOT_SCALAR.search(ty)):
                continue
            key = (p, " ".join(ty.split()))
            eq = not scalar_param(ty)          # a pointer; `== NULL' counts
            groups.setdefault(key, []).append(
                (f, [g for g in f_guards if tests(g.cond, p, eq, masks)]))

    for (p, ty), members in groups.items():
        if len(members) < 2:
            continue
        # A name defined twice is one function and its #ifdef/#else twin;
        # comparing it with itself is noise.
        if len({f.name for f, _ in members}) < len(members):
            continue

        strong: list[tuple[Func, Guard]] = []   # a check a real kernel runs
        weak: list[tuple[Func, Guard]] = []     # #ifdef DIAGNOSTIC, KASSERT
        none: list[Func] = []
        for f, gs in members:
            hard = [g for g in gs if not g.weak]
            entry = [g for g in hard
                     if g.depth == 0 and g.stmt <= args.window]
            if entry:
                strong.append((f, entry[0]))
            elif hard:
                strong.append((f, hard[0]))
            elif gs:
                weak.append((f, gs[0]))
            else:
                none.append(f)

        guarded_names = {f.name for f, _ in strong} | {f.name for f, _ in weak}
        bygf = dict(strong)

        def keep(f: Func) -> str | None:
            """None means this silence is fine. Otherwise, how it can hurt."""
            why = risky_use(f, p, local_funcs)
            if why is None:
                return None
            if delegates(f, p, guarded_names):
                return None
            return why

        # (1) the asymmetry proper: someone tests it near entry, someone
        # never tests it anywhere.
        ceiling = [(f, g) for f, g in strong if bounds_like(g.cond, p)]
        if ceiling and none and any(g.depth == 0 and g.stmt <= args.window
                                    for _, g in ceiling):
            for f in none:
                why = keep(f)
                if why is None:
                    continue
                gf, gg = min(ceiling, key=lambda t: t[1].stmt)
                pr = None
                for cf, cg in ceiling:
                    q = pair_of(cf.name, f.name)
                    if q:
                        pr, gf, gg = q, cf, cg
                        break
                pairs.setdefault((gf.name, f.name), []).append(
                    (p, ty, gf, gg, f, why, pr))

        # (2) the group only ever tests it where a production kernel has no
        # test at all. isa_dma.c is this and it is not a lesser finding: a
        # file that reads as carefully validated compiles to one that is
        # not validated anywhere.
        if weak and not strong:
            exposed = [f for f in none if keep(f)]
            if not exposed and not args.include_pointers:
                continue
            wf, wg = weak[0]
            det = (f"{wf.name}():{wg.line}  if ({wg.cond}) {wg.kind}"
                   f"   <- {wg.why_weak}")
            det += (f"\n      {len(weak)} of {len(members)} function(s) "
                    f"test it and every one of them does so weakly: "
                    f"{', '.join(sorted(f.name for f, _ in weak)[:8])}")
            if exposed:
                det += (f"\n      never tested at all: "
                        f"{', '.join(sorted(f.name for f in exposed)[:8])}")
            # A KASSERT is how the kernel is written; a file full of them
            # with one sibling that has none is ordinary, not a defect, and
            # it is two thirds of everything this detector says. An
            # `#ifdef DIAGNOSTIC' around an if/panic is different: someone
            # wrote a real check and then arranged for it not to be
            # compiled, and when SEVERAL functions do it the file reads as
            # validated and is not. That is isa_dma.c and it is the only
            # shape promoted to high.
            ifdefd = [w for _, w in weak if w.why_weak.startswith("#ifdef")]
            sev = "high" if (exposed and len(ifdefd) >= 2) else "medium"
            out.append(Finding(
                path, "WEAK", sev, p, ty,
                (wf.name, wg.line),
                (exposed[0].name, exposed[0].line) if exposed else ("-", 0),
                det, None, wg.line))

    for (gname, uname), rows in pairs.items():
        p, ty, gf, gg, f, why, pr = rows[0]
        names = ", ".join(f"{t2} {p2}" for p2, t2, *_ in rows)
        out.append(Finding(
            path, "SIBLING", "high" if pr else "medium", names, ty,
            (gf.name, gg.line), (f.name, f.line),
            f"{gname}():{gg.line}  if ({gg.cond}) {gg.kind}\n"
            f"      {uname}():{f.line}  no test; {p} reaches {why}",
            pr, f.line))
    return out


# ---------------------------------------------------------------------------
# Detector B: adjacent checks, one bails out and one does not
# ---------------------------------------------------------------------------

LVALUE = re.compile(r"[A-Za-z_]\w*(?:\s*(?:->|\.)\s*[A-Za-z_]\w*)*")
KEYWORDS = {"sizeof", "if", "return", "NULL", "true", "false"}


def lead_lvalue(cond: str) -> str | None:
    """The first thing the condition talks about.

    `sc->blksz % 512 != 0' and `sc->blksz > MAX_BLKSZ' both lead with
    `sc->blksz'. `(dma_inuse & (1 << chan)) == 0' leads with `dma_inuse'
    and `(dma_busy & ...) == 0' leads with `dma_busy', which is why those
    two - adjacent, one printing and one returning, in isa_dmastop() -
    are correctly NOT a finding. They check different things.
    """
    for m in LVALUE.finditer(cond):
        t = " ".join(m.group(0).split()).replace(" ", "")
        head = re.match(r"[A-Za-z_]\w*", t).group(0)
        if head in KEYWORDS:
            continue
        # A bare all-caps token is a macro constant, not a subject.
        if "->" not in t and "." not in t and t.isupper():
            continue
        return t
    return None


def diag_only(cons: str) -> bool:
    """Body is nothing but diagnostic output - no transfer, no assignment."""
    if TRANSFER.search(cons):
        return False
    if not DIAG_CALL.search(cons):
        return False
    rest = cons
    for m in list(DIAG_CALL.finditer(cons))[::-1]:
        cp = match_fwd(cons, m.end() - 1)
        if cp < 0:
            return False
        rest = rest[:m.start()] + rest[cp + 1:]
    return not re.search(r"[A-Za-z_0-9]", rest)


LIT = re.compile(r"^(0[xX][0-9a-fA-F]+|\d+)[uUlL]*$")


def rejection_shaped(cond: str, subj: str) -> bool:
    """Is this condition asking whether `subj' is out of bounds?

    Both halves of an ADJACENT pair have to be, or the pair is a real check
    standing next to an informational message rather than a check that
    forgot its bail-out. Two that are NOT, both found this way:

        g_eli_taste():   if (i == tries) ... / if (i > 0) G_ELI_DEBUG(...)
        g_journal_create(): if (md->md_type & GJ_TYPE_DATA) GJ_DEBUG(...)

    The first compares a loop counter with another variable; the second is
    a flag test. Neither can reject anything. The uzip pair

        if (sc->blksz % 512 != 0) { printf; goto e4; }
        if (sc->blksz > MAX_BLKSZ) { printf; }

    is two rejections of a value read off the medium, one of which does not
    reject it.
    """
    cond = " ".join(cond.split())
    for conj in _split_top(cond, ("&&", "||")):
        conj = _peel(conj)
        if lead_lvalue(conj) != subj:
            continue
        if len(_split_top(conj, REL)) > 1:          # x < BOUND
            return True
        if "~" in conj and re.search(r"[&|^]", conj):   # x & ~MASK
            return True
        sides = _split_top(conj, EQ)                # (x % 512) != 0
        if len(sides) == 2:
            lit = [s for s in sides if LIT.match(s.strip())]
            other = [s for s in sides if not LIT.match(s.strip())]
            if lit and other and re.search(r"[%&|^/]|<<|>>", other[0]):
                return True
    return False


ASSIGN = re.compile(r"(?<![=!<>+\-*/%&|^])=(?!=)")


def consumed_after(f: "Func", subj: str, off: int) -> bool:
    """Is `subj' put to work after this point, or only compared and printed?

    A value that is never used again after the check cannot hurt anyone,
    however wrong the check is. `Used' means indexed with, shifted by,
    computed on, or handed to a function that is not a printf - being an
    operand of another comparison does not count.
    """
    tail = strip_diag(f.body[off:])
    q = re.escape(subj)
    if re.search(rf"\[[^][]*{q}[^][]*\]", tail):
        return True
    if re.search(rf"{q}\s*(<<|>>|[-+*/%])|(<<|>>|[-+*/%])\s*{q}", tail):
        return True
    for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", tail):
        # `if (option_length < sizeof(*p))' is not a use of option_length,
        # it is another comparison. Leaving `if' in here kept all five
        # gve_parse_device_option() findings alive through the filter that
        # was written to remove them.
        if m.group(1) in CTRL or m.group(1) in ("sizeof", "nitems",
                                                "howmany", "offsetof"):
            continue
        cp = match_fwd(tail, m.end() - 1)
        if cp < 0:
            continue
        if re.search(rf"\b{q}\b" if subj.isidentifier() else q,
                     tail[m.end():cp]):
            return True
    return False


def scan_adjacent(path, rel, funcs, ctx, lineof, args) -> list[Finding]:
    out = []
    for f in funcs:
        ifs = []
        pos = Positions(f.body)
        for m in re.finditer(r"\bif\s*\(", f.body):
            op = m.end() - 1
            cp = match_fwd(f.body, op)
            if cp < 0:
                continue
            cond = " ".join(f.body[op + 1:cp].split())
            cons, end = consequent(f.body, cp + 1)
            if has_else(f.body, end):
                continue
            ifs.append((m.start(), end, cond, cons, pos.depth(m.start())))
        for a, b in zip(ifs, ifs[1:]):
            if a[4] != b[4]:
                continue
            # Adjacent: nothing but whitespace between the end of one and
            # the start of the next. A statement in between means the
            # author moved on, and the two checks are not a pair.
            between = f.body[a[1]:b[0]]
            if re.search(r"[A-Za-z_0-9;{}]", between):
                continue
            ta, tb = TRANSFER.search(a[3]), TRANSFER.search(b[3])
            if bool(ta) == bool(tb):
                continue
            # The defect is a check written AFTER one that bails out,
            # copying its shape and losing the exit. The other order is a
            # progress message standing in front of a timeout, which is
            # what pqisrc_wait_for_cmnd_complete() has:
            #     if (count % 1000 == 0) DBG_WARN("Waited %d seconds");
            #     if (count >= PQI_QUIESCE_TIMEOUT) return FAILURE;
            if not ta:
                continue
            hot, cold = a, b
            if not diag_only(cold[3]):
                continue
            # `if ((error = copyout(...)) != 0) DPRINTF(...)' is error
            # propagation - sys_msgrcv() returns that same error on the
            # next line - not a bound that forgot to bail out.
            if any(ASSIGN.search(x[2]) for x in (hot, cold)):
                continue
            la, lb = lead_lvalue(hot[2]), lead_lvalue(cold[2])
            if not la or la != lb:
                continue
            if not (rejection_shaped(hot[2], la)
                    and rejection_shaped(cold[2], la)):
                continue
            # The bound has to protect something. In g_uzip_taste() the
            # unbounded sc->blksz goes on to LZ4_compressBound() and
            # malloc(); in gve_parse_device_option() an over-long option
            # is warned about and then never used again, because only the
            # known prefix of the struct is read - the warning is
            # deliberate forward-compatibility, not a missing exit. That
            # one test removed six of the seven ADJACENT findings in
            # sys/dev and kept the one that matters.
            if not consumed_after(f, la, cold[1]):
                continue
            if args.compound_only and "->" not in la and "." not in la:
                continue
            line = lineof(f.body_off + cold[0])
            out.append(Finding(
                path, "ADJACENT", "high", la, "", (f.name, lineof(
                    f.body_off + hot[0])), (f.name, line),
                f"{lineof(f.body_off + hot[0])}: if ({hot[2]}) -> "
                f"{TRANSFER.search(hot[3]).group(1)}\n"
                f"      {line}: if ({cold[2]}) -> prints only",
                None, line))
    return out


# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------

def scan_file(path: str, rel: str, args) -> list[Finding]:
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            raw = fh.read()
    except OSError:
        return []
    if "{" not in raw:
        return []
    blank = blank_comments_and_literals(raw)
    ctx = pp_context(raw)
    scan = blank_pp_lines(blank)
    starts = [0] + [k + 1 for k, c in enumerate(raw) if c == "\n"]

    def lineof(off: int) -> int:
        lo, hi = 0, len(starts) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if starts[mid] <= off:
                lo = mid
            else:
                hi = mid - 1
        return lo + 1

    funcs = find_functions(scan, raw)
    if not funcs:
        return []
    out = []
    if not args.only_adjacent:
        out += scan_siblings(rel, rel, funcs, ctx, lineof, args)
    if not args.only_sibling:
        out += scan_adjacent(rel, rel, funcs, ctx, lineof, args)
    return out


RANK = {"high": 0, "medium": 1, "weak": 2}


def main() -> int:
    ap = argparse.ArgumentParser(
        description="functions that validate a parameter next to functions "
                    "that do not")
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--scope", default="sys/dev",
                    help="subtree to examine (default: sys/dev)")
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 if any high-confidence finding is present")
    ap.add_argument("--window", type=int, default=12,
                    help="how many leading statements count as `entry' for "
                         "the guarded side (default 12)")
    ap.add_argument("--include-pointers", action="store_true",
                    help="also group pointer parameters. Roughly triples "
                         "the hit count and most of the increase is noise; "
                         "see the docstring")
    ap.add_argument("--compound-only", action="store_true",
                    help="ADJACENT: only report when the shared subject is "
                         "a field, not a bare local")
    ap.add_argument("--only-sibling", action="store_true")
    ap.add_argument("--only-adjacent", action="store_true")
    ap.add_argument("--min", choices=("high", "medium", "weak"),
                    default="high",
                    help="lowest severity to print (default high). "
                         "`medium\' is the same analysis without the "
                         "name-pair corroboration; a 22-finding sample of it "
                         "over sys/dev was about nine in ten false, so it is "
                         "for reading one driver, not a subtree")
    ap.add_argument("--top", type=int, default=0,
                    help="print at most this many (0 = all)")
    ap.add_argument("--file", action="append", default=[],
                    help="examine just this file, repeatable; --scope and "
                         "root are ignored")
    args = ap.parse_args()

    files = []
    if args.file:
        files = [(f, f) for f in args.file]
    else:
        root = os.path.abspath(args.root)
        base = os.path.join(root, args.scope)
        if not os.path.isdir(base):
            print(f"no {args.scope} under {args.root}", file=sys.stderr)
            return 2
        for dirpath, dirnames, filenames in os.walk(base):
            dirnames[:] = [d for d in dirnames if d != ".git"]
            for nm in sorted(filenames):
                if nm.endswith(".c"):
                    p = os.path.join(dirpath, nm)
                    files.append((p, os.path.relpath(p, root)))

    findings = []
    for path, rel in files:
        findings += scan_file(path, rel, args)

    lim = RANK[args.min]
    findings = [f for f in findings if RANK[f.sev] <= lim]
    findings.sort(key=lambda f: (RANK[f.sev], f.kind, f.path, f.line))
    shown = findings[:args.top] if args.top else findings

    scope = ", ".join(args.file) if args.file else args.scope
    print(f"scope {scope}: {len(files)} .c file(s)")
    nhigh = sum(1 for f in findings if f.sev == "high")
    print(f"{len(findings)} finding(s), {nhigh} high-confidence\n")
    for f in shown:
        tag = f"{f.kind}/{f.sev}"
        if f.kind == "ADJACENT":
            print(f"{tag:14s} {f.path}:{f.line}")
            print(f"      in {f.unguarded[0]}(), on {f.param}")
            print(f"      {f.detail}")
        elif f.kind == "WEAK":
            print(f"{tag:14s} {f.path}:{f.line}")
            print(f"      {f.ty} {f.param}: checked only where a production "
                  f"kernel has no check")
            print(f"      {f.detail}")
        else:
            pr = f"  [{f.pair}]" if f.pair else ""
            print(f"{tag:14s} {f.path}:{f.line}{pr}")
            print(f"      {f.param}: {f.unguarded[0]}() never tests it; "
                  f"{f.guarded[0]}() does")
            print(f"      {f.detail}")
        print()
    if args.top and len(findings) > args.top:
        print(f"... and {len(findings) - args.top} more\n")

    print("A parameter half a file checks is an oversight; a parameter")
    print("nobody checks is a decision. This reports only the first.")
    if args.gate:
        if nhigh:
            print(f"\nFAIL: {nhigh} high-confidence finding(s)")
            return 1
        print("\nno high-confidence finding.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
