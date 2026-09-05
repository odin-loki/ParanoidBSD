#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""What do the hardening knobs default to, and does a running system agree?

tools/ci/show_hardening.sh reports build options - whether PAX_ASLR is
compiled in. That is not the same question as whether
hardening.pax.aslr.status defaults to on, and this project has already been
caught once by exactly that distinction: WITHOUT_MACHDEP_OPTIMIZATIONS was
set, reported as set, and inert.

There are 59 distinct hardening.* sysctls in the kernel and nothing checked
one of them. This extracts each knob and its compiled-in default from the
source, and compares a dump from a booted system against it.

HardenedBSD declares these with its own macros:

  SYSCTL_HBSD_4STATE(pax_aslr_status, pr_hbsd.aslr.status,
      _hardening_pax_aslr, status, ...)

so the node path comes from the third and fourth arguments and the default
from the C initialiser of the first. The four states are

  0 disabled   1 opt-in   2 opt-out   3 force-enabled

and for a hardened system the interesting distinction is 1 against 2: opt-in
means off unless a binary asks, opt-out means on unless a binary asks not to.
A knob silently moving from 2 to 1 across an upstream merge is a mitigation
turning itself off for everything that does not know to ask.

Usage:
  hardening_sysctls.py                 the table, from source
  hardening_sysctls.py --check FILE    FILE is `sysctl hardening` from a
                                       booted system; compare and exit 1 on
                                       any difference
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SYS = ROOT / "hbsd/src/sys"

STATE = {
    "PAX_FEATURE_DISABLED": ("0", "disabled"),
    "PAX_FEATURE_OPTIN": ("1", "opt-in"),
    "PAX_FEATURE_OPTOUT": ("2", "opt-out"),
    "PAX_FEATURE_FORCE_ENABLED": ("3", "forced"),
    "PAX_FEATURE_SIMPLE_DISABLED": ("0", "disabled"),
    "PAX_FEATURE_SIMPLE_ENABLED": ("1", "enabled"),
}

# SYSCTL_HBSD_<n>STATE(var, jail_field, parent_node, leaf, ...)
HBSD = re.compile(
    r"SYSCTL_HBSD_(\d)STATE(?:_GLOBAL)?\s*\(\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*,\s*"
    r"[^,]+,\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*,\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*,",
    re.S)

# plain SYSCTL_INT / SYSCTL_UINT under a hardening node
PLAIN = re.compile(
    r"SYSCTL_(?:U?INT|LONG)\s*\(\s*"
    r"(_hardening[A-Za-z0-9_]*)\s*,\s*[^,]+,\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*,[^;]*?&\s*([A-Za-z_][A-Za-z0-9_]*)",
    re.S)

INIT = re.compile(
    r"^\s*(?:static\s+)?(?:int|u_int|long|pax_state_t)\s+"
    r"{var}\s*=\s*([A-Za-z0-9_]+)\s*;", re.M)


# SYSCTL_NODE(parent, OID_AUTO, name, ...) - the C node variable is the
# parent concatenated with the name, so the tree has to be built from the
# declarations. Splitting the variable on underscores looks like it works
# and gets hardening.pax.disallow_map32bit wrong, because the leaf name has
# an underscore in it and nothing in the variable name says which
# underscores are separators.
NODE = re.compile(
    r"SYSCTL_(?:NODE|DECL)\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,"
    r"(?:\s*OID_AUTO\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*,)?", re.S)


def build_nodes() -> dict[str, str]:
    """C node variable -> dotted sysctl path."""
    nodes = {"": ""}
    pending = []
    for p in SYS.rglob("*.c"):
        try:
            text = p.read_text(errors="replace")
        except OSError:
            continue
        if "_hardening" not in text:
            continue
        for m in NODE.finditer(text):
            parent, name = m.group(1), m.group(2)
            if not name:
                continue
            pending.append((parent, name))
    # The root is declared as SYSCTL_NODE(_kern, OID_AUTO, hardening, ...)
    # or as its own top-level node; seed whatever names itself "hardening".
    for parent, name in pending:
        if name == "hardening":
            nodes[f"{parent}_hardening" if parent != "_kern"
                  else "_hardening"] = "hardening"
    nodes.setdefault("_hardening", "hardening")
    # Resolve the rest by repeated passes, since declaration order is not
    # dependency order across files.
    for _ in range(8):
        for parent, name in pending:
            if parent in nodes and f"{parent}_{name}" not in nodes:
                base = nodes[parent]
                nodes[f"{parent}_{name}"] = f"{base}.{name}" if base else name
    return nodes


