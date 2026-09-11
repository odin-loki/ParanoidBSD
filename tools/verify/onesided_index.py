#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""An array subscript bounded above and not below.

Three of these turned up in one day and they are the same shape wearing
different clothes:

    systat(1) get_tbl_ptr()
        /* If our index is out of range, default to auto-scaling. */
        idx = scale < SC_AUTO ? scale : SC_AUTO;
        ...
        return (&convtbl[idx]);

    libc querylocale()
        int type = ffs(mask & ~LC_VERSION_MASK) - 1;
        if (type >= XLC_LAST)
                return (NULL);
        ...
        if (loc->components[type])

    libcalendar easterodn()
        dt.d = mc[y % 19];

The first two write the bound as ONE comparison for an index that has
two sides, and the comment in the first says so out loud -- "if our
index is out of range" -- while the code checks one end.  The third
subscripts with `%', which in C keeps the sign of the dividend, so a
negative year indexes backwards off the front of the table.

Both shapes read memory before the array and, in two of the three, hand
what they find back to the caller as a pointer.

WHAT THIS REPORTS

Two rules, deliberately narrow, because a lint that reports every
subscript reports nothing:

  MOD       arr[V % C] where V is a SIGNED integer in scope. Unsigned
            operands are not reported, and neither is a V the enclosing
            function pins non-negative.
  ONESIDED  arr[V] where V is a signed local or parameter, the function
            compares V against an upper bound, and nowhere compares it
            against a lower one.

WHAT IT DOES NOT REPORT, AND WHY

A loop counter. `for (i = 0; i < n; i++) a[i]' is the most common shape
in the tree and it is never this defect: the initialiser pins the
variable at zero and nothing else assigns it. So a variable assigned a
non-negative constant, and otherwise only incremented, is skipped.

An unsigned index, of any width. The whole defect is the sign.

A subscript on a variable this cannot find a declaration for. Guessing
would put the tree's most common shapes in the report and the report
would stop being read -- which is the failure every checker in this
directory is written against.

ONE OF THE TWO GATES, AND WHY ONLY ONE

--gate fails on a MOD site not on the record in EXPECTED below, and does
NOT fail on ONESIDED. That is a measurement, not a preference. Over the
whole tree MOD reports 8 sites, every one read and written down.
ONESIDED reports 264 -- down from 1,109 over four rounds of tightening
-- and they are dominated by internal contracts: a static helper whose
two callers both pass 0 or 1, a driver's ring index, a parameter that a
comment three functions away pins. Each is a reading, not a defect, and
a gate demanding 264 readings before the next commit is a gate somebody
turns off.

So ONESIDED prints and does not fail. It is the same call
noreturn_check.py makes about its 673, for the same reason: a report
nobody finishes reading is a report that has stopped working.

It is a lint, not a proof.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import noreturn_check                                    # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

# Signed integer types a subscript can go negative in. Deliberately not
# a catch-all: a name this does not know is not reported, because a
# wrong guess about signedness is a wrong report.
SIGNED = {
    "char",             # signed on every architecture this tree targets
    "signed char", "short", "short int", "signed short",
    "int", "signed", "signed int", "long", "long int", "signed long",
    "long long", "long long int", "signed long long",
    "int8_t", "int16_t", "int32_t", "int64_t", "intmax_t", "intptr_t",
    "ptrdiff_t", "ssize_t", "off_t", "pid_t", "id_t", "key_t",
    "daddr_t", "quad_t", "register_t", "time_t", "suseconds_t",
    "clock_t", "lwpid_t", "int_fast8_t", "int_fast16_t", "int_fast32_t",
    "int_fast64_t", "int_least8_t", "int_least16_t", "int_least32_t",
    "int_least64_t",
}

_TYPE = r"(?:const\s+|volatile\s+|register\s+|static\s+)*"
# `int foo', `int32_t foo', `long long foo' -- a declaration or a
# parameter. The type is captured whole so SIGNED can be consulted.
DECL_RE = re.compile(
    _TYPE + r"\b((?:signed\s+|unsigned\s+)?"
    r"(?:long\s+long|long|short|char|int|__?[A-Za-z_]\w*|[A-Za-z_]\w*_t)"
    r"(?:\s+int)?)\s+([A-Za-z_]\w*)\s*(?:[,;)=]|$)")

