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
import re, shutil, sys
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
# Resolved the way the build resolves it, not by basename. A module's
# SRCS names `blake3_zfs.c' and its .PATH decides WHICH one that is:
# sys/contrib/openzfs/module/zfs/blake3_zfs.c, not the boot loader's copy
# in sys/cddl/boot/zfs. Matching basenames called the second one built and
# reported a prefix that is correct as absorbing live code.
sys.path.insert(0, str(Path(__file__).resolve().parent))
import sweep_report  # noqa: E402

# One authority, not a copy of one. This block used to re-implement
# sweep_report's reading of sys/conf/files* and got to keep its own
# bugs: when that reading learned the `<name>.o ... dependency
# "$S/x.c"' form -- twenty-five entries, aesni's three among them --
# only one of the two copies learned it. sweep_report.names_it() is
# both authorities, kernel and userland, unioned; asking it here means
# the inventory and the sweep report can never disagree about what the
# build names.

# bmake IS the userland authority, and without it userland_names returns
# an empty set for everything - at which point every NOT_NAMED claim
# under lib/ and libexec/ passes because nothing names anything. The two
# sentinels below catch that, and did, on the first CI run after this
# check landed; saying it in one line here is friendlier than two
# cryptic sentinel failures.
if not shutil.which("bmake"):
    print("  NOTE bmake is not installed. The userland half of the "
          "authority is empty,\n       so the two sentinels below will "
          "fail. Install bmake.")

# It has to be able to say "named", or it says "not named" to everything.
# One from each authority, for the same reason.
for built in ("sys/dev/pci/pci.c", "sys/kern/kern_exec.c",
              "sys/vm/vm_page.c",
              "sys/contrib/openzfs/module/zfs/blake3_zfs.c",
              "sys/crypto/aesni/aesni_ghash.c",
              "lib/libc/gen/getcwd.c", "libexec/rtld-elf/rtld.c"):
    check(f"the build does name {built}", sweep_report.names_it(built),
          "if this fails every check below passes for the wrong reason")

_claimed = [pre for pre, why in NOT_BUILT.items() if why.endswith("NOT_NAMED")]
check("some prefix makes the NOT_NAMED claim", bool(_claimed))
for pre in _claimed:
    d = ROOT / "hbsd" / "src" / pre.rstrip("/")
    srcs = sorted(q.relative_to(ROOT / "hbsd" / "src").as_posix()
                  for q in d.glob("*.c")) if d.is_dir() else []
    named_here = [s for s in srcs if sweep_report.names_it(s)]
    check(f"{pre} is named by nothing", not named_here,
          f"the build names {named_here[:3]}, so this prefix is absorbing "
          f"ERRORs from code that IS compiled")
    check(f"{pre} has sources at all", bool(srcs),
          "an empty prefix absorbs nothing and hides its own staleness")

# The same claim made about ONE file, which is what a directory holding
# both built and unbuilt code needs. libexec/bootpd's try* probes are
# unnamed and bootpd.c beside them is not, so a prefix over that
# directory would absorb the whole daemon - this check caught exactly
# that, on its first run, on a prefix written five minutes earlier.
_claimed_f = [f for f, why in EXPECTED.items() if why.endswith("NOT_NAMED")]
check("some file makes the NOT_NAMED claim", bool(_claimed_f))
for _f in _claimed_f:
    check(f"{_f} exists", (ROOT / "hbsd" / "src" / _f).is_file())
    check(f"{_f} is named by nothing", not sweep_report.names_it(_f),
          "the build does name it, so this entry is hiding a real "
          "coverage gap")

print("\n== and the NOT_SUBDIR prefixes really are not descended into")
# A prefix whose reason ends in NOT_SUBDIR claims something the
# NOT_NAMED check cannot express: the directory has a Makefile of its
# own, so asking bmake IN it names its sources -- and no parent SUBDIR
# reaches it, so the build never asks. sbin/ipf/Makefile is the case
# that forced the distinction:
#
#   SUBDIR=       libipf .WAIT
#   SUBDIR+=      ipf ipfstat ipmon ipnat ippool
#   # XXX Temporarily disconnected.
#   # SUBDIR+=    ipftest ipresend ipsend
#
# NOT_NAMED was tried first and the gate rejected it, correctly: bmake
# in sbin/ipf/ipftest names ipftest.c. What is true is the line above,
# and this reads it.
_sub = [pre for pre, why in NOT_BUILT.items() if why.endswith("NOT_SUBDIR")]
check("some prefix makes the NOT_SUBDIR claim", bool(_sub))


def _subdirs(makefile: Path) -> set[str]:
    """The SUBDIR words a Makefile actually assigns, comments excluded."""
    out: set[str] = set()
    if not makefile.is_file():
        return out
    cont = False
    for raw in makefile.read_text().splitlines():
        line = raw.rstrip("\n")
        if not cont:
            stripped = line.lstrip()
            if stripped.startswith("#"):
                continue
            m = re.match(r"\s*SUBDIR(?:\.\$\{[^}]*\})?\s*\+?=\s*(.*)$", line)
            if not m:
                continue
            rest = m.group(1)
        else:
            if line.lstrip().startswith("#"):
                cont = line.rstrip().endswith("\\")
                continue
            rest = line
        cont = rest.rstrip().endswith("\\")
        rest = rest.rstrip().rstrip("\\")
        out.update(w for w in rest.split() if not w.startswith(".") and
                   not w.startswith("$"))
    return out


