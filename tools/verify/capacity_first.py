#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The capacity, committed before the allocation that is supposed to earn it.

    tstate->size += 4;
    tstate->idx = realloc(tstate->idx, tstate->size * sizeof(*idx));
    if (tstate->idx == NULL)
            return (0);

Two things go wrong on that failure and only one of them is obvious.
The pointer is destroyed -- realloc_self.py is the rule for that.  The
SIZE is also wrong, and stays wrong: `tstate->size' now claims a table
twice the one that exists, so the NEXT call finds its own growth test
false, skips the allocation, and indexes past the end.  The allocation
is never retried, because the structure already believes it happened.

This rule is for the residue realloc_self.py cannot see: the sites where
the POINTER was handled correctly, through a temporary, and the size was
not.  lib/libproc/proc_rtld.c is exactly that --

    phdl->maparrsz *= 2;
    tmp = reallocarray(phdl->mappings, phdl->maparrsz, ...);
    if (tmp == NULL)
            return (-1);
    phdl->mappings = tmp;

-- textbook temporary, textbook check, and a failed grow still leaves
maparrsz naming an array that was never allocated.  Neither clang's
analyser nor a realloc rule reports it.  A reader has to notice that the
two lines are in the wrong order, which is why this is written down.

The fix is always the same reordering: allocate at the NEW size computed
in the expression, and commit the variable after the check.

    tmp = reallocarray(phdl->mappings, phdl->maparrsz * 2, ...);
    if (tmp == NULL)
            return (-1);
    phdl->mappings = tmp;
    phdl->maparrsz *= 2;

WHAT THIS REPORTS

A persistent capacity -- a struct field, `p->n' or `p.n', never a bare
local -- GROWN in place (`+=', `*=', `<<=', `++') within five lines
before an allocation whose size expression mentions it, where a failure
arm exists, leaves the function, and neither restores the variable nor
calls something that ends the process.

Every one of those clauses earns its place:

  grown in place    `x = CONST' before an allocation is the ordinary
                    idiom for "this is how big it will be", and there
                    are four hundred of them in sys alone.  A RUNNING
                    capacity is the one that has a previous value worth
                    going back to.
  a struct field    a local is re-derived on the next call; only state
                    that outlives the frame can be left lying.
  a failure arm     no arm at all means the result is unchecked, which
                    is realloc_self.py's finding, not this one.
  not __dead2       a program that exits loses nothing by losing the
                    count.

WHAT IT DOES NOT REPORT, AND WHY

The restore.  `if (tmp == NULL) { p->n /= 2; return (-1); }' is the
other correct answer and this rule takes it.

contrib/.  Third-party trees, third-party maintainers.

A site being reported is not by itself a defect: the structure may be
about to be destroyed on that path, or a later entry point may reset
both halves together.  Both happen in this tree and both are one level
removed from the site, which is exactly why EXPECTED below records them
rather than leaving the next reader to derive them again.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parents[2] / "hbsd" / "src"

_FIELD = r"[A-Za-z_]\w*(?:\s*(?:->|\.)\s*[A-Za-z_]\w*)+"
GROW = re.compile(r"^\s*(?:(?P<a>" + _FIELD + r")\s*(?:\+=|\*=|<<=)\s*[^=]"
                  r"|\+\+\s*(?P<b>" + _FIELD + r")"
                  r"|(?P<c>" + _FIELD + r")\s*\+\+)")
ALLOC = re.compile(r"\b(?:realloc|reallocarray|malloc|calloc|mallocarray)\s*\(")
NULLT = re.compile(r"==\s*NULL|NULL\s*==|!\s*[A-Za-z_]")
LEAVE = re.compile(r"\b(?:return|goto|break|continue)\b")
DEAD = re.compile(r"\b(?:err|errx|xo_err|xo_errx|abort|exit|panic|_errx|"
                  r"out_of_mem|AbortProgram|fatal|bsdar_errc)\s*\(")

SCOPES = ("lib", "bin", "sbin", "usr.bin", "usr.sbin", "libexec", "sys")


# Every site read and left alone, with the reason. A site not here fails
# --gate. The one that was NOT left alone is lib/libproc/proc_rtld.c,
# written up in docs/security/UB_FINDINGS.md.
EXPECTED: dict[str, str] = {
    "lib/libc/gen/wordexp.c:233":
        "we_askshell() does bump we_wordc and we_nbytes before the two "
        "allocations -- but every error return from it reaches "
        "wordexp(), which calls wordfree(we) before returning, and "
        "wordfree() zeroes both counts and both pointers. The reason is "
        "one level up from the site, which is why it is written here.",
    "lib/libc/net/getservent.c:1332":
        "files_servent() frees st->buffer, doubles st->bufsize and "
        "mallocs -- and on failure returns with buffer NULL. The next "
        "call takes the `if (st->buffer == NULL)' arm at the top of the "
        "same function, which mallocs SERVENT_STORAGE_INITIAL and sets "
        "st->bufsize back to it. Both halves are reset together.",
    "lib/libc/rpc/getrpcent.c:964":
        "files_rpcent(), the same shape and the same self-correcting "
        "entry test, with RPCENT_STORAGE_INITIAL.",
    "sys/dev/gpio/gpiobus.c:357":
        "gpiobus_init_softc() bumps sc_npins past GPIO_PIN_MAX() and "
        "then mallocs sc_pins, returning ENOMEM on failure -- and it "
        "takes TWO facts to make that safe, which is why it is here. "
        "Both callers (gpiobus_attach, ofw_gpiobus_attach) propagate "
        "the error, so newbus tears the device down rather than "
        "running with the softc; and gpiobus_detach guards its walk "
        "with `if (sc->sc_pins)' before indexing by sc_npins. Either "
        "one alone would not be enough.",
    "sys/netipsec/xform_esp.c:386":
        "esp_input(): crp->crp_aad_length is bumped and the malloc "
        "feeds crp->crp_aad; the failure goes to crp_aad_fail, which "
        "falls into crypto_freereq(crp). The request carrying the stale "
        "length is destroyed on that path.",
    "sys/netipsec/xform_esp.c:945":
        "esp_output(), the same pair, the same crp_aad_fail.",
}


