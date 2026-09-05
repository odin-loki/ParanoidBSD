#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Assert PBSD's edits to the vendor tree are still there.

The vendor branch's base tree is hbsd/src as PBSD has it, not upstream as
it was at the snapshot -- that revision is not recoverable. For 11,337 of
the 11,341 files that differ from upstream this makes no difference, since
the difference is upstream moving on and a merge will simply take it.

For four files it does. PBSD changed them, and with base == ours a merge
takes upstream's side silently: no conflict, no message, PBSD's hundred
lines gone. There is no way to make git notice. So it is checked instead.

Run this after every upstream merge. If a marker is missing, the merge ate
it; recover the hunk from the previous commit rather than re-deriving it.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# file -> (marker that must appear, what PBSD changed there)
# Makefile.inc1's KNOWN_ARCHES trim and src.opts.mk's __LLVM_TARGETS trim
# used to be here. They were reverted: PBSD wants first-class support for
# every architecture, and narrowing the tree to amd64 and arm64 was the
# thing making each new one expensive. Both files now differ from upstream
# only where upstream has moved on.
MARKS = {
    "hbsd/src/sys/conf/kern.mk":
        ("ParanoidBSD", "freestanding kernel C++23 flags"),
    "hbsd/src/sys/conf/kmod.mk":
        ("PBSD", "C++23 module TUs, dual-link pattern"),
    "hbsd/src/lib/msun/src/math_private.h":
        ("PBSD", "__BEGIN_DECLS, so ported kernels keep C linkage"),
    "hbsd/src/share/mk/src.opts.mk":
        ("PBSD", "keep SafeStack when the toolchain is external"),
}

# The same hazard, for edits too small to carry a marker comment without the
# comment being larger than the fix. Each is a bug upstream still has, found
# by building an architecture upstream does not build. file -> (must appear,
# must not appear, what and why).
FIXES = {
    "hbsd/src/sys/hardenedbsd/hbsd_pax_aslr.c": (
        "#define\tPAX_ASLR_DELTA_THR_STACK_DEF_LEN\t14",
        "#ifdef MAP_32BIT\n",
        "32-bit ASLR: thread-stack default, and MAP_32BIT keyed off __LP64__",
    ),
    "hbsd/src/sys/hardenedbsd/hbsd_pax_common.c": (
        "#ifdef __LP64__\n\tflags |= pax_disallow_map32bit_setup_flags",
        "#ifdef MAP_32BIT\n",
        "call the map32bit setup only where the function is compiled",
    ),
    "hbsd/src/sys/vm/vm_mmap.c": (
        "#if defined(__LP64__) && defined(PAX_HARDENING)",
        "#if defined(MAP_32BIT) && defined(PAX_HARDENING)",
        "MAP_32BIT ASLR call sites match where the delta exists",
    ),
    "hbsd/src/sys/arm/allwinner/a64/sun50i_a64_acodec.c": (
        "mixer_lock = &m->lock;",
        "mixer_get_lock",
        "the sound stack published struct snd_mixer and dropped the accessor",
    ),
    "hbsd/src/sys/powerpc/pseries/phyp_vscsi.c": (
        "return (ENOMEM);",
        "return (ENOMEM)\n",
        "missing semicolon; the file has never been compiled upstream",
    ),
    "hbsd/src/sys/kern/sched_shim.c": (
        "#ifdef __DO_NOT_HAVE_SYS_IFUNCS",
        None,
        "plain-C shims where the architecture has no kernel ifunc (arm)",
    ),
    "hbsd/src/sys/modules/linux/Makefile": (
        "SRCS+=\tlinux.c",
        "imgact_linux.c",
        "i386 module listed a source removed from the tree years ago",
    ),
}


def main() -> int:
    missing = []
    for rel, (marker, what) in sorted(MARKS.items()):
        path = ROOT / rel
        if not path.is_file():
            missing.append((rel, what, "file is gone"))
            continue
        if marker.encode() not in path.read_bytes():
            missing.append((rel, what, f"no {marker!r} marker"))

    for rel, (want, unwanted, what) in sorted(FIXES.items()):
        path = ROOT / rel
        if not path.is_file():
            missing.append((rel, what, "file is gone"))
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        if want not in text:
            missing.append((rel, what, f"fix is gone: {want!r} not found"))
        elif unwanted is not None and unwanted in text:
            missing.append((rel, what, f"bug is back: {unwanted!r} present"))

    for rel, what, why in missing:
        print(f"FAIL  {rel}: {why}")
        print(f"      PBSD change here: {what}")

    if missing:
        print(f"\n{len(missing)} of {len(MARKS) + len(FIXES)} PBSD vendor edits lost.")
        print("An upstream merge takes upstream's side on these without a")
        print("conflict. Recover the hunks from the commit before the merge.")
        return 1

    print(f"PBSD vendor edits intact — {len(MARKS)} markers, {len(FIXES)} fixes.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
