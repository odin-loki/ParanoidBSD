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
    "hbsd/src/lib/msun/Makefile": (
        "ARCH_SRCS:=  ${ARCH_SRCS:N${i}}",
        None,
        "make WITHOUT_MACHDEP_OPTIMIZATIONS actually drop msun's assembly",
    ),
    "hbsd/src/sys/i386/i386/machdep.c": (
        "int i386_read_exec = 0;",
        None,
        "the definition pmap.c needs and nothing in the tree supplies",
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
    # Three memory-safety fixes in the netlink RPC transport, found by
    # clang's analyser and each reproduced on the single file before and
    # after. See docs/security/UB_FINDINGS.md.
    "hbsd/src/sys/netlink/netlink_snl.h": (
        "ss->init_done = false;",
        None,
        "snl_free() is idempotent; snl_init() calls it and callers call it "
        "again, which closed the fd twice and freed ss->buf twice",
    ),
    "hbsd/src/lib/libc/rpc/svc_nl.c": (
        "struct nl_request_parsed req = {};",
        None,
        "the parser writes only present attributes; an absent body left "
        "req.data a garbage pointer that NLA_DATA_LEN() dereferenced",
    ),
    "hbsd/src/usr.bin/genl/parser_rpc.c": (
        "struct nl_request_parsed req = {};",
        None,
        "same uninitialised parse target, in genl(1)",
    ),
    # Five more from the second sweep. Same discipline: reproduced on the
    # single file, read against the code that establishes the precondition,
    # confirmed gone. See docs/security/UB_FINDINGS.md.
    "hbsd/src/sys/geom/gate/g_gate.c": (
        "unit == G_GATE_NAME_GIVEN && name != NULL",
        None,
        "a KASSERT is not a check: gctl_unit = G_GATE_NAME_GIVEN with a "
        "NULL name reached strcmp(NULL, ...) from three ioctls",
    ),
    "hbsd/src/sys/netgraph/netflow/ng_netflow.c": (
        "if (resp == NULL)",
        None,
        "three M_NOWAIT NG_MKRESPONSE results dereferenced unchecked; the "
        "fourth in the same file checks",
    ),
    "hbsd/src/sys/fs/p9fs/p9_protocol.c": (
        "if (wnames == NULL) {",
        None,
        "nwname is a uint16_t off the 9P wire; a failed M_NOWAIT malloc "
        "then wrote through NULL for i in [0, nwname)",
    ),
    "hbsd/src/sys/dev/enic/vnic_dev.c": (
        "if (r == NULL) {",
        None,
        "device registers read straight into an unchecked M_NOWAIT "
        "allocation",
    ),
    "hbsd/src/sys/dev/usb/net/uhso.c": (
        "IFCOUNTER_IQDROPS",
        None,
        "mtod() on an unchecked m_getcl(M_NOWAIT); usbd_copy_out() then "
        "wrote actlen bytes of device data through NULL",
    ),
    "hbsd/src/sys/arm64/arm64/identcpu.c": (
        "if (prev_desc != NULL) {",
        None,
        "the guard tested the CPU INDEX and used the POINTER; with CPU 0 "
        "absent from all_cpus, check_cpu_regs() dereferences NULL",
    ),
}


# Files PBSD ADDED to the vendor tree. Not edits - these exist on our side
# and nowhere upstream, so a merge cannot eat them. A re-import can, and has:
# the flat import this tree started from lost 124 vendor files and every one
# of them cost a build run to find. Checking they are present is one stat
# each.
PBSD_FILES = {
    "hbsd/src/sys/sys/atomic_generic.h":
        "generic atomic(9); tools/atomic_generic_check.py measures it",
    "hbsd/src/sys/sys/_stdint_generic.h":
        "generic <machine/_stdint.h>",
    "hbsd/src/sys/sys/_inttypes_generic.h":
        "generic <machine/_inttypes.h>",
    "hbsd/src/sys/conf/std.hardenedbsd":
        "the hardening policy all six kernel configs include",
    "hbsd/src/sys/conf/std.hardenedbsd.debug":
        "WITNESS and HBSD_DEBUG, kept apart from the policy",
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

    for rel, what in sorted(PBSD_FILES.items()):
        if not (ROOT / rel).is_file():
            missing.append((rel, what, "PBSD added this file and it is gone"))

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
        print(f"\n{len(missing)} of {len(MARKS) + len(FIXES) + len(PBSD_FILES)}"
              " PBSD items in the vendor tree lost.")
        print("An upstream merge takes upstream's side on an edited file")
        print("without a conflict, and a re-import can drop an added one.")
        print("Neither says anything. Recover from the commit before it.")
        return 1

    print(f"PBSD vendor edits intact — {len(MARKS)} markers, {len(FIXES)} "
          f"fixes, {len(PBSD_FILES)} added files.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
