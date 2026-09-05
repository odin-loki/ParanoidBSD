#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Find MK_ options the build reads but nothing defines.

bmake treats an undefined variable in a .if as a fatal error, not as empty.
So a src.opts.mk edit that removes an option while a Makefile still tests it
does not disable a feature - it stops the build:

  llvm.build.mk:65: Variable "MK_LLVM_TARGET_ARM" is undefined
  make[3]: Fatal errors encountered -- cannot continue

That is what dropping arm, powerpc and riscv from __LLVM_TARGETS did. The
intent was right for an amd64/arm64 target; the loop it was removed from is
also what defines MK_LLVM_TARGET_<backend>, and lib/clang/llvm.build.mk
tests all seven unconditionally.

Definitions come from the option lists in share/mk (__DEFAULT_YES_OPTIONS,
__DEFAULT_NO_OPTIONS, __DEFAULT_DEPENDENT_OPTIONS, BROKEN_OPTIONS and the
like), from explicit MK_FOO:= assignments, and from the LLVM_TARGET loop,
which is expanded here rather than guessed at.
"""

from __future__ import annotations

import argparse
import os
import re
import sys

MK_REF = re.compile(r"\$\{MK_([A-Z0-9_]+)\}")
MK_ASSIGN = re.compile(r"^\s*MK_([A-Z0-9_]+)\s*[:+?]?=")
# Any __..._OPTIONS list, not an enumerated few: CASPER lives in
# __REQUIRED_OPTIONS, which an enumerated list missed.
OPTION_LIST = re.compile(
    r"^\s*(?:__[A-Z0-9_]*OPTIONS|BROKEN_OPTIONS)\s*[:+?]?=\s*(.*)$")

# bmake vendors its own mk system with its own MK_ namespace (MK_PIC,
# MK_OBJ, MK_AUTODEP...). Those are not FreeBSD src.opts options and are
# not defined by share/mk, by design.
SKIP_PREFIXES = ("contrib/bmake/", "tools/build/make_check/", "targets/")
LLVM_TARGETS = re.compile(r"^__LLVM_TARGETS\s*[:+?]?=\s*(.*)$")


def read(path: str) -> list[str]:
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            return fh.read().splitlines()
    except OSError:
        return []


def continued(lines: list[str], i: int, value: str) -> tuple[str, int]:
    while value.endswith("\\") and i + 1 < len(lines):
        i += 1
        value = value[:-1] + " " + lines[i].strip()
    return value, i


# MK_FOO the build reads that no .mk file can be shown to define, for a
# reason other than a bug. Keep this short; every entry is a hole.
EXCEPTIONS = {
    "FOO": "the placeholder in share/mk/bsd.confs.mk's own documentation",
    "MAN_UTILS": "set from the command line by Makefile.inc1's own recursion",
    "OSRELDATE_SH": "set by include/Makefile for its own sub-make",
    "USB_GADGET_EXAMPLES": "a sys/modules knob, set on the make line",
    "STATIC_DIRDEPS_CACHE": "dirdeps meta-mode, off unless set externally",
}


def defined_options(dirs: list[str]) -> set[str]:
    names: set[str] = set()
    files = []
    for d in dirs:
        if not os.path.isdir(d):
            continue
        files += [os.path.join(d, e) for e in sorted(os.listdir(d))
                  if e.endswith(".mk")]
    for path in files:
        lines = read(path)
        i = 0
        while i < len(lines):
            line = lines[i]
            m = MK_ASSIGN.match(line)
            if m:
                names.add(m.group(1))
            m = OPTION_LIST.match(line)
            if m:
                value, i = continued(lines, i, m.group(1))
                for tok in value.split():
                    # DEPENDENT options are written OPTION/CONTROLLING
                    names.add(tok.split("/")[0].strip().upper())
            m = LLVM_TARGETS.match(line)
            if m:
                value, i = continued(lines, i, m.group(1))
                for tok in value.split():
                    names.add("LLVM_TARGET_" + tok.upper())
            i += 1
    # src.opts.mk generates the whole MK_*_SUPPORT family from the plain
    # options with `__DEFAULT_DEPENDENT_OPTIONS+= ${var}_SUPPORT/${var}`
    # inside a .for. Rather than expand the loop, apply its rule.
    names |= {n + "_SUPPORT" for n in list(names)}
    return names


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--fail-on-undefined", action="store_true")
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    mkdir = os.path.join(root, "share", "mk")
    if not os.path.isdir(mkdir):
        print(f"no such tree: {args.root}", file=sys.stderr)
        return 2

    known = defined_options([mkdir, os.path.join(root, "sys", "conf")])
    refs: dict[str, tuple[str, int]] = {}
    scanned = 0
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d != ".git"]
        for name in filenames:
            if not (name.startswith("Makefile") or name.endswith(".mk")):
                continue
            path = os.path.join(dirpath, name)
            rel_early = os.path.relpath(path, root)
            if rel_early.startswith(SKIP_PREFIXES):
                continue
            scanned += 1
            for lineno, line in enumerate(read(path), 1):
                for opt in MK_REF.findall(line):
                    if opt in EXCEPTIONS:
                        continue
                    if opt not in known and opt not in refs:
                        refs[opt] = (os.path.relpath(path, root), lineno)

    print(f"makefiles scanned:  {scanned}")
    print(f"options defined:    {len(known)}")
    print(f"known-undefined (excepted): {len(EXCEPTIONS)}")
    print(f"referenced, undefined: {len(refs)}")
    for opt, (rel, lineno) in sorted(refs.items()):
        print(f"  MK_{opt}  first read at {rel}:{lineno}")

    if refs and args.fail_on_undefined:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