for pre in _sub:
    rel = pre.rstrip("/")
    d = ROOT / "hbsd" / "src" / rel
    check(f"{pre} is a directory", d.is_dir())
    srcs = sorted(q.name for q in d.glob("*.c")) if d.is_dir() else []
    check(f"{pre} has sources at all", bool(srcs),
          "an empty prefix absorbs nothing and hides its own staleness")
    parent = d.parent
    listed = _subdirs(parent / "Makefile")
    check(f"{pre} is in no parent SUBDIR", d.name not in listed,
          f"{parent.relative_to(ROOT / 'hbsd' / 'src')}/Makefile lists "
          f"{d.name} in SUBDIR, so the build DOES descend into it")

# And the sentinel, for the same reason the NOT_NAMED block has one: a
# reader that returns the empty set makes every claim above pass.
check("the SUBDIR reader can say `yes'",
      "ipf" in _subdirs(ROOT / "hbsd" / "src" / "sbin" / "ipf" / "Makefile"),
      "if this fails every NOT_SUBDIR check passes for the wrong reason")
check("and does not read commented lines",
      "ipftest" not in _subdirs(ROOT / "hbsd" / "src" / "sbin" / "ipf" /
                                "Makefile"),
      "the reader is taking `# SUBDIR+= ipftest' as an assignment")

print("\n== the DEFAULT_OFF entries name an option that really is off")
# A third shape, which neither NOT_NAMED nor NOT_SUBDIR can express: the
# file IS named -- bmake in its directory says so -- and the DIRECTORY
# is one the parent only descends into under an option. usr.bin/dpv is
# the case:
#
#   usr.bin/Makefile:194   SUBDIR.${MK_DIALOG}+=  dpv
#
# NOT_NAMED is false here and NOT_SUBDIR is false here; what is true is
# that MK_DIALOG is off unless somebody turns it on. The claim carries
# the option's name so both halves can be checked: the parent really
# gates the directory on that option, and src.opts.mk really defaults
# it to no.


def _default_no_options() -> set[str]:
    """The __DEFAULT_NO_OPTIONS list out of share/mk/src.opts.mk."""
    mk = ROOT / "hbsd" / "src" / "share" / "mk" / "src.opts.mk"
    out: set[str] = set()
    if not mk.is_file():
        return out
    taking = False
    for line in mk.read_text().splitlines():
        if re.match(r"^__DEFAULT_NO_OPTIONS\s*\+?=", line):
            taking = True
            line = line.split("=", 1)[1]
        elif not taking:
            continue
        body = line.strip()
        if body.startswith("#"):
            continue
        cont = body.endswith("\\")
        out.update(w for w in body.rstrip("\\").split() if w.isupper()
                   or "_" in w)
        if not cont:
            taking = False
    return {w for w in out if re.fullmatch(r"[A-Z0-9_]+", w)}


_no = _default_no_options()
check("the __DEFAULT_NO_OPTIONS reader finds something", len(_no) > 20)
check("...and it is not just returning everything", "KERBEROS" not in _no,
      "KERBEROS is __DEFAULT_YES; a reader that says no to everything "
      "makes every DEFAULT_OFF claim pass")
# Both shapes make this claim. A FILE entry names the file and the
# directory is its parent; a NOT_BUILT PREFIX names the directory
# itself, because the option gates the whole component -- lib/Makefile
# does list libsecureboot, but only under SUBDIR.${MK_BEARSSL}, so
# NOT_SUBDIR is the wrong claim for it and DEFAULT_OFF is the right
# one. The two halves checked below are the same either way, so the
# check is one loop over (label, directory, option).
_off = [(f, Path(f).parent, why.rsplit("DEFAULT_OFF:", 1)[1].strip())
        for f, why in EXPECTED.items() if "DEFAULT_OFF:" in why]
_off += [(pre, Path(pre.rstrip("/")),
          why.rsplit("DEFAULT_OFF:", 1)[1].strip())
         for pre, why in NOT_BUILT.items() if "DEFAULT_OFF:" in why]
check("some file makes the DEFAULT_OFF claim", bool(_off))
check("...and some prefix does too",
      any(_l.endswith("/") for _l, _d, _o in _off),
      "the prefix half of this check is testing nothing")
for _f, d, _opt in _off:
    check(f"{_f}: MK_{_opt} is off by default", _opt in _no,
          f"{_opt} is not in __DEFAULT_NO_OPTIONS, so this entry is "
          f"hiding a directory the build does descend into")
    parent = ROOT / "hbsd" / "src" / d.parent / "Makefile"
    txt = parent.read_text() if parent.is_file() else ""
    check(f"{_f}: its parent gates the directory on MK_{_opt}",
          re.search(r"SUBDIR\.\$\{MK_" + re.escape(_opt) +
                    r"\}\s*\+?=[^\n]*\b" + re.escape(d.name) + r"\b",
                    txt) is not None,
          f"{d.parent}/Makefile does not gate {d.name} on MK_{_opt}")