SUB_MOD_RE = re.compile(r"\[\s*([A-Za-z_]\w*)\s*%\s*[^\]]+\]")
SUB_VAR_RE = re.compile(r"\[\s*([A-Za-z_]\w*)\s*\]")

# A comparison that puts a floor under V. `V == -1' and `V != -1' count:
# rejecting the sentinel is how most of this tree spells the floor.
def _floor_re(v: str) -> re.Pattern:
    v = re.escape(v)
    return re.compile(
        r"\b%s\s*(?:<\s*0|>=\s*0|>\s*-\s*1|==\s*-\s*1|!=\s*-\s*1|<=\s*-\s*1)"
        # `assert(0 < i && i <= m->g->nsub)' -- regex/engine.c, and the
        # author stating the bound is a bound. Both directions of the
        # zero comparison: `0 < V' is a floor and so is `0 > V' as a
        # rejection test.
        r"|0\s*(?:>|<=|<|>=)\s*\b%s\b|-\s*1\s*(?:<|>=|==|!=)\s*\b%s\b"
        # `if ((scale = get_scale(args)) != -1)' -- ifcmds.c, and the
        # commonest way this tree spells a floor. The name and the
        # sentinel are on the same statement with the call between
        # them, so a rule anchored on the name alone does not see it.
        r"|\b%s\b\s*=[^;{]{0,60}?(?:!=|==)\s*-\s*1"
        % (v, v, v, v))


def _two_sided_re(v: str) -> re.Pattern:
    """One guard that tests V against a low bound AND a high one.

    setlocale() spells its floor with a NAME, not a zero:

        if (category < LC_ALL || category >= _LC_LAST) {
                errno = EINVAL;
                return (NULL);
        }

    LC_ALL is 0, but the checker cannot see that, and a rule that
    treated `V < NAME' as a floor would swallow every upper-bound test
    in the tree. What it CAN see is the shape: one expression comparing
    V against two different bounds in opposite directions. Every
    correctly guarded index in libc's locale code is written this way,
    and it is the difference between reporting three real sites and
    reporting fifteen.
    """
    v = re.escape(v)
    lo_hi = (r"\b%s\s*<=?\s*[^&|;]{1,40}(?:\|\||&&)[^;{]{0,40}?\b%s\s*>=?"
             % (v, v))
    hi_lo = (r"\b%s\s*>=?\s*[^&|;]{1,40}(?:\|\||&&)[^;{]{0,40}?\b%s\s*<=?"
             % (v, v))
    return re.compile("(?:%s)|(?:%s)" % (lo_hi, hi_lo), re.S)


def _ceiling_re(v: str) -> re.Pattern:
    """A comparison that puts a ceiling over V -- or an assignment that
    clamps one onto it.

    systat's get_tbl_ptr() never compares its INDEX at all:

        idx = scale < SC_AUTO ? scale : SC_AUTO;

    The ceiling is on the source expression and lands in idx by way of
    the ternary.  A rule anchored on the index variable saw no bound
    anywhere and filed it under "bounded nowhere", which is a different
    report -- so it missed the defect it was written for, which is how
    this line came to be tested.
    """
    v = re.escape(v)
    return re.compile(
        r"\b%s\s*(?:<|<=|>|>=)\s*[A-Za-z_(]"
        r"|\b%s\b\s*=[^;{]{0,80}?(?:<|<=|>|>=)[^;{]{0,40}?[?:]"
        % (v, v))


def _pinned_re(v: str) -> re.Pattern:
    """A loop counter, or a variable pinned at a non-negative literal.

    `for (i = ndx + 2; i <= n; i += 2)' is a loop induction variable
    whose initialiser is not a literal, and hash_page.c's __delpair()
    is full of them.  Requiring a literal reported every one.  What
    makes it not this defect is the `for', not the constant: the
    initialiser, the test and the step are all in view on one line and
    the bound is the loop's own.
    """
    v = re.escape(v)
    return re.compile(
        r"\b%s\s*=\s*(?:0|[1-9]\d*)\s*[;,)]"
        r"|\bfor\s*\(\s*(?:[A-Za-z_]\w*\s+)?%s\s*=" % (v, v))


