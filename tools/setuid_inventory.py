#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""What does PBSD ship setuid, and did that change?

Every setuid-root binary is a local privilege escalation waiting for a bug,
and a hardened system should know its list by heart. FreeBSD declares them in
the program's own Makefile - BINMODE=4555 for setuid, 2555 for setgid - so
the set can be read without building anything.

Two outputs, and the second is the point:

  * the inventory, with the src.conf option that removes each one where
    there is one. That is the menu: turning a subsystem off is one line and
    it removes the binary, its bugs, and its attack surface together.
  * a gate. The reviewed set is in ALLOWED below. A new setuid binary
    appearing - from an upstream merge, a new subsystem, a Makefile edit -
    fails, because "we gained a setuid root program" should never be
    something you find out later.

This reads Makefiles, not a staged tree. It is therefore a statement about
what the tree declares, not about what a built image contains; a
`find -perm -4000` over the staging directory is the other half and needs a
build. Both are worth having and they answer different questions.
"""
from __future__ import annotations

import argparse
import functools
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "hbsd/src"

BINMODE = re.compile(r"^\s*BINMODE\s*[?+]?=\s*([0-7]{4})\s*$")
SUBDIR_OPT = re.compile(r"^SUBDIR\.\$\{MK_([A-Z0-9_]+)\}\s*\+?=\s*(.*)$", re.M)
PROG_LINE = re.compile(r"^\s*PROG(?:_CXX)?\s*[?+]?=\s*(\S+)")
LINKS_LINE = re.compile(r"^\s*LINKS\s*[?+]?=\s*(.*)$")
MK_IF = re.compile(r"^\.\s*if\s+\$\{MK_([A-Z0-9_]+)\}\s*!=\s*\"no\"")
SUBDIR_PLAIN = re.compile(r"^SUBDIR\s*\+?=\s*(.*)$")


IFDEF = re.compile(r"^\.\s*(?:if)?(n?)def\s+([A-Za-z_]\w*)")
IF_DEFINED = re.compile(r"^\.\s*if\s+(!?)\s*defined\(([A-Za-z_]\w*)\)\s*$")
IF_MK = re.compile(r"^\.\s*if\s+\$\{MK_([A-Z0-9_]+)\}\s*!=\s*\"no\"\s*$")


@functools.lru_cache(maxsize=None)
def _defined(name: str) -> bool:
    """Is this make variable set anywhere the build would see it?

    A grep, deliberately: NOSUID, PPP_NO_SUID, ENABLE_SUID_NEWGRP,
    ENABLE_SUID_K5SU and SENDMAIL_SET_USER_ID are all knobs a builder
    passes in, and the only mentions of any of them in this tree are the
    .if that reads them and a COMMENTED-OUT line in
    share/examples/etc/make.conf. If one of them ever gains a real
    definition this returns true and the entry moves back into the
    shipped set, which is the behaviour that keeps the gate honest.
    """
    pat = re.compile(rf"^\s*{re.escape(name)}\s*[?+]?=", re.M)
    for f in [ROOT / "hbsd/src.conf.pbsd", SRC / "share/mk/src.opts.mk",
              SRC / "Makefile.inc1"]:
        if f.is_file() and pat.search(f.read_text(errors="replace")):
            return True
    return False


@functools.lru_cache(maxsize=None)
def _without(opt: str) -> bool:
    """Does this project's src.conf turn MK_<opt> off?"""
    f = ROOT / "hbsd/src.conf.pbsd"
    if not f.is_file():
        return False
    return re.search(rf"^\s*WITHOUT_{re.escape(opt)}\s*=", 
                     f.read_text(errors="replace"), re.M) is not None


def cond_holds(cond: str) -> bool | None:
    """True, False, or None for a condition this cannot decide."""
    neg = cond.startswith("!")
    if neg:
        cond = cond[1:]
    r: bool | None = None
    m = IF_DEFINED.match(cond)
    if m:
        r = _defined(m.group(2)) if not m.group(1) else not _defined(m.group(2))
    else:
        m = IFDEF.match(cond)
        if m:
            r = _defined(m.group(2)) if not m.group(1) else not _defined(m.group(2))
        else:
            m = IF_MK.match(cond)
            if m:
                r = not _without(m.group(1))
    if r is None:
        return None
    return (not r) if neg else r


