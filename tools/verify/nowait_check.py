#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""M_NOWAIT allocations whose result is used without a NULL check.

WHY THIS EXISTS AND THE ANALYSER DOES NOT COVER IT

clang's analyser found ng_netflow.c:414 and stopped. The same file has the
identical bug at :388 and :556, and the analyser reported neither, because
it explores paths and stops at the first defect on each - three instances
of one mistake in one file came back as one finding. A pattern check finds
all three by construction.

M_WAITOK cannot fail; M_NOWAIT can, and returns NULL. The kernel is full
of both and the difference is one token.

WHAT THIS IS NOT

It is a LINT over lines, not a compiler, and each of its false-positive
classes was found by reading output that looked like a hundred bugs:

  `if ((p = malloc(..., M_NOWAIT)) == NULL)`   the test compares the whole
        ASSIGNMENT, so the variable name is nowhere near the `== NULL`.
        Sixty of the first eighty "findings" were this.
  `if (m)` / `if (m && ...)`                   a plain truth test is a NULL
        test. Leaving it out called sys/kern/uipc_mbuf.c and
        sys/netinet/igmp.c bugs; both are correct.
  `sizeof(p->field)`                           not an evaluation, so not a
        dereference. sys/vm/uma_core.c:1252 is this.
  a condition split across lines                the allocation line begins
        mid-expression, so "does this line start with if" says no.
        sys/netinet/ip_icmp.c:894 is this.

