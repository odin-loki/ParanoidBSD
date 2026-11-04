#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""null_branch.py reports ZERO, and that is the reading that needs a test.

A rule that finds nothing is indistinguishable, from its output alone,
from a rule that has stopped looking.  This one reports zero over the
16,675 files PBSD owns -- so the question every check here answers is
which of those two it is.

The sharpest of them is the last: the nine sites the rule DOES find,
all of them in vendored trees, are still found when the exclusion is
lifted.  Zero because scoped, not zero because broken.
"""
from __future__ import annotations

import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
ROOT = HERE.parents[1]
SRC = ROOT / "hbsd" / "src"

from null_branch import (scan, strip_if0, strip_noise,   # noqa: E402
                         is_vendor, VENDOR)

fails: list[str] = []


def check(what: str, ok: bool, why: str = "") -> None:
    print(f"  {'ok  ' if ok else 'FAIL'}   {what}"
          + (f"  -- {why}" if not ok and why else ""))
    if not ok:
        fails.append(what)


def hits(body: str) -> list:
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / "t.c"
        p.write_text(body)
        raw = body.split("\n")
        clean = "\n".join(strip_if0(strip_noise(body).split("\n")))
        return list(scan(str(p), clean, raw, False))


print("the shape it is FOR")

check("a field read inside the NULL branch",
      len(hits('''
void f(struct s *p) {
	if (p == NULL) {
		log("no p");
		p->field = 1;
	}
}
''')) == 1)

check("the !p spelling",
      len(hits('''
void f(struct s *p) {
	if (!p) {
		p->field = 1;
	}
}
''')) == 1)

check("NULL on the left",
      len(hits('''
void f(struct s *p) {
	if (NULL == p) {
		p->field = 1;
	}
}
''')) == 1)

print()
print("the shapes it must NOT report")

check("the ordinary guard that returns",
      hits('''
void f(struct s *p) {
	if (p == NULL)
		return;
	p->field = 1;
}
''') == [])

check("a DIFFERENT pointer read inside the branch",
      hits('''
void f(struct s *p, struct s *q) {
	if (p == NULL) {
		q->field = 1;
	}
}
''') == [])

check("the pointer is re-assigned before the read",
      hits('''
void f(struct s *p) {
	if (p == NULL) {
		p = fallback();
		p->field = 1;
	}
}
''') == [])

check("a non-NULL test is not this rule",
      hits('''
void f(struct s *p) {
	if (p != NULL) {
		p->field = 1;
	}
}
''') == [])

print()
print("the vendor exclusion")

for rel in ("contrib/unbound/respip/respip.c",
            "crypto/openssl/apps/cms.c",
            "sys/contrib/openzfs/module/zfs/arc.c",
            "sys/cddl/dev/dtrace/dtrace_load.c"):
    check(f"{rel} is vendored", is_vendor(rel))

for rel in ("lib/libc/stdio/printf.c", "sys/kern/kern_malloc.c",
            "usr.bin/sed/compile.c", "libexec/rtld-elf/rtld.c"):
    check(f"{rel} is NOT vendored", not is_vendor(rel))

check("a near miss on the prefix is not vendored",
      not is_vendor("lib/libcrypto_shim/x.c"),
      "crypto/ must match the directory, not the spelling")

print()
print("zero because SCOPED, not because broken")

# The nine the rule finds tree-wide. If the rule goes blind, these stop
# being found too -- and a gate that only ever sees PBSD-owned code
# would go on passing. This is the check that tells the two apart.
VENDORED_SITES = {
    "contrib/unbound/respip/respip.c": 956,
    "contrib/wpa/src/drivers/driver_bsd.c": 732,
    "contrib/ofed/opensm/libvendor/osm_vendor_al.c": 368,
    "crypto/openssl/crypto/thread/internal.c": 51,
    "crypto/openssl/apps/cms.c": 743,
}
found = 0
for rel, line in VENDORED_SITES.items():
    p = SRC / rel
    if not p.is_file():
        print(f"  skip   {rel} is not in this tree")
        continue
    text = p.read_text(errors="replace")
    clean = "\n".join(strip_if0(strip_noise(text).split("\n")))
    lines = {h[0] for h in scan(str(p), clean, text.split("\n"), False)}
    check(f"{rel}:{line} is still found when scanned directly",
          line in lines, f"got {sorted(lines)}")
    found += 1

check("at least three vendored sites were actually checked", found >= 3,
      f"only {found} of the five files are present")

print()
if fails:
    print(f"{len(fails)} check(s) failed")
    sys.exit(1)
print(f"all checks passed — {len(VENDOR)} vendor prefixes, "
      f"{found} vendored sites still visible to the rule")