def declared(lines, body, end, proto_line):
    """({name: type} for everything in scope, {name} for parameters).

    The prototype's own line and the two above it carry the parameters
    when KNF puts the return type on its own line; the body's leading
    lines carry the locals.  The two are kept apart because the
    parameters are what makes a site worth reporting -- see origins().
    """
    out, params = {}, set()
    for k in range(max(0, proto_line - 2), body):
        for m in DECL_RE.finditer(lines[k]):
            ty = re.sub(r"\s+", " ", m.group(1)).strip()
            out.setdefault(m.group(2), ty)
            params.add(m.group(2))
    for k in range(body, min(end, body + 40)):
        for m in DECL_RE.finditer(lines[k]):
            ty = re.sub(r"\s+", " ", m.group(1)).strip()
            out.setdefault(m.group(2), ty)
    return out, params


def from_outside(v: str, params: set, text: str) -> bool:
    """Does V's value come from outside this function?

    The whole rule, and the reason the report is 23 lines instead of
    1,109.  All three defects this was written after take their index
    from beyond the function: systat's from a parameter through a
    ternary, querylocale's from ffs(), easterodn's straight from its
    parameter.  A local computed from other locals is the tree's
    commonest shape and it is almost never this bug -- the bound is
    usually two lines up in a form no regex reads.

    So: V is a parameter, or V is assigned from an expression that
    mentions a parameter or calls something.
    """
    if v in params:
        return True
    asg = re.compile(r"\b%s\b\s*=\s*([^;{}]{0,120})" % re.escape(v))
    for m in asg.finditer(text):
        rhs = m.group(1)
        if re.search(r"[A-Za-z_]\w*\s*\(", rhs):      # a call
            return True
        for p in params:
            if re.search(r"\b%s\b" % re.escape(p), rhs):
                return True
    return False


def scan(path: Path):
    """Yield (kind, line_no, function, index, type, text)."""
    try:
        raw = path.read_text(errors="surrogateescape")
    except OSError:
        return
    lines = noreturn_check.strip_noise(raw).split("\n")
    orig = raw.split("\n")
    # One report per (function, variable). fetch_bitmap() subscripts
    # with the same `ndx' five times and _collate_lookup() with the
    # same `which' five more; five lines saying one thing is four lines
    # of a report nobody finishes reading.
    seen = set()
    for name, proto, body, end in noreturn_check.definitions(lines):
        decl, params = declared(lines, body, end, proto)
        text = "\n".join(lines[body:end])
        for k in range(body, end):
            s = lines[k]
            for m in SUB_MOD_RE.finditer(s):
                v = m.group(1)
                ty = decl.get(v)
                if (ty in SIGNED and from_outside(v, params, text)
                        and ("MOD", name, v) not in seen):
                    seen.add(("MOD", name, v))
                    yield ("MOD", k + 1, name, v, ty,
                           orig[k].strip() if k < len(orig) else s.strip())
            for m in SUB_VAR_RE.finditer(s):
                v = m.group(1)
                ty = decl.get(v)
                if ty not in SIGNED:
                    continue
                if _pinned_re(v).search(text):
                    continue                    # a loop counter
                if _floor_re(v).search(text):
                    continue                    # bounded below somewhere
                if _two_sided_re(v).search(text):
                    continue                    # one guard, both bounds
                if not _ceiling_re(v).search(text):
                    continue                    # bounded nowhere: not this
                if not from_outside(v, params, text):
                    continue                    # a local from locals
                if ("ONESIDED", name, v) in seen:
                    continue
                seen.add(("ONESIDED", name, v))
                yield ("ONESIDED", k + 1, name, v, ty,
                       orig[k].strip() if k < len(orig) else s.strip())