def read_prog(mk: Path) -> tuple[str, list[str], str | None, str | None]:
    """(installed name, its hard links, mode, the .if guarding that mode).

    Three things this file used to get wrong, all found the same way - by
    comparing it against the first `find -perm -4000' over a real PBSD
    image, which boot run 60 produced:

    * BINMODE was matched anywhere in the file. usr.bin/newgrp/Makefile is

          .if defined(ENABLE_SUID_NEWGRP)
          BINMODE=      4555
          .endif

      and ENABLE_SUID_NEWGRP appears nowhere in this tree except that
      line and a COMMENTED-OUT example in share/examples/etc/make.conf.
      So newgrp ships 0555 and this inventory said it was setuid root -
      a false entry in a security inventory, and the same defect as
      hardening_sysctls.py's: a value read out of a conditional block
      without asking whether the condition holds.

    * The directory is not the program. libexec/dma/dmagent has
      `PROG= dma', so the inventory named a path no system has.

    * LINKS makes one binary several. The image has 33 setuid/setgid
      files and this tool declared 30; the difference is entirely hard
      links - at/atq/atrm/batch, ping/ping6, shutdown/poweroff,
      authpf/authpf-noip - and nothing said so.
    """
    prog, links = mk.parent.name, []
    modes: list[tuple[str, str | None]] = []
    depth, conds = 0, []
    for line in mk.read_text(errors="replace").replace("\\\n", " ").splitlines():
        st = line.strip()
        if st.startswith((".if", ".for")):
            depth += 1
            conds.append(st)
            continue
        if st.startswith(".else"):
            # The setuid BINMODE is usually in the ELSE arm:
            #
            #   .if defined(NOSUID)
            #   BINMODE=554
            #   .else
            #   BINMODE=4554
            #   .endif
            #
            # so a version of this that kept the .if text attributed the
            # setuid mode to the condition that switches it OFF, and
            # declared mksnap_ffs, ppp and login not shipped setuid -
            # which the image says they are. Negate.
            if conds:
                conds[-1] = "!" + conds[-1]
            continue
        if st.startswith((".endif", ".endfor")):
            depth = max(0, depth - 1)
            if conds:
                conds.pop()
            continue
        m = PROG_LINE.match(line)
        if m:
            prog = m.group(1)
            continue
        m = LINKS_LINE.match(line)
        if m:
            # LINKS is pairs: ${BINDIR}/at ${BINDIR}/atq ...  Every second
            # token is the new name.
            toks = m.group(1).split()
            links += [t.rsplit("/", 1)[-1] for t in toks[1::2]]
            continue
        m = BINMODE.match(line)
        if m and m.group(1)[0] in "2467":
            modes.append((m.group(1), conds[-1] if depth else None))
    # One Makefile can carry several, one per branch. sendmail is
    #
    #   .ifdef SENDMAIL_SET_USER_ID
    #   BINMODE=4555
    #   .else
    #   BINMODE=2555
    #   .endif
    #
    # so taking the highest mode and then asking about its guard picked
    # the setuid arm nobody builds, decided it was not shipped, and lost
    # the setgid arm that IS - the image has sendmail 2555. Pick among the
    # branches whose condition holds, and only report a guard when none
    # does.
    live = [(md, g) for md, g in modes
            if g is None or cond_holds(g) is True]
    if live:
        return prog, links, max(md for md, _ in live), None
    if modes:
        md, g = max(modes)
        return prog, links, md, g
    return prog, links, None, None

# The reviewed set: path -> (mode, why it is here).
#
# Read as a list of decisions not yet made. Nothing in src.conf.pbsd removes
# any of these today.
#
# Four of them are mode 6555 - setuid AND setgid root: authpf, lpq, lpr and
# lprm. The first hand-written version of this list missed all four, because
# the grep behind it matched 4xxx and 2xxx and not 6xxx. The tool caught its
# own author on its first run, which is the argument for having it.
ALLOWED = {
    "libexec/dma/dma-mbox-create":   "4554",
    "libexec/dma/dmagent":           "2555",
    "libexec/ulog-helper":           "4555",
    "sbin/mksnap_ffs":               "4554",
    "sbin/ping":                     "4555",
    "sbin/shutdown":                 "4554",
    "secure/libexec/ssh-keysign":    "4555",
    "usr.bin/at":                    "4555",
    "usr.bin/bluetooth/btsockstat":  "2555",
    "usr.bin/chpass":                "4555",
    "usr.bin/lock":                  "4555",
    "usr.bin/login":                 "4555",
    "usr.bin/passwd":                "4555",
    "usr.bin/quota":                 "4555",
    "usr.bin/su":                    "4555",
    "usr.bin/wall":                  "2555",
    "usr.bin/write":                 "2555",
    "usr.sbin/authpf":               "6555",
    "usr.sbin/cron/crontab":         "4555",
    "usr.sbin/lpr/lpc":              "2555",
    "usr.sbin/lpr/lpq":              "6555",
    "usr.sbin/lpr/lpr":              "6555",
    "usr.sbin/lpr/lprm":             "6555",
    "usr.sbin/ppp":                  "4554",
    "usr.sbin/sendmail":             "2555",
    "usr.sbin/traceroute":           "4555",
    "usr.sbin/traceroute6":          "4555",
}