def scan(path: Path):
    """(line, variable, source) for each capacity grown before its alloc."""
    lines = path.read_text(errors="replace").split("\n")
    for i, line in enumerate(lines):
        m = GROW.match(line)
        if not m:
            continue
        var = _norm(m.group("a") or m.group("b") or m.group("c"))
        aidx = None
        for j in range(i + 1, min(i + 6, len(lines))):
            if ALLOC.search(lines[j]):
                aidx = j
                break
        if aidx is None:
            continue
        if var not in _norm("".join(lines[aidx:aidx + 4])):
            continue
        arm = None
        for j in range(aidx + 1, min(aidx + 7, len(lines))):
            if NULLT.search(lines[j]) and LEAVE.search("".join(lines[j:j + 4])):
                arm = "".join(lines[j:j + 5])
                break
        if arm is None:
            continue                    # unchecked: realloc_self.py's find
        if DEAD.search(arm):
            continue                    # the process is going away
        if var in _norm(arm):
            continue                    # restored
        yield i + 1, var, line.strip()


def _norm(s: str) -> str:
    return re.sub(r"\s+", "", s)


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append", default=[],
                    help="path prefix under hbsd/src, repeatable")
    ap.add_argument("--gate", action="store_true",
                    help="exit non-zero on a site not in EXPECTED")
    args = ap.parse_args()

    hits = unexpected = 0
    for scope in (args.scope or SCOPES):
        base = SRC / scope
        if not base.exists():
            continue
        files = [base] if base.is_file() else sorted(base.rglob("*.c"))
        for p in files:
            rel = p.relative_to(SRC).as_posix()
            if "/contrib/" in "/" + rel or rel.startswith("contrib/"):
                continue
            for line, var, src in scan(p):
                hits += 1
                key = "%s:%d" % (rel, line)
                known = key in EXPECTED
                if not known:
                    unexpected += 1
                print("%s  %s%s" % (key, var,
                                    "  [expected]" if known else ""))
                print("        %s" % src[:100])

    print("\n%d site(s), %d not on the record" % (hits, unexpected),
          file=sys.stderr)
    if args.gate and unexpected:
        print("\nEach is a persistent capacity that a failed allocation "
              "leaves claiming\nmemory nobody allocated. The next growth "
              "test reads it and is wrong.\nAllocate at the new size, "
              "commit the variable after the check -- or write\ndown why "
              "the stale value cannot be read.", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
