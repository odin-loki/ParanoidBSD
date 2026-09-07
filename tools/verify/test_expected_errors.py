#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The ERROR inventory, and whether its reasons are still true.

`--check-errors` already fails on a file that errors and is not listed,
and on a listed file that compiles again. Neither catches the thing a
NOT_BUILT prefix can get wrong, which is its *reason*: the prefix says
"sys/modules/Makefile does not descend into this module", and the day
somebody adds it there the prefix goes on quietly absorbing three
hundred ERRORs that have become real.

The ratio the gate prints - "135 of 135 under sys/contrib/dev/mediatek/
are NOT_BUILT" - is a weaker signal on purpose: it only fails when every
file under a prefix compiles. So the justification is checked here
instead, against the build system, the way the flags are.
"""
from __future__ import annotations
import re, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from expected_errors import EXPECTED, NOT_BUILT, not_built  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SYS = ROOT / "hbsd" / "src" / "sys"

fails: list[str] = []


def check(name: str, cond: bool, why: str = "") -> None:
    if cond:
        print(f"  ok   {name}")
    else:
        print(f"  FAIL {name}{'  ' + why if why else ''}")
        fails.append(name)


print("== the prefixes do not overlap the file entries")
for f in sorted(EXPECTED):
    pre = not_built(f)
    check(f"{f} is listed once", pre is None,
          f"also covered by the NOT_BUILT prefix {pre}")

print("\n== every prefix names a real directory")
for pre in sorted(NOT_BUILT):
    d = ROOT / "hbsd" / "src" / pre.rstrip("/")
    check(f"{pre} exists", d.is_dir())

print("\n== and its reason is still true")
# The three vendored driver trees claim sys/modules/Makefile does not
# descend into their module. That is a fact about one file.
_raw = (SYS / "modules" / "Makefile").read_text(errors="replace")
# Comments first, THEN the continuations - joining first makes every
# entry one enormous line and a per-line ^...$ match never fires again.
# That was this test's own bug, found the way the rest were: by adding
# mt76 to the SUBDIR list and watching the check pass.
_nc = "\n".join(l.split("#")[0] for l in _raw.splitlines())
mods = _nc.replace("\\\n", " ")


def _token(name: str, text: str) -> bool:
    return bool(re.search(rf"(?<![\w./-]){re.escape(name)}(?![\w./-])", text))


def descends(name: str) -> bool:
    """Does sys/modules/Makefile reach this module directory?

    Both spellings: a bare `name' in the SUBDIR list, and the `${_name}'
    indirection with a `_name= name' assignment that is itself used.
    """
    if re.search(rf"^\s*_{re.escape(name)}\s*=\s*\S", _nc, re.M):
        return _token(f"${{_{name}}}", mods)
    # The bare name, but not the `_name= name' right-hand side, which is
    # only a definition - being defined is not being built.
    stripped = re.sub(rf"^\s*_\w+\s*=.*$", "", _nc, flags=re.M)
    return _token(name, stripped)


# The check has to be able to say yes, or it says no to everything.
for built in ("iwlwifi", "rtw88", "rtw89"):
    check(f"sys/modules/Makefile does descend into {built}", descends(built),
          "if this fails the test below proves nothing")

CLAIMED_NOT_DESCENDED = {
    "sys/contrib/dev/mediatek/": ["mt76"],
    "sys/contrib/dev/athk/": ["ath10k", "ath11k", "ath12k", "athk_common"],
    "sys/contrib/dev/broadcom/": ["brcm80211"],
}
for pre, names in CLAIMED_NOT_DESCENDED.items():
    for n in names:
        check(f"{pre} still holds: no descent into {n}", not descends(n),
              "sys/modules/Makefile builds it now, so this prefix is "
              "absorbing ERRORs that are real")

# ...and the module directory has to still be there, or the reason is
# describing something that no longer exists.
for pre, names in CLAIMED_NOT_DESCENDED.items():
    for n in names:
        check(f"sys/modules/{n} exists", (SYS / "modules" / n).is_dir(),
              "the prefix's reason names a module that is gone")

print("\n== and the NOT_NAMED prefixes really are named by nothing")
# A prefix whose reason ends in NOT_NAMED claims that no sys/conf/files*
# entry and no sys/modules Makefile mentions any source under it. That is
# a computation over the build system, so it is done here rather than
# believed - the same rule as everywhere else in tools/verify: read the
# build, do not keep a second copy of its answer.
_named: set[str] = set()
for _p in sorted(SYS.glob("conf/files*")):
    _t = _p.read_text(errors="replace").replace("\\\n", " ")
    for _l in _t.splitlines():
        _m = re.match(r"^(\S+\.[cS])\s", _l)
        if _m:
            _named.add(_m.group(1))
            _named.add(Path(_m.group(1)).name)
for _p in sorted(SYS.rglob("modules/**/Makefile")):
    for _m in re.finditer(r"(\S+\.[cS])", _p.read_text(errors="replace")):
        _named.add(_m.group(1))
        _named.add(Path(_m.group(1)).name)

# It has to be able to say "named", or it says "not named" to everything.
for built in ("dev/pci/pci.c", "kern/kern_exec.c", "vm/vm_page.c"):
    check(f"the build does name {built}", built in _named,
          "if this fails every check below passes for the wrong reason")

_claimed = [pre for pre, why in NOT_BUILT.items() if why.endswith("NOT_NAMED")]
check("some prefix makes the NOT_NAMED claim", bool(_claimed))
for pre in _claimed:
    d = ROOT / "hbsd" / "src" / pre.rstrip("/")
    srcs = sorted(q.relative_to(ROOT / "hbsd" / "src" / "sys").as_posix()
                  for q in d.glob("*.c")) if d.is_dir() else []
    named_here = [s for s in srcs
                  if s in _named or Path(s).name in _named]
    check(f"{pre} is named by nothing", not named_here,
          f"the build names {named_here[:3]}, so this prefix is absorbing "
          f"ERRORs from code that IS compiled")
    check(f"{pre} has sources at all", bool(srcs),
          "an empty prefix absorbs nothing and hides its own staleness")

# ACPICA's two are option-gated rather than module-gated: no kernel
# config in the tree sets ACPI_DEBUGGER.
opt = "ACPI_DEBUGGER"
setters = [p for p in sorted((SYS).glob("*/conf/*"))
           if p.is_file() and p.name.isupper() and p.name != "NOTES"
           and re.search(rf"^\s*options?\s+{opt}\b", p.read_text(errors="replace"),
                         re.M)]
check(f"no kernel config sets {opt}", not setters,
      f"set by {[p.name for p in setters]}")

print()
if fails:
    print(f"{len(fails)} check(s) failed")
    sys.exit(1)
print(f"all checks passed — {len(EXPECTED)} file entries, "
      f"{len(NOT_BUILT)} prefixes")