print("\n== the NEEDS_LOCALBASE entries really name the ports prefix")
# The last shape: the directory IS built, and its own Makefile says the
# header comes from a port. /usr/local is the ports prefix and nothing
# in this tree installs into it, so the claim is checkable by reading
# the Makefile that makes it.
_loc = [f for f, why in EXPECTED.items() if why.strip() == "NEEDS_LOCALBASE"]
check("some file makes the NEEDS_LOCALBASE claim", bool(_loc))
for _f in _loc:
    mk = ROOT / "hbsd" / "src" / Path(_f).parent / "Makefile"
    check(f"{_f}: its Makefile exists", mk.is_file())
    check(f"{_f}: and it names /usr/local",
          "/usr/local" in (mk.read_text() if mk.is_file() else ""),
          "nothing in the tree says this program needs a port")

print("\n== the INCLUDED_BY entries name a file that really includes them")
# An entry whose reason is INCLUDED_BY:<path> claims that the named file
# #includes this one, which is why it is not a translation unit of its
# own. dtrace.c does it nine times with `#include <dtrace_anon.c>' and
# angle brackets, so the check looks for either spelling of the basename
# in an #include line.
for _f, _why in sorted(EXPECTED.items()):
    if not _why.startswith("INCLUDED_BY:"):
        continue
    _by = _why.split("INCLUDED_BY:", 1)[1].strip()
    _p = ROOT / "hbsd" / "src" / _by
    check(f"{_f}: {_by} exists", _p.is_file())
    if not _p.is_file():
        continue
    _base = Path(_f).name
    check(f"{_f}: it is #included there",
          bool(re.search(rf'#\s*include\s*[<"][^>"]*{re.escape(_base)}[>"]',
                         _p.read_text(errors="replace"))),
          "nothing in that file includes this one, so the reason is "
          "describing a layout that has changed")

print("\n== and the DEAD_OPTION prefixes name a variable nothing sets")
# A prefix whose reason ends in DEAD_OPTION:NAME claims that no makefile
# in the tree ASSIGNS that make variable - only tests it - so the block
# guarding these sources never runs. Recomputed here, because "nobody
# enables this" is exactly the sort of claim that stops being true
# quietly.
_MK = list(SYS.rglob("**/Makefile*")) + list(SYS.rglob("**/*.mk"))
for pre, why in sorted(NOT_BUILT.items()):
    if "DEAD_OPTION:" not in why:
        continue
    var = why.rsplit("DEAD_OPTION:", 1)[1].strip()
    setters = []
    for mk in _MK:
        for ln in mk.read_text(errors="replace").splitlines():
            if re.match(rf"^\s*{re.escape(var)}\s*[?+:]?=", ln):
                setters.append(mk.relative_to(ROOT).as_posix())
                break
    check(f"{pre}: nothing assigns {var}", not setters,
          f"assigned by {setters[:3]}, so the block is live and this "
          f"prefix is absorbing ERRORs from code that IS compiled")
    check(f"{pre}: something still tests {var}",
          any(re.search(rf"defined\({re.escape(var)}\)",
                        mk.read_text(errors="replace")) for mk in _MK),
          "no makefile mentions it at all any more, so the reason "
          "describes a block that is gone")

print("\n== vchiq is unreachable from both ends, still")
# Four facts make that prefix true, and any one of them flipping means a
# kernel can reach the driver and the exemption is hiding real ERRORs.
if any("VCHIQ_UNREACHABLE" in w for w in NOT_BUILT.values()):
    _f_arm64 = (SYS / "conf" / "files.arm64").read_text(errors="replace")
    _f_arm = (SYS / "conf" / "files.arm").read_text(errors="replace")
    _f_mi = (SYS / "conf" / "files").read_text(errors="replace")
    check("files.arm64 names vchiq sources", "vchiq" in _f_arm64)
    check("files.arm names none", "vchiq" not in _f_arm)
    check("the MI files names none", "vchiq" not in _f_mi)
    check("there is no sys/modules/vchiq",
          not (SYS / "modules" / "vchiq").exists())
    _a64 = [p for p in sorted((SYS / "arm64" / "conf").rglob("*"))
            if p.is_file() and re.search(r"^\s*device\s+vchiq\b",
                                         p.read_text(errors="replace"), re.M)]
    check("no arm64 config declares device vchiq", not _a64,
          f"declared by {[p.name for p in _a64]}, so arm64 CAN build it")
    _a32 = [p for p in sorted((SYS / "arm" / "conf").rglob("*"))
            if p.is_file() and re.search(r"^\s*device\s+vchiq\b",
                                         p.read_text(errors="replace"), re.M)]
    check("an arm config still declares it, pointlessly", bool(_a32),
          "if nothing declares it anywhere the reason needs rewriting, "
          "not deleting")
    check("arm64 has no machine/fdt.h",
          not (SYS / "arm64" / "include" / "fdt.h").exists())

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