# Every MOD site read and left alone, with the reason. A site not here
# fails --gate, which is the point: a new one has to be looked at.
#
# The one that was NOT left alone is strsignal(), which is why this
# checker exists in the tree rather than in a scratch directory:
#
#     signum = num;
#     if (num < 0)
#             signum = -signum;
#     ...
#     *t++ = "0123456789"[signum % 10];
#
# strsignal(3) takes a plain int and that is the arm handling every
# value outside the signal range, so strsignal(INT_MIN) reached it.
# Negating the most negative int is undefined, and it does not even come
# out positive -- signum stayed negative and the subscript read off the
# front of the string literal, into the buffer strsignal() returns. The
# file had carried an `XXX: negative num ?' above the definition for
# years. strerror()'s errstr() had the same line and the same fix.
EXPECTED: dict[str, str] = {
    "usr.bin/number/number.c:261":
        "pfract()'s len is a fraction-digit count derived from strlen "
        "of the caller's string; it cannot be negative.",
    "usr.bin/pr/pr.c:1420":
        "addnum()'s line is pr(1)'s own line counter, which starts at "
        "one and only increases.",
    "usr.sbin/jail/command.c:854":
        "add_proc()'s pid comes from fork(2), which never hands back a "
        "negative pid on the parent's success path.",
    "usr.sbin/jail/command.c:909":
        "find_proc(), same hash and the same pid.",
    "usr.sbin/lpr/common_source/displayq.c:620":
        "prank()'s n is a queue position; zero is handled two lines "
        "above and the rest are counted up from one.",
    "sys/dev/hyperv/netvsc/if_hn.c:5198":
        "hn_tx_ring_create()'s id is a ring index the attach loop "
        "counts from zero.",
    "sys/dev/qat/qat_common/adf_cfg_section.c:115":
        "process_num is parsed by compat_strtoul(), whose result is "
        "unsigned and checked before it is used.",
    "sys/dev/sound/pcm/buffer.c:310":
        "sndbuf_clear()'s i is sndbuf_getfreeptr(), an unsigned offset "
        "already less than bufsize.",
}


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append", default=[],
                    help="path prefix under hbsd/src, repeatable")
    ap.add_argument("--gate", action="store_true",
                    help="exit non-zero on a site not in EXPECTED")
    args = ap.parse_args()

    scopes = args.scope or ["lib", "bin", "sbin", "usr.bin", "usr.sbin",
                            "libexec", "sys"]
    hits, unexpected, advisory = 0, 0, 0
    for scope in scopes:
        base = SRC / scope
        if base.is_file():
            files = [base]
        else:
            files = sorted(p for ext in ("*.c", "*.cpp")
                           for p in base.rglob(ext))
        for p in files:
            rel = p.relative_to(SRC).as_posix()
            if "/contrib/" in "/" + rel or rel.startswith("contrib/"):
                continue
            for kind, ln, fn, v, ty, src in scan(p):
                hits += 1
                key = "%s:%d" % (rel, ln)
                known = key in EXPECTED
                if kind == "MOD" and not known:
                    unexpected += 1
                if kind != "MOD":
                    advisory += 1
                print("%-8s %s:%d  %s()  %s (%s)%s"
                      % (kind, rel, ln, fn, v, ty,
                         "" if not known else "  [expected]"))
                print("         %s" % src[:100])

    print("\n%d site(s): %d MOD not on the record, %d ONESIDED advisory"
          % (hits, unexpected, advisory), file=sys.stderr)
    #
    # ONLY the MOD rule gates, and the reason is a measurement rather
    # than a preference. Over the whole tree MOD reports 8 sites, every
    # one read and on the record above. ONESIDED reports 264 -- after
    # four rounds of tightening that took it from 1,109 -- and they are
    # dominated by internal contracts: a static helper whose two callers
    # both pass 0 or 1, a driver's ring index, a parameter a comment
    # three functions away pins. Each is a reading, not a defect, and a
    # gate that demands 264 readings before the next commit is a gate
    # somebody turns off.
    #
    # So ONESIDED is advisory: it prints, it does not fail. That is the
    # same call noreturn_check.py makes about its 673, and for the same
    # reason -- a report nobody finishes reading is a report that has
    # stopped working.
    if args.gate and unexpected:
        print("A subscript by `V %% C' with V signed reads off the front "
              "of the array when V is negative.\nRead it, then fix it or "
              "put it in EXPECTED with the reason.", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