NODES: dict[str, str] = {}


def node_to_path(node: str, leaf: str) -> str:
    base = NODES.get(node)
    if base is None:
        # Unresolved. Say so rather than guessing, because a guessed path
        # silently becomes "not present in the dump" at --check time.
        return f"?{node}.{leaf}"
    return f"{base}.{leaf}"


def default_of(text: str, var: str) -> tuple[str, str]:
    m = re.search(INIT.pattern.format(var=re.escape(var)), text, re.M)
    if not m:
        return ("?", "not found in this file")
    tok = m.group(1)
    if tok in STATE:
        return STATE[tok]
    return (tok, "")


def collect() -> list[tuple[str, str, str, str, str]]:
    global NODES
    NODES = build_nodes()
    rows = []
    for p in sorted(SYS.rglob("*.c")):
        try:
            text = p.read_text(errors="replace")
        except OSError:
            continue
        if "hardening" not in text:
            continue
        rel = p.relative_to(SYS).as_posix()
        for m in HBSD.finditer(text):
            nstates, var, node, leaf = m.groups()
            val, meaning = default_of(text, var)
            rows.append((node_to_path(node, leaf), val, meaning,
                         f"{nstates}-state", rel))
        for m in PLAIN.finditer(text):
            node, leaf, var = m.groups()
            val, meaning = default_of(text, var)
            rows.append((node_to_path(node, leaf), val, meaning, "int", rel))
    # deduplicate on the path, keeping the first
    seen = set()
    out = []
    for r in rows:
        if r[0] in seen:
            continue
        seen.add(r[0])
        out.append(r)
    return sorted(out)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", metavar="FILE",
                    help="output of `sysctl hardening` from a booted system")
    args = ap.parse_args()

    rows = collect()
    unresolved = [r for r in rows if r[0].startswith("?")]
    if not args.check:
        print(f"{len(rows)} hardening knobs declared with a compiled-in "
              f"default\n")
        print(f"{'default':>8s} {'kind':>8s}  sysctl")
        for path, val, meaning, kind, src in rows:
            note = f"  ({meaning})" if meaning else ""
            print(f"{val:>8s} {kind:>8s}  {path}{note}")
        print()
        print("A 4-state knob at 2 (opt-out) is on for everything that does")
        print("not ask to be excused; at 1 (opt-in) it is off for everything")
        print("that does not ask for it. That difference is a mitigation")
        print("being on or off for the whole system, and it is one token in")
        print("one C file.")
        print()
        if unresolved:
            print(f"{len(unresolved)} node path(s) could not be resolved from")
            print("the SYSCTL_NODE declarations and are printed with a")
            print("leading ?. They are excluded from --check rather than")
            print("guessed, because a guessed path becomes a silent")
            print("\"not present in the dump\".")
            print()
        print("Nothing verifies these against a running kernel yet, because")
        print("PBSD has not booted. --check takes `sysctl hardening` from a")
        print("booted system and compares.")
        return 0

    live: dict[str, str] = {}
    for line in Path(args.check).read_text(errors="replace").splitlines():
        if ":" not in line:
            continue
        k, _, v = line.partition(":")
        live[k.strip()] = v.strip()

    expected = {r[0]: r[1] for r in rows
                if r[1] != "?" and not r[0].startswith("?")}
    bad = 0
    missing = 0
    for path, want in sorted(expected.items()):
        got = live.get(path)
        if got is None:
            missing += 1
            continue
        if got != want:
            print(f"FAIL {path}: source says {want}, kernel says {got}")
            bad += 1
    print(f"\n{len(expected) - bad - missing} knobs agree, {bad} differ, "
          f"{missing} not present in the dump.")
    if bad:
        print("A knob whose runtime default is not its compiled-in one is")
        print("either a loader tunable in the image or a bug. Both are worth")
        print("knowing; neither should be a surprise.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