def subdir_options() -> dict[str, str]:
    """directory path -> the MK_ option that includes it, where there is one."""
    out: dict[str, str] = {}
    # Makefile.inc1 as well as every Makefile: the top-level SUBDIR list
    # that decides whether krb5 or kerberos5 is built at all lives there,
    # and rglob("Makefile") does not match it.
    for mk in [SRC / "Makefile.inc1", *SRC.rglob("Makefile")]:
        if "contrib" in mk.parts or not mk.is_file():
            continue
        try:
            text = mk.read_text(errors="replace")
        except OSError:
            continue
        parent = mk.parent.relative_to(SRC).as_posix()
        for m in SUBDIR_OPT.finditer(text):
            opt, names = m.group(1), m.group(2)
            for n in names.replace("\\", " ").split():
                key = f"{parent}/{n}" if parent != "." else n
                out.setdefault(key, opt)
        # ...and the OTHER spelling, which is the one Makefile.inc1 uses
        # for Kerberos:
        #
        #   .if ${MK_KERBEROS} != "no"
        #   .if ${MK_MITKRB5} != "no"
        #   SUBDIR+=krb5
        #   .else
        #   SUBDIR+=kerberos5
        #   .endif
        #   .endif
        #
        # Only the SUBDIR.${MK_X} form was matched, so both copies of ksu -
        # a setuid-root program, and the most attractive one on the list -
        # were reported with an empty "removed by" column, as though
        # nothing could take them out. WITHOUT_KERBEROS takes both out.
        stack: list[str] = []
        for line in text.replace("\\\n", " ").splitlines():
            st = line.strip()
            m = MK_IF.match(st)
            if m:
                stack.append(m.group(1))
                continue
            if st.startswith((".if", ".for")):
                stack.append("")
                continue
            if st.startswith((".endif", ".endfor")):
                if stack:
                    stack.pop()
                continue
            m = SUBDIR_PLAIN.match(line)
            if m:
                # The OUTERMOST option, not the innermost. Nested .if
                # blocks all have to hold, so any of them being "no"
                # removes the directory - but MK_MITKRB5 only chooses
                # WHICH Kerberos is built, and WITHOUT_MITKRB5 builds
                # kerberos5 with the same setuid ksu in it. The answer to
                # "what removes this" is MK_KERBEROS, the outer one.
                opt = next((o for o in stack if o), None)
                if not opt:
                    continue
                for n in m.group(1).replace("\\", " ").split():
                    key = f"{parent}/{n}" if parent != "." else n
                    out.setdefault(key, opt)
    return out


