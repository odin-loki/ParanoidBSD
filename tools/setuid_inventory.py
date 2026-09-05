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
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "hbsd/src"

BINMODE = re.compile(r"^\s*BINMODE\s*=\s*([0-7]{4})\s*$", re.M)
SUBDIR_OPT = re.compile(r"^SUBDIR\.\$\{MK_([A-Z0-9_]+)\}\s*\+?=\s*(.*)$", re.M)

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
    "kerberos5/usr.bin/ksu":          "4555",
    "krb5/usr.bin/ksu":               "4555",
    "libexec/dma/dma-mbox-create":    "4554",
    "libexec/dma/dmagent":            "2555",
    "libexec/ulog-helper":            "4555",
    "sbin/mksnap_ffs":                "4554",
    "sbin/ping":                      "4555",
    "sbin/shutdown":                  "4554",
    "secure/libexec/ssh-keysign":     "4555",
    "usr.bin/at":                     "4555",
    "usr.bin/bluetooth/btsockstat":   "2555",
    "usr.bin/chpass":                 "4555",
    "usr.bin/lock":                   "4555",
    "usr.bin/login":                  "4555",
    "usr.bin/newgrp":                 "4555",
    "usr.bin/passwd":                 "4555",
    "usr.bin/quota":                  "4555",
    "usr.bin/su":                     "4555",
    "usr.bin/wall":                   "2555",
    "usr.bin/write":                  "2555",
    "usr.sbin/authpf":                "6555",
    "usr.sbin/cron/crontab":          "4555",
    "usr.sbin/lpr/lpc":               "2555",
    "usr.sbin/lpr/lpq":               "6555",
    "usr.sbin/lpr/lpr":               "6555",
    "usr.sbin/lpr/lprm":              "6555",
    "usr.sbin/ppp":                   "4554",
    "usr.sbin/sendmail":              "4555",
    "usr.sbin/traceroute":            "4555",
    "usr.sbin/traceroute6":           "4555",
}


def subdir_options() -> dict[str, str]:
    """directory path -> the MK_ option that includes it, where there is one."""
    out: dict[str, str] = {}
    for mk in SRC.rglob("Makefile"):
        if "contrib" in mk.parts:
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
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 on any change to the reviewed set")
    args = ap.parse_args()

    opts = subdir_options()
    found: dict[str, str] = {}
    for mk in SRC.rglob("Makefile"):
        if "contrib" in mk.parts:
            continue
        try:
            text = mk.read_text(errors="replace")
        except OSError:
            continue
        for m in BINMODE.finditer(text):
            mode = m.group(1)
            if mode[0] not in "2467":       # no setuid or setgid bit
                continue
            rel = mk.parent.relative_to(SRC).as_posix()
            # A directory can declare both (sendmail: 4555 and 2555).
            found[rel] = mode if rel not in found else \
                max(found[rel], mode)

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
        print(f"{found[rel]:>5s}  {opt:<18s} {rel}")

    print(f"\n{removable} of {len(found)} are removed by a src.conf option.")
    print("The rest are login, su, passwd, chpass, newgrp, ping, shutdown and")
    print("friends - the ones a Unix cannot drop without stopping being one.")

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
