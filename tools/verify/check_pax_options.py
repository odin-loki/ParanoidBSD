#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Every call to a feature-gated PaX function must be reachable only when
that feature is compiled in.

sys/conf/files gates each PaX feature separately:

    hardenedbsd/hbsd_pax_common.c    optional pax
    hardenedbsd/hbsd_pax_aslr.c      optional pax pax_aslr
    hardenedbsd/hbsd_pax_hardening.c optional pax pax_hardening

so `options PAX` without a given feature must still link. It did not,
and finding out cost three boot runs of forty minutes each:

    run 52  pax_kmod_load_disabled, pax_control_extattr_kmod,
            pax_enforce_tpe, pax_harden_tty
    run 54  pax_disallow_map32bit_active

Twice I answered "are there more?" by reading some call sites and
generalising - "five of them, checked one at a time", then "the other 34
are called only from sites that are themselves #ifdef'd". Both were
wrong, and the second was wrong *after* the first had already been
proved wrong the same way. This script is what asking the question
properly looks like: it enumerates rather than samples.

THE RULE

A call to a function whose defining file needs option X is fine when any
of these holds:

  1. an enclosing #ifdef in the caller mentions X;
  2. the CALLER's own file is gated on X by sys/conf/files, so it is not
     compiled when X is off;
  3. sys/sys/pax.h declares the function under `#ifdef X` with an `#else`
     that #defines a no-op - the idiom already used for the five
     *_init_prison functions and now for the ones the linker named.

Anything else is a kernel that does not link in some supported
configuration. Unlike the bool-outside-_KERNEL rule attempted in
docs/security/UB_FINDINGS.md, this one is decidable from the tree: the
gate is in sys/conf/files, the guard is in the preprocessor, and the
stub is in one header.
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

COND = re.compile(r"#\s*(if|ifdef|ifndef|elif)\b(.*)")
ENDIF = re.compile(r"#\s*endif\b")
ELSE = re.compile(r"#\s*else\b")


def file_gates() -> dict[str, set[str]]:
    out: dict[str, set[str]] = {}
    for ln in (SRC / "sys/conf/files").read_text().splitlines():
        m = re.match(r"(\S+\.c)\s+optional\s+(.+?)\s*$", ln)
        if m:
            opts = {o.upper() for o in m.group(2).split()} - {"PAX"}
            if opts:
                out[m.group(1)] = opts
    return out


def gated_functions(gates: dict[str, set[str]]) -> dict[str, str]:
    """pax function -> the option its defining file needs."""
    res = subprocess.run(
        ["grep", "-rn", "-E", r"^pax_[a-z0-9_]+\(", "sys/hardenedbsd/"],
        cwd=SRC, capture_output=True, text=True).stdout
    out: dict[str, str] = {}
    for ln in res.splitlines():
        f, _, rest = ln.split(":", 2)
        need = gates.get(f[len("sys/"):], set())
        if need:
            out[rest.split("(")[0]] = sorted(need)[0]
    return out


def stubbed(fns: dict[str, str]) -> set[str]:
    """Functions pax.h declares under #ifdef X with an #else no-op."""
    text = (SRC / "sys/sys/pax.h").read_text()
    ok: set[str] = set()
    for m in re.finditer(r"#ifdef\s+(PAX_\w+)(.*?)#else(.*?)#endif", text, re.S):
        opt, guarded, alt = m.group(1), m.group(2), m.group(3)
        for fn, need in fns.items():
            if need == opt and re.search(rf"\b{fn}\s*\(", guarded) \
               and re.search(rf"define\s+{fn}\b", alt):
                ok.add(fn)
    return ok


def main() -> int:
    gates = file_gates()
    fns = gated_functions(gates)
    have_stub = stubbed(fns)
    if not fns:
        print("FAIL  found no feature-gated pax functions - this check")
        print("      cannot have passed, so it fails instead.")
        return 1

    bad = []
    for cf in sorted(SRC.glob("sys/**/*.c")):
        rel = cf.relative_to(SRC).as_posix()[len("sys/"):]
        own = gates.get(rel, set())
        try:
            lines = cf.read_text(errors="replace").splitlines()
        except OSError:
            continue
        blob = "\n".join(lines)
        if not any(f"{fn}(" in blob for fn in fns):
            continue
        stack: list[str] = []
        for i, line in enumerate(lines, 1):
            s = line.strip()
            m = COND.match(s)
            if m and m.group(1) != "elif":
                stack.append(m.group(2).strip())
            elif ELSE.match(s) and stack:
                stack[-1] = "!(" + stack[-1] + ")"
            elif ENDIF.match(s) and stack:
                stack.pop()
            if s.startswith(("*", "/*", "//")):
                continue
            for fn, need in fns.items():
                if not re.search(rf"\b{fn}\s*\(", line):
                    continue
                if fn in have_stub or need in own \
                   or any(need in g for g in stack):
                    continue
                bad.append((cf.relative_to(SRC).as_posix(), i, fn, need,
                            " / ".join(stack[-2:]) or "(no #ifdef)"))

    for f, i, fn, need, ctx in bad:
        print(f"FAIL  {f}:{i}")
        print(f"      {fn}() needs {need}; guarded by: {ctx}")

    if bad:
        print(f"\n{len(bad)} call site(s) that do not link when their")
        print("feature is off. Either guard the call, or give the")
        print("function an #ifdef/#else no-op in sys/sys/pax.h beside")
        print("the ones that already have it.")
        return 1

    print(f"{len(fns)} feature-gated PaX functions, "
          f"{len(have_stub)} with an #else no-op in pax.h:")
    print("every call site is reachable only when its feature is on.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
