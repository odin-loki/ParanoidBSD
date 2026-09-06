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
    "hbsd/src/sys/vm/vm_mmap.c": [
        (
            "#if defined(__LP64__) && defined(PAX_HARDENING)",
            "#if defined(MAP_32BIT) && defined(PAX_HARDENING)",
            "MAP_32BIT ASLR call sites match where the delta exists",
        ),
        (
            "int error = 0, lastvecindex, mincoreinfo, vecindex;",
            "int error, lastvecindex, mincoreinfo, vecindex;",
            "kern_mincore() returned an uninitialised int to userland for "
            "mincore(addr, 0, vec) on a page-aligned map entry start, which "
            "skips both loops that assign it",
        ),
    ],
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
    "hbsd/src/sys/kern/kern_procctl.c": [
        (
            "int d = PROC_PROTMAX_NOFORCE;",
            None,
            "protmax_status() switched on a TWO-bit mask with THREE arms "
            "and copied d to userland with *(int *)data, so both bits set "
            "would be a kernel stack disclosure through procctl(2)",
        ),
        (
            "int d = PROC_ASLR_NOFORCE;",
            None,
            "aslr_status(), the same shape, reporting the ASLR state",
        ),
    ],
    # Two header macros that shift into bit 31 of a signed int. Found by
    # CBMC once report.py stopped hiding arithmetic failures behind
    # pointer ones in the same record. See docs/security/UB_FINDINGS.md.
    "hbsd/src/sys/arm64/include/cpu.h": (
        "#define\tCPU_IMPL_MASK\t(0xffU << 24)",
        "#define\tCPU_IMPL_MASK\t(0xff << 24)",
        "0xff << 24 is UB and NEGATIVE as an int, so CPU_MATCH()'s two "
        "sides sign-extend differently and it cannot match any ARM "
        "implementer >= 0x80 - CPU_IMPL_AMPERE is 0xC0",
    ),
    "hbsd/src/sys/dev/psci/smccc.h": (
        "(((uint32_t)(type) << 31) |",
        "(((type) << 31) |",
        "1 << 31 is UB, and the negative int it produces sign-extends "
        "in psci_call()'s register_t parameter, so the SMCCC function "
        "identifier handed to the secure monitor is 0xffffffff80000000",
    ),
    "hbsd/src/sys/i386/pci/pci_cfgreg.c": (
        "pcie_badslots |= (1U << slot);",
        "pcie_badslots |= (1 << slot);",
        "PCI_SLOTMAX is 31 and pcie_init_badslots() walks 0..31, so "
        "`1 << 31` on a signed int is executed on every PCIe boot - a "
        "shift of a VARIABLE at run time, unlike the 977 folded constants "
        "left alone in the same class",
    ),
    "hbsd/src/sys/kern/posix4_mib.c": (
        "\tif (P31B_VALID(num)) {\n\t\tfacility[num - 1] = 0;",
        None,
        "p31b_unsetcfg() was the only one of the file's four functions "
        "indexing facility[num - 1] without P31B_VALID, and the only one "
        "that writes without reading first; it is exported in "
        "sys/sys/posix4.h and num == 0 gives facility[-1]",
    ),
    "hbsd/src/sys/arm/ti/clk/ti_clk_dpll.c": [
        (
            "uint64_t cur, best = 0;",
            # NOT "uint64_t cur, best;" as the must-not-appear: the same
            # declaration is in ti_dpll_clk_set_freq() forty lines down,
            # which assigns `best = cur = 0;` on the next line and is
            # fine. The gate caught that on its first run, which is what
            # it is for; the `want` string alone is unique and a merge
            # that reverts the fix removes it.
            None,
            "ti_dpll_clk_find_best() read best at its first comparison "
            "before assigning it, and returned it uninitialised when "
            "either loop had zero iterations",
        ),
        (
            "\t\tif (p == 0) {\n\t\t\tp++;\n\t\t\tcontinue;\n\t\t}",
            None,
            "p is a divisor and ti_clk_factor_get_min() returns 0 for a "
            "TI_CLK_FACTOR_ZERO_BASED factor, so a zero-based p started "
            "the loop at a divisor of zero",
        ),
    ],
    "hbsd/src/sys/fs/nfsserver/nfs_nfsdsubs.c": (
        "if (nfsrv_clienthashsize <= 0) {",
        None,
        "nfsd_init() took four CTLFLAG_RDTUN hash sizes from loader.conf "
        "and validated none of them; three are used as a modulus, so "
        "vfs.nfsd.clienthashsize=0 is a kernel divide by zero the first "
        "time a client connects",
    ),
    # Two GEOM tasters, which run on whatever medium is plugged in.
    "hbsd/src/sys/geom/part/g_part_ldm.c": [
        (
            "db->dh.size == 0 || db->dh.last_seq == 0 ||",
            None,
            "ldm_vmdbhdr_check() rejected an on-disk dh.size of 0 and not "
            "last_seq, and ldm_vmdb_parse() does `size -= 1` on "
            "howmany(last_seq * size, sectorsize) - so last_seq == 0 made "
            "the read loop's size_t bound SIZE_MAX",
        ),
        (
            "u_char *buf = NULL, *p;",
            None,
            "`fail:` frees buf unconditionally and the loop that assigns "
            "it can have zero iterations",
        ),
    ],
    "hbsd/src/sys/geom/linux_lvm/g_linux_lvm.c": (
        "\tbzero(&ll, sizeof(ll));\n\tbzero(&md, sizeof(md));",
        None,
        "g_llvm_taste() zeroed md and not ll, and llvm_label_decode() has "
        "early returns above the assignment to the ll_md_offset that "
        "g_llvm_read_md() reads",
    ),
    # Four uninitialised returns, three of them reachable, all found by
    # clang's core.uninitialized.UndefReturn. See docs/security/UB_FINDINGS.md.
    "hbsd/src/sys/net/if.c": (
        "\t\terror = 0;\n\t\tCK_STAILQ_FOREACH(ifgl, &ifp->if_groups, ifgl_next) {",
        None,
        "if_getgroup() set error only inside a loop an interface with no "
        "groups never enters, and error is SIOCGIFGROUP's errno",
    ),
    "hbsd/src/sys/kern/vfs_lookup.c": (
        "int error = 0, crosslkflags;",
        "int error, crosslkflags;",
        "vfs_lookup_cross_mount() has two `continue`s above its only "
        "assignment to error, so an iteration taking either and then "
        "leaving the loop returned garbage into namei()",
    ),
    "hbsd/src/sys/fs/p9fs/p9fs_vnops.c": (
        "\t    ret = P9PROTO_OREAD;\n\t    break;\n\t}",
        None,
        "p9fs_uflags_mode() switched on a two-bit value with three cases; "
        "an O_EXEC or O_PATH descriptor gives (OFLAGS(0) & 3) == 3 and put "
        "an uninitialised open mode on the 9P wire",
    ),
    "hbsd/src/sys/kern/kern_event.c": (
        "int error = 0, i;\t/* PBSD: as kevent11_copyout() above */",
        None,
        "the two COMPAT_FREEBSD11 k_copyops assigned error only inside "
        "their loop, where their two non-compat siblings handle count == 0 "
        "by construction",
    ),
    # Three divisions by zero, all the same shape: a value that is 0 to
    # mean ABSENT used as a divisor by code that reads it as SMALL.
    # See docs/security/UB_FINDINGS.md.
    "hbsd/src/sys/netinet/igmp.c": (
        "if (inm->inm_timer != 0)\n\t\t\t\ttimer = min(inm->inm_timer, timer);",
        "inm_clear_recorded(inm);\n\t\t\ttimer = min(inm->inm_timer, timer);",
        "inm_timer == 0 means the timer is STOPPED, so min()ing with it "
        "made IGMP_RANDOM_DELAY's `random() % (X)` divide by zero, from "
        "a remote IGMPv3 group query",
    ),
    "hbsd/src/sys/netinet6/mld6.c": (
        "if (inm->in6m_timer != 0)\n\t\t\t\ttimer = min(inm->in6m_timer, timer);",
        "in6m_clear_recorded(inm);\n\t\t\ttimer = min(inm->in6m_timer, timer);",
        "the same defect in the IPv6 twin, which the analyser did not "
        "report because it explores paths and not classes",
    ),
    "hbsd/src/sys/net/route/nhgrp_ctl.c": (
        "if (xmin == 0)\n\t\treturn (0);",
        None,
        "all-zero weights made `total % xmin` divide by zero, and "
        "alloc_nhgrp()'s own \"Zero weights, abort\" branch could never "
        "be reached because the divide came first",
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
    "hbsd/src/sys/netinet/tcp_stacks/bbr.c": (
        "if (rtt != 0)\n\t\t\t\tgoto measure;",
        "rtt = bbr_get_rtt(bbr, BBR_SRTT);\n\t\t\tgoto measure;",
        "`measure:` is inside the `if (rtt && ...)` guard and this goto "
        "entered it having tested t_srtt instead",
    ),
    "hbsd/src/sys/geom/raid3/g_raid3.c": (
        "if (md->md_all < 2)",
        "if (md->md_all < 1)",
        "md_all is a uint16_t off the medium and `sc_ndisks - 1` is a "
        "divisor in fifteen places, two of them inside "
        "g_raid3_check_metadata() itself; the bound was < 1",
    ),
    "hbsd/src/sys/geom/eli/g_eli.h": (
        "eli_metadata_sectorsize_supported",
        None,
        "sc_data_per_sector can be zero and is the divisor for "
        "sc_bytes_per_sector; md(4) allows any power-of-two sector size",
    ),
    "hbsd/src/sys/geom/eli/g_eli.c": (
        "if (!eli_metadata_sectorsize_supported(md, bpp->sectorsize))",
        None,
        "the call site, in g_eli_create() rather than its two callers, "
        "because the crypto check there is only a KASSERT",
    ),
    "hbsd/src/sys/cam/cam_queue.c": [
        (
            "if (new_size > CAM_MAX_DEV_OPENINGS)",
            "int delta;\n\n\tdelta = new_size -",
            "cam_ccbq_resize() computes 1 << fls(n + n/2) from an int a "
            "userland ccb supplies through XPT_REL_SIMQ; the caller "
            "bounded it below and not above",
        ),
        (
            "if (openings > CAM_MAX_DEV_OPENINGS)",
            None,
            "cam_ccbq_init() is the twin of cam_ccbq_resize() with the "
            "identical expression, exported beside it",
        ),
        (
            "if (size < 0 || size == INT_MAX)",
            None,
            "camq_init()'s `size + 1` is UB at INT_MAX; exported, same "
            "file, same class",
        ),
    ],
    "hbsd/src/sys/cam/cam_queue.h": (
        "#define\tCAM_MAX_DEV_OPENINGS\t65536",
        None,
        "the bound the three cam_queue.c clamps use",
    ),
    "hbsd/src/sys/geom/raid/md_promise.c": (
        "meta->total_disks == 0 || meta->total_disks > PROMISE_MAX_DISKS",
        "width = vol->v_disks_count / 2;\n\t\tdisk_pos",
        "total_disks is a uint8_t off the medium and the check had only an "
        "upper bound; promise_meta_translate_disk() then divides and mods "
        "by total_disks/2 for RAID1E",
    ),
    "hbsd/src/sys/geom/virstor/g_virstor.c": [
        (
            "comp->chunk_count > 0 ? 100 -",
            "100-(used * 100) / count",
            "three divisions in g_virstor_dumpconf() guarded on the "
            "numerator instead of the divisor; kern.geom.confxml is "
            "world-readable",
        ),
        (
            "if (sc->map[n].flags & VIRSTOR_MAP_ALLOCATED)\n\t\t\t\tcount++;",
            "sc->map[n].flags || VIRSTOR_MAP_ALLOCATED != 0",
            "`||` where `&` was meant: VIRSTOR_MAP_ALLOCATED is 1, so the "
            "condition was constant-true and the INVARIANTS allocation "
            "count was always chunk_count",
        ),
    ],
    "hbsd/src/lib/libc/rpc/rpc_prot.c": (
        "NOT unreachable, and this used to say NOTREACHED",
        "assert(0);",
        "rj_stat is decoded off the wire and has two valid values; a third "
        "reached assert(0), and libc is not built -DNDEBUG, so a peer's "
        "reply aborted the client",
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

    nfixes = 0
    for rel, entry in sorted(FIXES.items()):
        # One file can carry more than one independent PBSD fix, so a value
        # may be a single (want, unwanted, what) or a list of them. It went
        # this way the first time sys/vm/vm_mmap.c needed a second.
        entries = entry if isinstance(entry, list) else [entry]
        nfixes += len(entries)
        path = ROOT / rel
        if not path.is_file():
            missing += [(rel, w, "file is gone") for _, _, w in entries]
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for want, unwanted, what in entries:
            if want not in text:
                missing.append((rel, what, f"fix is gone: {want!r} not found"))
            elif unwanted is not None and unwanted in text:
                missing.append((rel, what, f"bug is back: {unwanted!r} present"))

    for rel, what, why in missing:
        print(f"FAIL  {rel}: {why}")
        print(f"      PBSD change here: {what}")

    if missing:
        print(f"\n{len(missing)} of {len(MARKS) + nfixes + len(PBSD_FILES)}"
              " PBSD items in the vendor tree lost.")
        print("An upstream merge takes upstream's side on an edited file")
        print("without a conflict, and a re-import can drop an added one.")
        print("Neither says anything. Recover from the commit before it.")
        return 1

    print(f"PBSD vendor edits intact — {len(MARKS)} markers, {nfixes} "
          f"fixes, {len(PBSD_FILES)} added files.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