So this REPORTS. It is not a gate, because a lint that cannot parse C
cannot carry a build. Read every hit; the ones that survive reading are
worth fixing, and four did.
"""
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

# Allocators that take a wait flag and can return NULL with M_NOWAIT.
ALLOC = re.compile(
    r'(?P<var>[A-Za-z_]\w*)\s*=\s*'
    r'(?P<fn>malloc|mallocarray|malloc_domainset|malloc_domainset_aligned'
    r'|uma_zalloc|uma_zalloc_arg|uma_zalloc_domain|contigmalloc'
    r'|m_get|m_getcl|m_gethdr|m_getjcl)\s*\(')
# ng_message.h:402 - `if ((rsp) == NULL) break;` leaves rsp NULL and the
# macro's callers each have to notice.
MK = re.compile(r'NG_MKRESPONSE\(\s*(?P<var>\w+)')

COND_START = ("if (", "if(", "} else if", "else if", "while (", "while(",
              "for (", "for(")


def _uncomment(lines: list[str]) -> list[str]:
    """Blank out comment text, keeping line count and line lengths.

    Not cosmetic: this file's own fix to sys/fs/p9fs/p9_protocol.c explains
    the bug in a comment that says `&wnames[i]`, and the scanner read its
    own explanation as the dereference and reported the fixed line as still
    broken.

    Done on the joined text with one regex rather than per character - the
    per-character version took over two minutes across sys/.
    """
    text = "\n".join(lines)
    def blank(m: re.Match) -> str:
        return re.sub(r"[^\n]", " ", m.group(0))
    text = re.sub(r"/\*.*?\*/|//[^\n]*", blank, text, flags=re.S)
    return text.split("\n")


def _in_condition(lines: list[str], i: int, back: int = 4) -> bool:
    """Is line i a continuation of an if/while condition above it?

    sys/netinet/ip_icmp.c:894 is `(opts = m_gethdr(M_NOWAIT, MT_DATA))) {`
    - the allocation is in the second half of a condition that opened two
      lines earlier, so "does this line start with if" says no and the
      value is tested all the same.
    """
    for k in range(i - 1, max(-1, i - 1 - back), -1):
        head = lines[k].lstrip()
        if not head.startswith(COND_START):
            continue
        depth = 0
        for line in lines[k:i]:
            depth += line.count("(") - line.count(")")
        if depth > 0:
            return True
    return False


def sites(lines: list[str], ahead: int = 10):
    """(line, var, allocation, first use) for each unchecked allocation."""
    lines = _uncomment(lines)
    out = []
    for i, line in enumerate(lines):
        m = ALLOC.search(line) or MK.search(line)
        if not m:
            continue
        stmt = " ".join(x.strip() for x in lines[i:i + 4])
        if "M_NOWAIT" not in stmt or "M_WAITOK" in stmt:
            continue
        # Inside an if/while condition: the value is being tested.
        if (line.lstrip().startswith(COND_START)
                or _in_condition(lines, i)
                or re.search(r'\)\s*(==|!=)\s*NULL', stmt)
                or re.search(r'if\s*\(\s*!', stmt)):
            continue
        var = m.group("var")
        # `mhead = mtail = m_getcl(...)` then `if (mhead == NULL)`: the
        # check is on the OTHER name. sys/dev/netmap/if_ptnet.c:1760 is
        # this, and it is correct code.
        names = re.findall(r'([A-Za-z_]\w*)\s*=', line[:m.start("var") + 1]
                           + line[m.start("var"):m.end("var")] + " =")
        alias = [n for n in names if n != var]
        v = re.escape(var)
        checked = deref = None
        for j, w in enumerate(lines[i:i + 1 + ahead]):
            if checked is None and any(
                    re.search(
                        rf'{re.escape(nm)}\s*==\s*NULL'
                        rf'|!\s*\(?\s*{re.escape(nm)}\b'
                        rf'|{re.escape(nm)}\s*!=\s*NULL'
                        rf'|{re.escape(nm)}\s*==\s*0\b'
                        rf'|\b(if|while)\s*\(\s*{re.escape(nm)}\s*'
                        rf'(\)|&&|\?)', w)
                    for nm in [var, *alias]):
                checked = j
            if j and deref is None:
                # sizeof(p->x) does not evaluate p.
                stripped = re.sub(r'sizeof\s*\([^)]*\)', '', w)
                if re.search(rf'\b{v}\s*(->|\[)', stripped):
                    deref = j
        if deref is not None and (checked is None or checked > deref):
            out.append((i + 1, var, lines[i].strip(),
                        lines[i + deref].strip()))
    return out


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append",
                    help="limit to paths under hbsd/src starting with this "
                         "(repeatable). Default: sys")
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 on any finding. The tree is at zero, so "
                         "this catches NEW ones; a false positive is fixed "
                         "by teaching sites() a fifth class, not by an "
                         "allowlist that would hide the next real one.")
    ap.add_argument("--ahead", type=int, default=10,
                    help="lines after the allocation to look in (default 10)")
    args = ap.parse_args()
    scopes = args.scope or ["sys"]

    hits = []
    for scope in scopes:
        base = SRC / scope
        files = ([base] if base.is_file()
                 else sorted(base.rglob("*.c")) if base.is_dir() else [])
        for f in files:
            try:
                lines = f.read_text(errors="replace").splitlines()
            except OSError:
                continue
            rel = f.relative_to(SRC).as_posix()
            for ln, var, alloc, use in sites(lines, args.ahead):
                hits.append((rel, ln, var, alloc, use))

    print(f"{len(hits)} M_NOWAIT allocation(s) used without a NULL check\n")
    by = collections.Counter(h[0].rsplit("/", 1)[0] for h in hits)
    for d, n in by.most_common():
        print(f"  {n:3d}  {d}")
    print()
    for rel, ln, var, alloc, use in hits:
        print(f"{rel}:{ln}  ({var})")
        print(f"    {alloc[:78]}")
        print(f"    -> {use[:74]}")
    if hits:
        print("\nRead each one. The four false-positive classes this DOES")
        print("know about are in the docstring; a fifth belongs there too,")
        print("taught to sites(), rather than in an allowlist.")
    if args.gate and hits:
        print(f"\nFAIL  {len(hits)} unchecked M_NOWAIT allocation(s). The")
        print("      tree was at zero when this gate was added.")
        return 1
    if args.gate:
        print("no unchecked M_NOWAIT allocation")
    return 0


if __name__ == "__main__":
    sys.exit(main())