def check_image(path: Path, found: dict[str, str], progs: dict[str, str],
                links: dict[str, list[str]]) -> int:
    """The other half: what a built image actually has.

    This file's own docstring said a `find -perm -4000' over a staged tree
    was the missing half and needed a build. Boot run 60 is that build -
    the first PBSD image with a login, asked the question directly - and
    the answer disagreed with this tool in three places, all of which
    were this tool's fault and are fixed above.
    """
    live: dict[str, str] = {}
    for line in path.read_text(errors="replace").splitlines():
        f = line.split()
        if len(f) >= 9 and f[0][0] == "-":
            live[f[-1]] = f[0]

    names: dict[str, str] = {}
    for rel, mode in found.items():
        for n in [progs[rel]] + links[rel]:
            names[n] = rel

    extra = sorted(p for p in live if p.rsplit("/", 1)[-1] not in names)
    absent = sorted({n for n in names} -
                    {p.rsplit("/", 1)[-1] for p in live})

    print(f"\n== against {path}: {len(live)} setuid/setgid files")
    for p2 in extra:
        print(f"  FAIL on the image and not declared: {p2}  {live[p2]}")
    for n in absent:
        rel = names[n]
        print(f"  note declared and not on this image: {n} ({rel})")
    if not extra:
        print("  ok    every setuid file on the image is one the tree "
              "declares")
    print("  A binary the tree does not declare is one nobody reviewed. "
          "The\n  other direction is a build option, not a surprise.")
    return 1 if extra else 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 on any change to the reviewed set")
    ap.add_argument("--check", metavar="FILE",
                    help="`find / -xdev -type f \\( -perm -4000 -o -perm "
                         "-2000 \\) -exec ls -l {} +' from a booted system; "
                         "compare it against what the tree declares")
    args = ap.parse_args()

    opts = subdir_options()
    found: dict[str, str] = {}
    progs: dict[str, str] = {}
    links: dict[str, list[str]] = {}
    guarded: dict[str, tuple[str, str]] = {}
    for mk in SRC.rglob("Makefile"):
        if "contrib" in mk.parts:
            continue
        try:
            prog, lnks, mode, guard = read_prog(mk)
        except OSError:
            continue
        if mode is None:
            continue
        rel = mk.parent.relative_to(SRC).as_posix()
        if guard is not None and cond_holds(guard) is not True:
            # Declared setuid under a condition that does not hold, or
            # that this cannot decide. Reported, and not counted as
            # shipped: see read_prog().
            guarded[rel] = (mode, guard)
            continue
        # A directory can declare both (sendmail: 4555 and 2555).
        found[rel] = mode if rel not in found else max(found[rel], mode)
        progs[rel] = prog
        links[rel] = lnks

    setuid = {k: v for k, v in found.items() if v[0] in "467"}
    setgid = {k: v for k, v in found.items() if v[0] == "2"}

    print(f"{len(found)} programs are declared setuid or setgid "
          f"({len(setuid)} setuid, {len(setgid)} setgid)\n")
    print(f"{'mode':>5s}  {'removed by':<18s} path")
    removable = 0
    for rel in sorted(found):
        # Walk up looking for a directory an MK_ option controls.
        opt = ""
        parts = rel.split("/")
        for i in range(len(parts), 0, -1):
            cand = "/".join(parts[:i])
            if cand in opts:
                opt = "WITHOUT_" + opts[cand]
                break
        if opt:
            removable += 1
        also = (" -> " + ", ".join(links[rel])) if links[rel] else ""
        name = progs[rel]
        shown = rel if name == rel.rsplit("/", 1)[-1] else f"{rel} ({name})"
        print(f"{found[rel]:>5s}  {opt:<18s} {shown}{also}")

    nfiles = len(found) + sum(len(v) for v in links.values())
    print(f"\n{removable} of {len(found)} are removed by a src.conf option.")
    print("The rest are login, su, passwd, chpass, ping, shutdown and")
    print("friends - the ones a Unix cannot drop without stopping being one.")
    print(f"\n{nfiles} files on an installed system: {len(found)} programs "
          f"plus {nfiles - len(found)} hard links that LINKS makes.")
    if guarded:
        print(f"\n{len(guarded)} declared setuid only under a condition, and "
              f"NOT shipped setuid:")
        for rel, (mode, guard) in sorted(guarded.items()):
            print(f"  {mode}  {rel}   {guard}")
        print("  Nothing in this tree satisfies those, so the binary ships")
        print("  0555. Reading BINMODE without its guard is how this file")
        print("  used to claim newgrp was setuid root.")

    if args.check:
        return check_image(Path(args.check), found, progs, links)

    if not args.gate:
        return 0

    cur = {k: v for k, v in found.items()}
    added = sorted(set(cur) - set(ALLOWED))
    removed = sorted(set(ALLOWED) - set(cur))
    changed = sorted(k for k in set(cur) & set(ALLOWED)
                     if cur[k] != ALLOWED[k])
    if not (added or removed or changed):
        print(f"\nOK  the setuid set is the reviewed one, {len(ALLOWED)} "
              "entries.")
        return 0
    print()
    for k in added:
        print(f"FAIL new setuid/setgid program: {k} mode {cur[k]}")
        print("     Gaining one of these is not something to find out later.")
    for k in removed:
        print(f"NOTE no longer setuid/setgid: {k} (was {ALLOWED[k]})")
    for k in changed:
        print(f"FAIL mode changed: {k} {ALLOWED[k]} -> {cur[k]}")
    if added or changed:
        print("\nUpdate ALLOWED once the change has been read, not before.")
        return 1
    print("\nRemovals only. Update ALLOWED.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
