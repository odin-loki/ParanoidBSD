#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Licence compliance gate for the PBSD tree — enforces LICENSING.md.

Checks, in order:

  1. The root licence documents exist and LICENSE is the AGPL-3.0 text.
  2. The vendored upstream trees still carry their own notices.
  3. Nothing under pbsd/ carries a GPL-2.0-only identifier (the KDE
     compatibility route in LICENSING.md 4.3.1 depends on "or later").
  4. Every SPDX identifier under pbsd/ is one LICENSING.md 5 knows about.
  5. The BSD-4-Clause set of LICENSING.md 4.2.1 is a ratchet: it may shrink,
     never grow. Those files are excluded from the AGPL grant.
  6. Upstream headers have not been stripped — the per-identifier file counts
     recorded in NOTICES.md 2 are floors.

Exit 0 clean, 1 on any violation. Run from anywhere:

    python3 tools/check_licences.py [--verbose]
"""
from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"

SPDX = re.compile(rb"SPDX-License-Identifier:\s*([^\r\n*]+)")
# Ports carry the upstream header wherever the original had it, which for the
# libc resolver sources is several hundred lines down — read the whole file.
MAX_BYTES = 4 << 20
SKIP_SUFFIXES = frozenset({".png", ".jpg", ".jpeg", ".gif", ".ico", ".pdf", ".zip", ".gz"})

REQUIRED_DOCS = (
    "LICENSE",
    "LICENSING.md",
    "NOTICES.md",
    "COMMERCIAL-LICENCE.md",
    "LICENSES/BSD-2-Clause.txt",
    "LICENSES/BSD-3-Clause.txt",
)

# Upstream notice files that must survive in the vendored trees. Checked only
# when the tree is present, so sparse checkouts still pass.
UPSTREAM_NOTICES = (
    ("hbsd", "hbsd/src/COPYRIGHT"),
    ("kde", "kde/kwin/LICENSES"),
    ("kde", "kde/frameworks/kconfig/LICENSES"),
)

# LICENSING.md 5. Anything outside this set is unreviewed and fails the gate.
ALLOWED = frozenset({
    "AGPL-3.0-or-later",
    "BSD-1-Clause",
    "BSD-2-Clause",
    "BSD-3-Clause",
    "BSD-4-Clause",          # excluded from the AGPL grant — see BSD4_RATCHET
    "(BSD-3-Clause AND ISC)",
    "0BSD",
    "ISC",
    "MIT",
    "GPL-2.0-or-later",
    "LGPL-2.1-or-later",
    "CDDL-1.0",              # excluded from the AGPL grant — LICENSING.md 4.3.2
})

FORBIDDEN = frozenset({"GPL-2.0-only", "GPL-2.0", "GPL-3.0-only", "AGPL-3.0-only"})

# LICENSING.md 4.2.1. The advertising clause is an added restriction under
# AGPL 7, and the 1999 UC rescission does not reach these holders. The list may
# shrink as files are relicensed upstream or reimplemented; it may never grow.
BSD4_RATCHET = frozenset({
    "pbsd/bin/chio/b0192/oracle.c",
    "pbsd/bin/chio/b0192/port.cppm",
    "pbsd/lib/libc/amd64/gen/b0055/oracle.c",
    "pbsd/lib/libc/amd64/gen/b0055/port.cppm",
    "pbsd/lib/libc/locale/b0147/port.cppm",
    "pbsd/lib/libc/rpc/b0196/oracle.c",
    "pbsd/lib/libc/rpc/b0196/port.cppm",
    "pbsd/lib/libc/rpc/b0196s2/oracle.c",
    "pbsd/lib/libc/rpc/b0196s2/port.cppm",
    "pbsd/lib/libcam/scsi_cmdparse_m/oracle.c",
    "pbsd/lib/libcam/scsi_cmdparse_m/port.cppm",
    "pbsd/lib/libthr/sys/b0234/oracle.c",
    "pbsd/lib/libthr/sys/b0234/port.cppm",
    "pbsd/lib/libthr/thread/b0271/oracle.c",
    "pbsd/lib/libthr/thread/b0271/port.cppm",
    "pbsd/lib/libthr/thread/b0278/oracle.c",
    "pbsd/lib/libthr/thread/b0278/port.cppm",
    "pbsd/sbin/bsdlabel/bsdlabel_m/oracle.c",
    "pbsd/sbin/rcorder/ealloc_m/oracle.c",
    "pbsd/sbin/rcorder/ealloc_m/port.cppm",
    "pbsd/usr.sbin/rtprio/rtprio_m/oracle.c",
    "pbsd/usr.sbin/rtprio/rtprio_m/port.cppm",
})

# NOTICES.md 2. Floors, not equalities: porting more upstream files is fine,
# losing a header is not.
MIN_FILES = {
    "BSD-2-Clause": 557,
    "BSD-3-Clause": 546,
    "ISC": 22,
    "MIT": 8,
    "BSD-1-Clause": 5,
    "(BSD-3-Clause AND ISC)": 4,
    "GPL-2.0-or-later": 3,
    "0BSD": 2,
}


def scan(tree: Path) -> tuple[Counter[str], dict[str, set[str]]]:
    """Map SPDX identifier -> file count, and identifier -> repo-relative paths."""
    counts: Counter[str] = Counter()
    paths: dict[str, set[str]] = {}
    for f in tree.rglob("*"):
        if not f.is_file() or f.suffix.lower() in SKIP_SUFFIXES:
            continue
        try:
            head = f.open("rb").read(MAX_BYTES)
        except OSError:
            continue
        seen = {m.group(1).decode("utf-8", "replace").strip() for m in SPDX.finditer(head)}
        rel = f.relative_to(ROOT).as_posix()
        for ident in seen:
            counts[ident] += 1
            paths.setdefault(ident, set()).add(rel)
    return counts, paths


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--verbose", action="store_true", help="list every identifier found")
    args = ap.parse_args()

    failures: list[str] = []
    notes: list[str] = []

    # 1. Root documents.
    for doc in REQUIRED_DOCS:
        if not (ROOT / doc).exists():
            failures.append(f"missing licence document: {doc}")
    licence = ROOT / "LICENSE"
    if licence.exists():
        text = licence.read_text(encoding="utf-8", errors="replace")
        if "GNU AFFERO GENERAL PUBLIC LICENSE" not in text or "Version 3" not in text:
            failures.append("LICENSE is not the AGPL-3.0 text")

    # 2. Vendored upstream notices, where the tree is checked out.
    for tree, notice in UPSTREAM_NOTICES:
        if (ROOT / tree).is_dir() and not (ROOT / notice).exists():
            failures.append(f"vendored tree {tree}/ present but its notice {notice} is missing")
        elif not (ROOT / tree).is_dir():
            notes.append(f"{tree}/ not checked out — upstream notice check skipped")

    if not PBSD.is_dir():
        failures.append("pbsd/ is missing")
        for f in failures:
            print(f"FAIL {f}")
        return 1

    counts, paths = scan(PBSD)

    # 3. Identifiers with no compatibility route.
    for ident in sorted(FORBIDDEN & counts.keys()):
        for rel in sorted(paths[ident]):
            failures.append(f"{ident} is incompatible with AGPL-3.0 (LICENSING.md 5): {rel}")

    # 4. Unreviewed identifiers.
    for ident in sorted(counts.keys() - ALLOWED - FORBIDDEN):
        failures.append(
            f"unreviewed SPDX identifier {ident!r} in {sorted(paths[ident])[0]} "
            f"({counts[ident]} file(s)) — add it to LICENSING.md 5 first"
        )

    # 5. BSD-4-Clause ratchet.
    found4 = paths.get("BSD-4-Clause", set())
    for rel in sorted(found4 - BSD4_RATCHET):
        failures.append(
            f"new BSD-4-Clause file {rel} — the advertising clause bars it from an "
            f"AGPL binary (LICENSING.md 4.2.1)"
        )
    cleared = BSD4_RATCHET - found4
    if cleared:
        notes.append(
            f"{len(cleared)} BSD-4-Clause file(s) cleared since the ratchet was set — "
            f"drop them from BSD4_RATCHET and NOTICES.md 3.1: {', '.join(sorted(cleared))}"
        )

    # 6. Header-retention floors.
    for ident, floor in sorted(MIN_FILES.items()):
        have = counts.get(ident, 0)
        if have < floor:
            failures.append(
                f"{ident} header count fell from {floor} to {have} — an upstream notice was "
                f"stripped (LICENSING.md 3), or update NOTICES.md 2 with the reason"
            )

    if args.verbose:
        print("SPDX identifiers under pbsd/:")
        for ident, n in counts.most_common():
            print(f"  {n:6d}  {ident}")

    for n in notes:
        print(f"note {n}")
    for f in failures:
        print(f"FAIL {f}")

    if failures:
        print(f"\n{len(failures)} licence violation(s). See LICENSING.md.")
        return 1
    tagged = len({rel for group in paths.values() for rel in group})
    print(f"licences OK — {tagged} SPDX-tagged file(s) under pbsd/, "
          f"{len(counts)} distinct identifier(s), all reviewed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
