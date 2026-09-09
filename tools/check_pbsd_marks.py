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
    # The five .cpp wrappers around this body are compiled as C++ by
    # lib/msun/Makefile:86 and :88, and C++ means libc++'s <math.h>,
    # which reaches <__type_traits/enable_if.h> and its `typedef _Tp
    # type;'. A macro called `type' rewrites that line. Renamed here and
    # in the five includers - the complete set, nothing else includes
    # this file - so a vendor resync that puts `type' back is a failure
    # rather than a build that stops on a header nobody edited.
    "hbsd/src/lib/msun/src/s_lround.c": (
        "#ifndef ftype\n#define ftype\t\tdouble",
        "#define type\t\tdouble",
        "s_lround.c's macro is ftype, not type: `type' collides with "
        "libc++ when the five .cpp wrappers compile it as C++",
    ),
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
    "hbsd/src/sys/powerpc/pseries/phyp_vscsi.c": [
        (
            "return (ENOMEM);",
            "return (ENOMEM)\n",
            "missing semicolon; the file has never been compiled upstream",
        ),
        (
            "allocate buffer\\n\");\n\t\tmtx_unlock(&sc->io_lock);",
            "allocate buffer\\n\");\n\t\treturn (ENOMEM);",
            "vscsi_attach() returned holding sc->io_lock on the M_NOWAIT "
            "failure path",
        ),
    ],
    # Twelve more, all one shape: a lock this function releases on every
    # other way out, and not on this one. lock_balance.py found them and
    # reading confirmed every one; the tool's own test asserts they stay
    # reported as fixed, and these entries are what survives a vendor
    # merge that drops the hunk without dropping the file.
    "hbsd/src/sys/netipsec/ipsec.c": (
        "if (th == 0) {\n\t\t\tSECREPLAY_UNLOCK(replay);",
        "if (th == 0)\n\t\t\treturn (0);",
        "ipsec_chkreplay() returned holding replay->lock on the th == 0 "
        "arm; ipsec_updatereplay() twenty lines down has the same block "
        "with the unlock in it",
    ),
    "hbsd/src/sys/dev/drm2/drm_bufs.c": (
        ("mtx_unlock(&dev->pcir_lock);\n\t\treturn 0;", 2),
        "if (drm_alloc_resource(dev, resource) != 0)\n\t\treturn 0;",
        "drm_get_resource_start() and drm_get_resource_len() both "
        "returned holding dev->pcir_lock; the count is 2 because the two "
        "are the same three lines copied",
    ),
    "hbsd/src/sys/arm/allwinner/aw_mmc.c": (
        "active command\\n\");\n\t\tAW_MMC_UNLOCK(sc);",
        "active command\\n\");\n\t\treturn (EBUSY);",
        "aw_mmc_cam_request() returned EBUSY holding the softc lock when "
        "a CAM request arrived with one already in flight",
    ),
    "hbsd/src/sys/arm/nvidia/drm2/tegra_bo.c": (
        "!= 0) {\n\t\t\tVM_OBJECT_WUNLOCK(bo->cdev_pager);",
        "&pages) != 0)\n\t\t\treturn (EINVAL);",
        "tegra_bo_init_pager() returned holding the object write lock it "
        "drops two lines below",
    ),
    "hbsd/src/sys/arm64/nvidia/tegra210/max77620_gpio.c": (
        # Two arms in this file carry that unlock now: the one at :530
        # always did, and the one below it did not. Five arms print the
        # same message, so "the message then return" matches three that
        # are correct - the unwanted string has to reach the line AFTER
        # the return to name only the one that was wrong.
        ("GIPO_CFG register\\n\");\n\t\tGPIO_UNLOCK(sc);", 2),
        "GIPO_CFG register\\n\");\n\t\treturn (ENXIO);\n\t}\n"
        "\tif (old_reg_pue != sc->gpio_reg_pue) {",
        "one of five identical error arms in "
        "max77620_gpio_pin_setflags() did not unlock",
    ),
    "hbsd/src/sys/dev/sound/pci/ich.c": (
        "default:\n\t\tICH_UNLOCK(sc);\n\t\treturn (NULL);",
        "default:\n\t\treturn (NULL);",
        "ichchan_init()'s default: arm returned holding ICH_LOCK",
    ),
    "hbsd/src/sys/netpfil/ipfw/ip_fw_table.c": (
        "if (ta->find_tentry == NULL) {\n\t\tIPFW_UH_RUNLOCK(ch);",
        "if (ta->find_tentry == NULL)\n\t\treturn (ENOTSUP);",
        "find_table_entry() returned holding IPFW_UH_RLOCK when a table "
        "algorithm has no find_tentry method",
    ),
    "hbsd/src/sys/powerpc/mpc85xx/fsl_espi.c": (
        "giving up.\\n\");\n\t\tFSL_ESPI_UNLOCK(sc);",
        "giving up.\\n\");\n\t\treturn (EINVAL);",
        "fsl_espi_transfer() returned holding the controller lock when "
        "the platform clock reads zero",
    ),
    "hbsd/src/sys/dev/pci/pci.c": (
        "if (b < (int)nitems(dw))",
        "\t\t\tdw[b] = REG(ptr, 4);\n",
        "pci_ea_fill_info() indexed uint32_t dw[4] with a three-bit "
        "entry size read out of the device's own config space",
    ),
    "hbsd/src/sys/dev/cardbus/cardbus_cis.c": (
        "int rid = 0;",
        "\tstruct resource *res;\n\tint rid;\n",
        "cardbus_read_tuple_init() returns the CIS_CONFIG_SPACE sentinel "
        "without writing *rid, and the caller passed it by value",
    ),
    "hbsd/src/sbin/ping/ping6.c": (
        "static struct iovec iov[2];",
        "struct icmp6_hdr *icp;\n\tstruct iovec iov[2];",
        "pinger() left the file-scope smsghdr.msg_iov pointing into its "
        "own dead frame",
    ),
    "hbsd/src/bin/sh/eval.c": (
        "pip[0] = pip[1] = -1;",
        "prehash(lp->n);\n\t\tpip[1] = -1;\n",
        "evalpipe() read pip[0] on the last element of the pipeline, "
        "where pipe() is not called and never wrote it",
    ),
    "hbsd/src/usr.sbin/rpc.lockd/lockd_lock.c": (
        "deallocate_file_lock(*left_lock);\n\t\t\t\t/*",
        "deallocate_file_lock(*left_lock);\n\t\t\t}\n\t\t\treturn "
        "SPL_RESERR;",
        "split_nfslock() freed *left_lock and left the pointer behind, "
        "so unlock_nfslock() read it and freed it a second time",
    ),
    "hbsd/src/usr.sbin/ppp/datalink.c": (
        "char *name = realloc(dl->name, DATALINK_MAXNAME);",
        "iov[*niov].iov_base = dl ? realloc(dl->name, DATALINK_MAXNAME)"
        " : NULL;",
        "datalink2iov() put the realloc() result in the iovec and not "
        "back in dl->name, so the error path freed what realloc freed",
    ),
    "hbsd/src/usr.bin/patch/util.h": (
        ("__attribute__((noreturn));", 2),
        "__printf__, 1, 2)));\nvoid\t\tpfatal",
        "fatal() and pfatal() end in my_exit(), which this header "
        "already declares noreturn, and said so themselves nowhere -- "
        "so every path after a fatal() anywhere in patch(1) was "
        "analysed as though the program continued",
    ),
    "hbsd/src/usr.bin/gencat/gencat.c": (
        "while ((msg = set->msghead.lh_first) != NULL) {",
        "while (msg) {\n\t\t\tfree(msg->str);",
        "MCDelSet()'s loop never advanced msg -- LIST_REMOVE unlinks it "
        "and does not change it -- so $delset for an existing set hung "
        "gencat and freed msg->str again on the second pass",
    ),
    "hbsd/src/usr.sbin/rtadvd/if.c": (
        ("free(ifi);\n", 3),
        "if (ifi_new)\n\t\t\t\t\tfree(ifi);\n\t\t\t\tcontinue;",
        "update_ifinfo() ends `return (ifi)' and three arms free ifi and "
        "continue, so a last iteration taking one of them returned a "
        "dangling pointer; the count is 3 because losing one of the "
        "three is the same bug",
    ),
    "hbsd/src/usr.sbin/mountd/mountd.c": (
        "if ((opt_flags & OP_MASKLEN) && prefp != NULL) {",
        "if (opt_flags & OP_MASKLEN) {\n\t\t\tpreflen = strtol(prefp",
        "get_net() tested the OP_MASKLEN flag rather than the string it "
        "claims, so `-network a/len -mask m' passed NULL to strtol() and "
        "wrote through a NULL p; check_options() rejects that pair at "
        ":3886, after the option line has been parsed",
    ),
    "hbsd/src/usr.sbin/rtadvd/config.c": (
        "delete_prefix(pfx);\n\t\treturn;",
        "delete_prefix(pfx);\n\t}\n\ttimo.tv_sec = prefix_timo;",
        "invalidate_prefix() fell through from delete_prefix(), which "
        "ends in free(pfx), into a dereference of pfx",
    ),
    "hbsd/src/sys/i386/i386/trap.c": (
        "if (usermode && (eva == (unsigned int)&idt[6]) &&",
        "if ((eva == (unsigned int)&idt[6]) && has_f00f_bug) {",
        "trap_pfault()'s F00F arm wrote through *ucode and *signo before "
        "the usermode test that guards every other write to them, and "
        "the kernel-mode caller at :481 passes NULL for both",
    ),
    "hbsd/src/sys/amd64/vmm/amd/svm.c": (
        "errcode_valid = 0;\n\t\tidtvec = code - 0x40;",
        "reflect = 1;\n\t\tidtvec = code - 0x40;",
        "the 0x40...0x5F exception arm reflects the exception into the "
        "guest with an errcode_valid that case IDT_DB never assigns - and "
        "IDT_DB clears reflect only for a TF single-step it is expecting, "
        "so a #DB from a guest debug register injected an exception with "
        "an uninitialised error-code-valid bit; vmx.c:2740 sets its "
        "equivalent unconditionally before the test that can raise it",
    ),
    "hbsd/src/lib/libc/nls/msgcat.c": (
        "#define\tTRY_WLOCK()",
        "\t\t\t\t\t\tWLOCK(NLERR);",
        "WLOCK()'s `return (fail)' fires in the middle of an ownership "
        "transfer to the cache - SAVEFAIL()'s entry, and in load_msgcat() "
        "the whole mmap'd catalogue with its five allocations - and the "
        "cache is where that ownership lives, so the failure path lost "
        "them; _pthread_rwlock_wrlock() on this statically-initialised "
        "rwlock really can fail, because libthr's rwlock_init() "
        "aligned_alloc()s on first use and returns ENOMEM",
    ),
    "hbsd/src/lib/libc/iconv/citrus_stdenc.c": (
        "if (ce == NULL)\n\t\treturn;",
        "{\n\n\tif (ce == &_citrus_stdenc_default)",
        "_citrus_stdenc_open()'s `bad:' label is reached from its own "
        "`ce = malloc(...); if (ce == NULL)' arm, so an allocation failure "
        "called _citrus_stdenc_close(NULL), which dereferenced it at once",
    ),
    "hbsd/src/lib/libc/rpc/pmap_prot2.c": (
        "next = (*rp)->pml_next;",
        "next = &((*rp)->pml_next);",
        "xdr_pmaplist() under XDR_FREE remembered the ADDRESS of the field "
        "inside the object xdr_reference() then free()d, so the next "
        "iteration read *rp out of freed memory and free()d what it found; "
        "rpcb_prot.c's two copies of this loop, in libc and in sys/rpc, "
        "both already carry the next_copy form and this one did not",
    ),
    "hbsd/src/lib/libc/gen/sysctl.c": (
        "if (oldlenp == NULL)\n\t\treturn (0);\n\n\tswitch (name[1]) {",
        "\t}\n\n\tswitch (name[1]) {\n\tcase USER_CS_PATH:",
        "sysctl(3) documents oldp and oldlenp both being NULL when the old "
        "value is not wanted, and of the three writes through oldlenp under "
        "CTL_USER only USER_LOCALBASE tested for it, so "
        "sysctl({CTL_USER, USER_BC_BASE_MAX}, 2, NULL, NULL, NULL, 0) "
        "stored through a null pointer from a public libc entry point",
    ),
    "hbsd/src/sys/modules/tpm/Makefile": (
        ".if !empty(OPT_FDT)",
        ".if defined(${OPT_FDT})",
        "bmake expands ${OPT_FDT} and then asks whether a variable of THAT "
        "NAME is defined, so the block was dead however OPT_FDT was set and "
        "tpm_spibus.c and tpm_tis_spibus.c were compiled by nothing on any "
        "architecture; twenty other modules and sys/conf/kern.opts.mk:221 "
        "spell the same test !empty(OPT_FDT)",
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
    "hbsd/src/sys/sys/pax.h": [
        (
            "#define\tpax_disallow_map32bit_active(td, flags)",
            None,
            "hbsd_pax_aslr.c is gated on pax_aslr and both call sites "
            "guard on a different option; boot run 54 died on it",
        ),
        (
            "#define\tpax_insecure_kmod()",
            None,
            "called under #ifdef HARDEN_KLD, which is not the "
            "pax_hardening its definition needs",
        ),
        (
            "#define\tpax_harden_tty(td)\t\t({ 0; })",
            None,
            "pax_harden_tty and pax_kmod_load_disabled are called from "
            "files compiled under plain `options PAX` while their "
            "definitions are gated on pax_hardening",
        ),
        (
            "#define\tpax_control_extattr_kmod(td, vp)\t((pax_flag_t)0)",
            None,
            "same, gated on pax_control_extattr and called from link_elf",
        ),
        (
            "#define\tpax_enforce_tpe(td, vn, path)\t({ 0; })",
            None,
            "same, called from vm_mmap.c under `#ifdef PAX` rather than "
            "`#ifdef PAX_HARDENING`",
        ),
    ],
    "hbsd/src/sys/netinet/tcp_stacks/rack.c": [
        (
            "if (high_rate && (fill_bw > high_rate)) {",
            None,
            "fill_bw is the divisor at `lentim /= fill_bw` and this was "
            "the one of the file's four hardware-rate caps that did not "
            "check the value it assigns",
        ),
        (
            "int32_t len = 0, error = 0;",
            None,
            "rack_output()'s `goto just_return_nolock` on an allocation "
            "failure is seven lines above len's first assignment, and "
            "two sites past the label read it",
        ),
        (
            "uint32_t if_hw_tsomaxsegsize = 0;",
            None,
            "three functions declared the TSO seg-count initialised and "
            "the seg-size not, and passed both to one call",
        ),
        (
            "\tuint64_t loptval = 0;\n\tint32_t error = 0, optval = 0;",
            None,
            "rack_set_sockopt()'s TCP_HYBRID_PACING arm sets neither, and "
            "rack_process_option() is passed both unguarded",
        ),
        (
            # NOT the panic() line itself: rack_init_outstanding() at
            # :14268 and :14328 already use that exact wording, so the
            # obvious marker matches two vendor sites as well as this
            # fix and would pass with the fix gone. The second time this
            # gate has caught an over-broad marker of mine.
            "rack_log_output() declares nrsm at :8131 and does not",
            None,
            "rack_log_output() printed nrsm, which it never assigns, in a "
            "panic message",
        ),
        (
            "\t\tif (rate_wanted == 0) {",
            "if (((bw_est == 0) || (rate_wanted == 0) || (rack->gp_ready == 0)) &&",
            "rack_get_pacing_delay: the zero test was one disjunct of a "
            "disjunction conjoined with `use_fixed_rate == 0', so a fixed "
            "rate switched the check OFF for the one case that produces a "
            "zero - and TCP_RACK_PACE_RATE_CA takes 0 from any user",
        ),
        (
            'fails ret:%d rack:%p rsm:%p",\n\t\t\t\t      nrsm, insret',
            "fails ret:% rack",
            "one of the nine copies of this panic had `ret:%` - a "
            "conversion with no specifier",
        ),
    ],
    "hbsd/src/sys/netinet/tcp_stacks/bbr.c": [
        (
            "if (rtt != 0)\n\t\t\t\tgoto measure;",
            "rtt = bbr_get_rtt(bbr, BBR_SRTT);\n\t\t\tgoto measure;",
            "`measure:` is inside the `if (rtt && ...)` guard and this "
            "goto entered it having tested t_srtt instead",
        ),
        (
            "\t\t\tdelta = 0;\n\t\t\trtt_gain = 0;",
            None,
            "the else arm assigned rtt_gain and not delta, and "
            "bbr_log_startup_event() reads both",
        ),
    ],
    # Two entries and not one, because the comment is the same in both
    # twins and `want in text' is satisfied by either. A single marker
    # here would stay silent when a merge ate one of the two -- which is
    # the guard-on-one-of-a-pair defect this file exists to catch,
    # committed in the file that catches it. Each names its function.
    "hbsd/src/sys/fs/nfsserver/nfs_nfsdport.c": (
        "PBSD: iv and cnt are out-parameters, so read them only",
        "\t\terror = nfsrv_createiovecw(len, m, cp, &iv, &cnt);\n"
        "\t\tuiop->uio_iov = iv;",
        "nfsvno_setxattr() stored nfsrv_createiovecw()'s out-parameters "
        "into the uio without checking the return, while nfsvno_write() "
        "- the only other caller - returns on error before touching them",
    ),
    "hbsd/src/sys/fs/nfsserver/nfs_nfsdserv.c": [
        (
            # Counting the PBSD comment and not "NFSVNO_ATTRINIT",
            # which this file already uses thirteen times for other
            # reasons -- a count-based gate over a string that is not
            # unique to the fix is satisfied by the pre-existing uses and
            # bites on nothing, which is how the first version of this
            # entry passed with one of the six sites deliberately removed.
            ("PBSD: see nfsrvd_remove() - change_info4 has no", 6),
            None,
            "seven NFSv4 handlers wrote dirfor/diraft na_filerev into "
            "change_info4 without consulting the dirfor_ret/diraft_ret "
            "flags the NFSv3 path honours, so a successful operation "
            "whose directory getattr failed sent the client eight bytes "
            "of stack per value -- ten sites, counted rather than tested "
            "for presence so losing one of them is not silent",
        ),
        (
            "PBSD: define the change_info4 attributes before any path can",
            None,
            "the long-form explanation in nfsrvd_remove() that the six "
            "short markers refer back to",
        ),
        (
            "PBSD: assign on this arm too, as the other three do",
            None,
            "nfsrvd_opendowngrade()'s access switch left ls_flags "
            "unassigned on its default arm and the deny switch below "
            "read-modify-writes it, with both selectors off the wire",
        ),
    ],
    "hbsd/src/sys/kern/subr_mchain.c": [
        (
            "PBSD: report m_copym()'s failure instead of returning 0",
            # No `unwanted': this fix only INSERTS. The tail it guards -
            # md_get_mem(); *ret = rm; return (0) - is unchanged by
            # design, so any marker drawn from it matches the fixed file
            # too. Third time today a bug-is-back marker has fired on
            # code that was correct; the rule is that `unwanted' must be
            # text the fix DELETES, and a purely additive fix deletes
            # nothing. Where there is nothing to delete, the positive
            # marker is the whole gate.
            None,
            "md_get_mbuf() discarded m_copym()'s NULL and returned 0 "
            "always, making the `if (error) goto freerq' at all four of "
            "its netsmb call sites dead code",
        ),
        (
            "PBSD: and be NULL-tolerant here too",
            None,
            "mb_put_mbuf() assigned NULL happily, tested for it in its "
            "loop, and then dereferenced it in M_TRAILINGSPACE()",
        ),
    ],
    "hbsd/src/sys/kern/kern_time.c": (
        "PBSD: cts is only readable when the gettime",
        "\t\t} else {\n\t\t\tts = it->it_time.it_value;\n"
        "\t\t\ttimespecsub(&ts, &cts, &ts);",
        "realtimer_expire_l() guards its first use of cts on `error == "
        "0' and its second on nothing, so a failed kern_clock_gettime() "
        "re-armed the callout from an indeterminate timespec",
    ),
    "hbsd/src/sys/kern/uipc_ktls.c": (
        "PBSD: record_type is written together with last_offset",
        "\tuint8_t record_type;\n",
        "tls13_find_record_type() returned record_type to the caller "
        "with only a test against tls->params.tls_hlen between an "
        "all-zero TLS record and a byte of stack",
    ),
    "hbsd/src/sys/netinet/sctp_pcb.c": [
        (
            "PBSD: SCTP_MALLOC is unconditionally M_NOWAIT",
            None,
            "sctp_startup_mcore_threads() indexed straight into an "
            "unchecked SCTP_MALLOC result; the wait flag is inside the "
            "macro, so no M_NOWAIT token appears at any of the tree's "
            "SCTP_MALLOC call sites",
        ),
        (
            "PBSD: same unchecked SCTP_MALLOC as",
            None,
            "sctp_pcb_init()'s SCTP_BASE_STATS allocation, live only "
            "under SCTP_USE_PERCPU_STAT which this tree never defines",
        ),
    ],
    "hbsd/src/sys/net80211/ieee80211_mesh.c": [
        (
            ("PBSD: M_NOWAIT returns NULL under memory pressure", 1),
            None,
            "two of this file's five IEEE80211_MALLOC sites dereferenced "
            "an M_NOWAIT result unchecked, both allocating the same "
            "struct on a path reached from a received 802.11 frame",
        ),
        (
            "PBSD: see ieee80211_mesh_mark_gate() - M_NOWAIT can fail",
            None,
            "the GANN action-frame handler's copy of the same unchecked "
            "allocation",
        ),
        (
            "PBSD: say so when the frame carried no GANN element",
            "\t\tfrm += frm[1] + 2;\n\t}\n\n\treturn 0;\n}",
            "mesh_parse_meshgate_action() returned success whether or "
            "not it found the element, so a GANN frame carrying none "
            "left the caller's stack struct untouched and had six bytes "
            "of kernel stack copied into the known-gates table and back "
            "onto the air",
        ),
    ],
    "hbsd/src/sys/net80211/ieee80211_hwmp.c": (
        # Counted, not tested for presence: there are two call sites and
        # losing the guard on either one restores the defect. The first
        # version of this entry used the unguarded assignment as the
        # `unwanted' marker and matched the FIXED line, because adding
        # the `if' left the RANN site at the indentation the marker was
        # written for.
        ("if (gr != NULL)", 2),
        None,
        "both callers of ieee80211_mesh_mark_gate() assigned through "
        "its result unchecked, on the PREQ and RANN frame paths",
    ),
    "hbsd/src/sys/netpfil/pf/pf.c": [
        (
            "PBSD: s is optional in pf_route() and every other use says so",
            "\tif (r->rt == PF_DUPTO || (pd->af != pd->naf && s->direction",
            "pf_route() uses the optional pf_kstate pointer eleven times "
            "and guarded ten of them; the eleventh sat between an "
            "`if (s != NULL)' and an MPASS(s != NULL)",
        ),
        (
            "PBSD: s is optional in pf_route6() and every other use says so",
            None,
            "pf_route6() is the same function for IPv6 and had the same "
            "census: eleven uses, the same one unguarded",
        ),
    ],
    "hbsd/src/sys/netpfil/pf/pf_ioctl.c": (
        "PBSD: only read old_limit when it was written",
        None,
        "DIOCSETLIMIT copied pf_ioctl_set_limit()'s out-parameter into "
        "the userland-visible buffer even on the EINVAL path that never "
        "writes it; not a leak only because kern_ioctl() gates copyout "
        "on error == 0",
    ),
    "hbsd/src/libexec/rtld-elf/rtld.c": [
        (
            "PBSD: phdyn gets the same NULL as its two siblings",
            None,
            "load_kpreload() left phdyn indeterminate while giving seg0 "
            "and segn a NULL, then dereferenced all three unchecked; a "
            "vdso with no PT_LOAD or no PT_DYNAMIC gave a NULL "
            "dereference or a wild read inside the run-time linker",
        ),
        (
            "PBSD: where starts NULL, and a bitmap entry that arrives",
            None,
            "reloc_relr() read `where' on the RELR bitmap branch and "
            "assigned it only on the address branch, so a DT_RELR whose "
            "first entry is a bitmap made the run-time linker WRITE "
            "through an indeterminate pointer, driven by the contents "
            "of the object being loaded",
        ),
        (
            "PBSD: first_seg was declared and never initialised",
            "\tbool first_seg;\n",
            "parse_rtld_phdr() read an indeterminate bool to decide "
            "whether a PT_LOAD was the first segment, so obj->vaddrbase "
            "was set or skipped according to a stack byte; digest_phdr() "
            "sixty lines away initialises the same counter",
        ),
        (
        "PBSD: read the PaX flags AFTER the vector has been digested",
        "aux = (Elf_Auxinfo *)sp;\n\n\n#ifdef HARDENEDBSD",
        "THE BOOT BUG. _rtld() read aux_info[AT_PAXFLAGS] before either "
        "loop had written the array, then dereferenced and stored "
        "through it. Reading an indeterminate automatic is undefined "
        "behaviour, and clang 18 at -O2 deletes the whole function on "
        "that basis: _rtld was 32 bytes in boot run 56's ld-elf.so.1 and "
        "13,136 in run 57's, which is the first run to reach multi-user. "
        "Lose this line and /sbin/init stops booting",
        ),
    ],
    "hbsd/src/sys/arm64/nvidia/tegra210/tegra210_xusbpadctl.c": [
        (
            ("if (port != NULL && port->supply_vbus != NULL)", 6),
            # No `unwanted': `if (port->supply_vbus != NULL) {' is still
            # correct at :1016, in a function that reached port another
            # way. The count is the marker - six of them, and reverting
            # any one is caught.
            None,
            "search_lane_port() can return NULL and the block above each "
            "of these says so - it prints \"Cannot find port for lane\" "
            "and falls through without returning. "
            "tegra124_xusbpadctl.c, which this file is the Tegra210 copy "
            "of, writes every one as `if (port != NULL && "
            "port->supply_vbus != NULL)'. The newer copy lost the first "
            "half at four sites and two more in their error paths",
        ),
    ],
    "hbsd/src/sys/arm/ti/ti_sysc.c": [
        (
            "PBSD: err is assigned only inside the loop",
            "\tstruct ti_sysc_softc *sc = device_get_softc(dev);\n\tint err;\n",
            "ti_sysc_clock_enable() returns err unassigned for an empty "
            "clk_list, to a caller that reads it as success or failure. "
            "ti_sysc_clock_disable(), twenty lines below and otherwise "
            "identical, already says `int err = 0;'",
        ),
    ],
    "hbsd/src/sys/dev/vnic/nicvf_queues.c": [
        (
            ("_cfg = { 0 };", 4),
            "\tstruct cq_cfg cq_cfg;\n",
            "all four queue-config structs are written whole to a "
            "hardware register with *(uint64_t *)&x, and none of their "
            "reserved_* bitfields is ever assigned, so what reached the "
            "NIC's reserved bits was whatever the stack held. Linux's "
            "thunder driver declares these as a union and opens with "
            "`.value = 0'; the FreeBSD port dropped the union and the "
            "zeroing with it",
        ),
    ],
    "hbsd/src/sys/dev/regulator/regulator.c": [
        (
            "PBSD: udelay is an OUT cell and every driver writes it",
            "\trv = REGNODE_SET_VOLTAGE(regnode, min_uvolt, max_uvolt, "
            "&udelay);\n\tregnode_delay(udelay);\n",
            "REGNODE_SET_VOLTAGE has three call sites in the tree and "
            "one of them checked rv before using the OUT cell. Every "
            "driver writes *udelay only on its success path, so the "
            "other two called regnode_delay() on an indeterminate int - "
            "a DELAY() for as long as a stack word says",
        ),
    ],
    "hbsd/src/sys/dev/iicbus/pmic/rockchip/rk8xx_regulators.c": [
        (
            "PBSD: rv first. rk8xx_regnode_set_voltage() writes",
            "\t    param->max_uvolt, &udelay);\n\tif (udelay != 0)\n",
            "the third of those three call sites: it checked neither "
            "the return nor anything else, and rk8xx_regnode_set_voltage"
            "() returns ENXIO for a regulator with no voltage step and "
            "ERANGE for a request it cannot meet, both before writing "
            "the cell",
        ),
    ],
    # The same family, one SoC over. axp81x's call site was worse than
    # rk8xx's: it DELAY()ed on `rv != 0' - exactly and only the paths
    # where axp8xx_regnode_set_voltage() returns before writing *udelay.
    "hbsd/src/sys/arm/allwinner/axp81x.c": [
        (
            "PBSD: rv == 0, not rv != 0.",
            "\t    param->max_uvolt, &udelay);\n\tif (rv != 0)\n",
            "axp8xx_regnode_init() read a stack slot on every path where "
            "axp8xx_regnode_set_voltage() had returned ENXIO or ERANGE "
            "without writing it, and DELAY() busy-waits for what it is "
            "given",
        ),
    ],
    # The same driver twice: vf_sai.c is imx6_ssi.c one SoC over, down
    # to the dead `err = bus_dma_tag_create(...)'.
    "hbsd/src/sys/arm/freescale/imx/imx6_ssi.c": [
        (
            "PBSD: err, checked. This assignment was dead",
            "\t    &sc->dma_tag);\n\n\terr = bus_dmamem_alloc(",
            "bus_dma_tag_create()'s return was overwritten before "
            "anything read it, and that function opens with "
            "*dmat = NULL, so a failure handed NULL to "
            "bus_dmamem_alloc()",
        ),
    ],
    "hbsd/src/sys/arm/freescale/vybrid/vf_sai.c": [
        (
            "PBSD: err, checked. This assignment was dead",
            "\t    &sc->dma_tag);\n\n\terr = bus_dmamem_alloc(",
            "the same dead assignment as imx6_ssi.c, in the driver it "
            "was copied from or to",
        ),
    ],
    # Three drivers whose `timeout' is assigned only inside the loop
    # that a zero-length transfer skips, and read after it.
    "hbsd/src/sys/arm/mv/gpio.c": [
        (
            "PBSD: allocate where it is used, check M_NOWAIT",
            "\ts = malloc(sizeof(struct mv_gpio_pindev), M_DEVBUF, M_NOWAIT | M_ZERO);\n\n\tif (pin < 0",
            "the pindev was allocated at the top of "
            "mv_gpio_setup_intrhandler() and intr_event_create() is "
            "its only consumer, so it leaked on the bounds check, on "
            "both failing returns, and on the ordinary success path "
            "whenever the pin already had an event; a NULL from the "
            "M_NOWAIT malloc went through as the cookie the three "
            "mv_gpio_intr_* callbacks dereference",
        ),
    ],
    "hbsd/src/sys/arm/ti/ti_adc.c": [
        (
            "PBSD: bounded by the buffer, and the FIFO still drained.",
            "\t\tdata[i++] = ADC_READ4(sc, ADC_FIFO1DATA) & ADC_FIFO_DATA_MSK;\n",
            "ti_adc_tsc_read_data() filled a 16-word stack array from "
            "the hardware FIFO with no bound at all - the count "
            "register is masked with 0x7f, so up to 127 words",
        ),
        (
            "PBSD: bounded. This was taken from the device",
            "\t\t    sizeof(cell))) > 0)\n\t\t\tsc->sc_coord_readouts = cell;\n",
            "ti,coordinate-readouts came from the device tree "
            "unchecked and is used as an index base into that same "
            "16-word buffer over [n + 2, 2n + 2), and at "
            "ti_adc_setup() as ADC_STEPS - (n * 2 + 2) + 1",
        ),
    ],
    "hbsd/src/sys/arm/mv/mvebu_gpio.c": [
        (
            "PBSD: read the pin here rather than through",
            "\tGPIO_LOCK(sc);\n\tmvebu_gpio_pin_get(sc->dev, pin, &val);\n",
            "mvebu_gpio_pin_toggle() held GPIO_LOCK and called "
            "mvebu_gpio_pin_get(), which takes it again; the mutex is "
            "MTX_DEF, so every GPIOTOGGLE on this controller panicked "
            "on a non-recursive mutex",
        ),
    ],
    "hbsd/src/sys/arm/ti/am335x/am335x_pwmss.c": [
        (
            "PBSD: `id' had no default and is read two lines down",
            "\t\tid = 2;\n\t\tbreak;\n\t}\n",
            "an unrecognised PWMSS revision from the device tree left "
            "id unset and `reg |= (1 << id)' shifted by a stack value "
            "into SCM_PWMSS_CTRL",
        ),
    ],
    "hbsd/src/sys/arm/ti/clk/ti_divider_clock.c": [
        (
            "PBSD: ti_max_div was assigned only here",
            "\t\tti_max_div = value;\n\t}\n\n\tif (OF_hasprop(node, \"clock-output-names\"))",
            "a ti,divider-clock node without ti,max-div took a stack "
            "slot as fls()'s argument and so as the bit width of a "
            "clock divider field; 16 of the 200 such nodes in the "
            "shipped device trees have no ti,max-div",
        ),
    ],
    "hbsd/src/sys/arm/mv/mv_spi.c": [
        (
            "PBSD: nonzero, which is what \"did not time out\" reads as",
            "\tint resid, timeout;\n",
            "mv_spi_transfer() ends `return ((timeout == 0) ? EIO : 0);' "
            "and assigns timeout only inside a loop bounded by the "
            "bytes left to send",
        ),
    ],
    "hbsd/src/sys/arm/mv/a37x0_spi.c": [
        (
            "PBSD: nonzero, which is what \"did not time out\" reads as",
            "\tint timeout;\n\n\tsc = device_get_softc(dev);",
            "the same shape as mv_spi.c, in the same directory",
        ),
    ],
    "hbsd/src/sys/arm/ti/ti_i2c.c": [
        (
            "PBSD: 0, the value the normal flow leaves here.",
            "\tint err, i, repstart, timeout;\n",
            "ti_i2c_transfer() assigns timeout only inside "
            "`for (i = 0; i < nmsgs; i++)' and reads it at the out: "
            "label to decide whether to wait for the bus",
        ),
    ],
    "hbsd/src/sys/dev/ti/if_ti.c": [
        (
            "PBSD: error = 0, which ti_copy_mem()",
            "\tint cnt, error;\n",
            "ti_copy_scratch()'s `while (cnt && error == 0)' read error "
            "on its very first test, and returned it when the loop did "
            "not run; ti_copy_mem(), the sibling it was copied from, "
            "sets error = 0 explicitly",
        ),
    ],
    "hbsd/src/sys/dev/ichwd/ichwd.c": [
        (
            "PBSD: smb = NULL.",
            "\tdevice_t ich, smb;\n",
            "ichwd_identify() assigns smb only inside "
            "`if (ich == NULL)' and a KASSERT reads it as "
            "`id_p->tco_version != 4 || smb != NULL'; safe today only "
            "because no ichwd_devices[] row has tco_version 4",
        ),
    ],
    "hbsd/src/sys/dev/mii/mv88e151x.c": [
        (
            "PBSD: cop_extcap = 0.",
            "\tuint32_t cop_cap, cop_extcap;\n",
            "the saved copper extended-capability word was assigned "
            "only under BMSR_EXTSTAT and restored unconditionally into "
            "sc->mii_extcapabilities at the end of the E1512 fiber "
            "block",
        ),
    ],
    # The same detach, twice: check the pointer for NULL and then
    # dereference it six times and free it.
    "hbsd/src/sys/dev/sound/pci/es137x.c": [
        (
            "PBSD: honour the NULL the next line already tests for.",
            "\tif (es != NULL && es->num != 0) {\n",
            "es_pci_detach() tested es != NULL and then dereferenced "
            "it unconditionally",
        ),
    ],
    "hbsd/src/sys/dev/sound/pci/via8233.c": [
        (
            "PBSD: honour the NULL the next line already tests for.",
            "\tif (via != NULL && (via->play_num != 0 || via->rec_num != 0)) {\n",
            "via_detach() tested via != NULL and then dereferenced it "
            "unconditionally",
        ),
    ],
    "hbsd/src/sys/dev/pci/pci_user.c": [
        (
            "PBSD: error, before the walk.",
            "\t\tcio->num_matches = 0;\n\n\t\t/*\n\t\t * If the user specified an offset",
            "PCIOCGETCONF reached its getconfexit: label with `error' "
            "unassigned whenever the device walk matched nothing - "
            "which is `pciconf -l' with a selector that matches no "
            "device",
        ),
    ],
    "hbsd/src/sys/dev/hid/u2f.c": [
        (
            "PBSD: error, initialised, like `length' one line up.",
            "\tsize_t length = 0;\n\tint error;\n",
            "u2f_read() assigns error only inside the wait loop or on "
            "a goto exit: path, so a zero-length read issued while a "
            "report is already buffered returned a stack word",
        ),
    ],
    "hbsd/src/sys/dev/usb/misc/cp2112.c": [
        (
            "PBSD: err, initialised. Both loops below are",
            "\tuint16_t read_off, to_read;\n\tint err;\n",
            "cp2112iic_transfer()'s validation pass and transfer pass "
            "are both `for (i = 0; i < nmsgs; i++)', so nmsgs == 0 ran "
            "neither and returned err unassigned",
        ),
    ],
    "hbsd/src/sys/dev/backlight/backlight.c": [
        (
            "PBSD: an unknown cmd fell out of this switch",
            "\t\t\tbcopy(&info, data, sizeof(struct backlight_info));\n\t\tbreak;\n\t}\n",
            "backlight_ioctl()'s switch had three cases and no "
            "default, and returned an uninitialised int. d_ioctl gets "
            "whatever number the caller passed, and the node is "
            "GID_VIDEO, not root-only",
        ),
    ],
    "hbsd/src/sys/dev/amdsmb/amdsmb.c": [
        (
            "PBSD: amdsmb_ec_read() returns 1 from each of its three",
            "\tamdsmb_ec_read(sc, SMB_PRTCL, &temp);\n\tif (temp != 0)\n",
            "all nine call sites discarded a return that means *data "
            "was never written, so the protocol register, the status "
            "byte, and the length and data bytes a block read hands "
            "back were stack slots whenever the embedded controller "
            "timed out",
        ),
    ],
    "hbsd/src/sys/arm/allwinner/aw_cir.c": [
        (
            "PBSD: nothing buffered, nothing to decode.",
            "\t\tdevice_printf(sc->dev, \"sc->dcnt = %d\\n\", sc->dcnt);\n\n\t/* Find Lead 1",
            "aw_ir_decode_packets() assigns `val' only inside three "
            "loops bounded by sc->dcnt, then branches on it twice. "
            "aw_ir_intr() calls it on every RX packet-end interrupt, "
            "including one whose FIFO counter is zero, where "
            "aw_ir_buf_reset() has already put sc->dcnt back to 0",
        ),
    ],
    "hbsd/src/sys/arm/allwinner/aw_usb3phy.c": [
        (
            "PBSD: error, not `int error'.",
            "\tint error, i;\n",
            "awusb3phy_attach() ends `return (error);' and assigns error "
            "only inside two for-loops whose conditions are the lookups "
            "themselves, so a node naming neither clocks nor resets "
            "returned a stack value to newbus as its attach status",
        ),
    ],
    "hbsd/src/sys/arm/allwinner/aw_gmacclk.c": [
        (
            "PBSD: clknode_create() COPIES what it is given",
            "\nfail:\n\treturn (error);\n}\n",
            "aw_gmacclk_attach() owned def.name and def.parent_names on "
            "every path out - clknode_create() strdups both - and freed "
            "neither, the success path included",
        ),
    ],
    "hbsd/src/sys/dev/clk/clk.c": [
        (
            "PBSD: `done' is the out-parameter every clknode driver's",
            "\tint rv, done;\n",
            "_clknode_set_freq() passed &done to CLKNODE_SET_FREQ() "
            "uninitialised. It is the *stop out-parameter every clknode "
            "driver's set_freq method receives, and a driver that sets "
            "it on only some paths - rk_clk_mux_set_freq() sets it "
            "inside `if (rv == 0)' and nowhere else - then has its "
            "`if (!*stop) return (0);' decided by this stack slot, with "
            "clknode_set_parent_by_idx(clk, best_parent) on the far "
            "side of the guard",
        ),
    ],
    "hbsd/src/sys/dev/clk/rockchip/rk_clk_composite.c": [
        (
            "PBSD: find_best() starts best_div at 0 and returns it",
            None,
            "rk_clk_composite_set_freq() divides by find_best()'s "
            "return inside its parent loop, and find_best() returns 0 "
            "when no divisor beat the initial best - which is every "
            "divisor when the parent is at 0 Hz, clknode_get_freq()'s "
            "return being unchecked. The `if (best_div == 0) return "
            "(ERANGE)' for exactly that case is ten lines below the "
            "divide",
        ),
    ],
    "hbsd/src/sys/dev/clk/rockchip/rk_clk_mux.c": [
        (
            "PBSD: best_parent is assigned only where *stop is",
            None,
            "the driver half of the clk.c fix: *stop = 0 before the "
            "loop that depends on it, matching the early return above "
            "which already sets it",
        ),
    ],
    "hbsd/src/sys/dev/clk/allwinner/aw_clk_nmm.c": [
        (
            "PBSD: best was declared and never assigned",
            "\tuint64_t cur, best;\n\tuint32_t n, m0, m1;",
            "aw_clk_nmm_find_best() compared against best on its first "
            "iteration and returned it if nothing beat it - a stack "
            "value handed back as the frequency this clock can make. "
            "aw_clk_nm.c, aw_clk_m.c and aw_clk_frac.c open with "
            "best = 0; aw_clk_nkmp.c and aw_clk_mipi.c assign it on the "
            "next line. Five of seven",
        ),
    ],
    "hbsd/src/sys/dev/clk/allwinner/aw_clk_np.c": [
        (
            "PBSD: best was declared and never assigned",
            "\tuint64_t cur, best;\n\tuint32_t n, p, max_n",
            "the other of the two find_best() functions that did not "
            "initialise best",
        ),
    ],
    "hbsd/src/sys/dev/clk/allwinner/aw_clk_nm.c": [
        (
            "PBSD: best_n and best_m are the divisors written into the",
            None,
            "aw_clk_nm_set_freq() assigns best_n and best_m only where "
            "a parent beat the running best, and writes them into the "
            "clock control register unconditionally. With every parent "
            "failing, best stays 0, survives the range checks whenever "
            "CLK_SET_ROUND_DOWN is set, and a stack value goes into a "
            "live clock divider. Three of the five locals were already "
            "initialised on the two lines above",
        ),
    ],
    "hbsd/src/sys/dev/clk/allwinner/aw_clk_frac.c": [
        (
            "PBSD: see aw_clk_nm_set_freq() - the same two divisors",
            None,
            "the same pair, written on aw_clk_frac_set_freq()'s "
            "integer-mode path",
        ),
    ],
    "hbsd/src/sys/dev/hwpmc/hwpmc_powerpc.c": [
        (
            ("if (pmc >= ppc_max_pmcs)", 2),
            "\tif (pmc > ppc_max_pmcs)\n",
            "six KASSERTs and two loops in this file bound the PMC "
            "index with `ri < ppc_max_pmcs'; powerpc_pmcn_read_default() "
            "and _write_default() were the two that used `>'. "
            "pmc == ppc_max_pmcs got through - on E500 (4 counters) "
            "that reads SPR_PMC5, which the CPU does not have",
        ),
        (
            ("panic(\"Invalid PMC number: %d\\n\", pmc);", 4),
            None,
            "and both switches fell through silently for an index the "
            "bound had already declared invalid: read_default returned "
            "an unassigned pmc_value_t as a counter reading, "
            "write_default wrote nothing and said nothing",
        ),
    ],
    "hbsd/src/sys/dev/hwpmc/hwpmc_e500.c": [
        (
            "PBSD: the switch on `vers' below covers four Freescale",
            "\tuint8_t pe_cpu_mask;\n",
            "the switch on mfpvr() >> 16 has no default, so a core this "
            "driver did not expect left pe_cpu_mask indeterminate - and "
            "the next statement is `if (pe_cpu_mask == 0) return "
            "(EINVAL);', admitting or refusing a performance event by a "
            "stack byte",
        ),
    ],
    "hbsd/src/sys/dev/cpufreq/cpufreq_dt.c": [
        (
            "PBSD: copp is the operating point to go back to",
            "\t\tif (CPUFREQ_DT_HAVE_REGULATOR(sc))\n"
            "\t\t\terror = regulator_set_voltage(sc->reg,\n",
            "cpufreq_dt_set() assigns copp only inside `if "
            "(regulator_get_voltage() != 0)' and reads it under the "
            "OUTER CPUFREQ_DT_HAVE_REGULATOR(sc), so a regulator that "
            "answers leaves it indeterminate - and both reads are in "
            "the clk_set_freq() failure handler, which programs the "
            "CPU supply voltage from copp->uvolt_min/max and the CPU "
            "clock from copp->freq. The least-tested path in the "
            "driver doing the most dangerous thing in it",
        ),
        (
            "PBSD: best_n indexed sc->opp[] without ever having been",
            None,
            "cpufreq_dt_find_opp() returns &sc->opp[best_n] with "
            "best_n unassigned when nopp == 0, and the caller hands "
            "the result to clk_set_freq() and regulator_set_voltage(). "
            "nopp counts device-tree nodes: a zero-length "
            "operating-points property, or an operating-points-v2 node "
            "with no children, and neither parser rejected either",
        ),
        (
            "PBSD: an operating-points-v2 node with no children",
            None,
            "the v2 parser's half of that: nopp <= 0 is refused, so "
            "the driver does not attach with an empty table",
        ),
    ],
    "hbsd/src/sys/dev/bnxt/bnxt_en/if_bnxt.c": [
        (
            "PBSD: rc is assigned only on a failure path",
            None,
            "bnxt_tx_queues_alloc() returns rc from a success exit that "
            "ntxqsets == 0 reaches having assigned it nowhere; iflib "
            "reads that as attach success or failure, so a zero-queue "
            "device attached according to a stack slot",
        ),
        (
            "PBSD: see bnxt_tx_queues_alloc() - the goto skips",
            None,
            "bnxt_msix_intr_assign() has the same shape one goto "
            "further: BNXT_CHIP_P5_PLUS jumps past the only assignment "
            "before the loop",
        ),
        (
            "media_type = BNXT_MEDIA_END;",
            None,
            "bnxt_add_media_types() leaves media_type unassigned in the "
            "PHY_TYPE_UNKNOWN and default arms - the two the card "
            "reaches by reporting a PHY type this driver does not know "
            "- and passes it to add_media() regardless, so the driver "
            "advertised whichever of ten BNXT_MEDIA_ values a stack "
            "byte named. Both arms say in a comment that only autoneg "
            "is supported, which is what BNXT_MEDIA_END gives",
        ),
    ],
    "hbsd/src/sys/compat/linuxkpi/common/include/linux/compiler.h": [
        (
            "#define\tuninitialized_var(x)\t\tx = 0",
            "#define\tuninitialized_var(x)\t\tx = x",
            "`int uninitialized_var(index);' expanded to `int index = "
            "index;', which reads an indeterminate object to initialise "
            "itself - undefined behaviour at the declaration, before any "
            "question of whether the variable is later read. Neither gcc "
            "nor clang warns on self-init, which is the macro's whole "
            "purpose and exactly why Linux deleted it in 2021. Eighteen "
            "uses in this tree, every one a scalar or a pointer",
        ),
    ],
    "hbsd/src/sys/compat/linuxkpi/common/include/linux/device.h": [
        (
            "PBSD: a NULL device logs, as it does on Linux",
            "#define\tdev_err(dev, fmt, ...)\t\tdevice_printf((dev)->bsddev",
            "Linux's _dev_printk() is `if (dev) ... else printk(...)', so "
            "dev_err(NULL, ...) is a supported call and drivers written "
            "for Linux make it. linuxkpi's eight dev_* macros wrote "
            "through the pointer unconditionally. bnxt_re defines "
            "rdev_to_dev(rdev) as ((rdev) ? &(rdev)->ibdev.dev : NULL) "
            "and feeds it to dev_err() at 52 sites - a macro that "
            "carefully yields NULL into one that dereferences it. "
            "sys/dev/bnxt: 63 findings -> 11",
        ),
        (
            ("__lkpi_dev_printf(dev,", 11),
            None,
            "all eleven uses of the macros that print through a device go "
            "through the NULL-tolerant helper, not just dev_err",
        ),
    ],
    "hbsd/src/libexec/bootpd/bootpgw/bootpgw.c": [
        (
            "PBSD: dst was declared here and never assigned",
            "\t\tstruct in_addr dst;\n",
            "the ARP-cache block in bootpgw's sendreply() lost the "
            "`dst = bp->bp_yiaddr;' that bootpd.c's identical block "
            "opens with, so inet_ntoa(dst) logged the stack and "
            "setarp() installed an SIOCSARP entry for whatever address "
            "it held - as root, on a program that answers packets from "
            "the network",
        ),
    ],
    "hbsd/src/lib/libc/db/hash/hash.c": [
        (
            ("return (destroy_hash(hashp));", 3),
            None,
            "init_hash() has four ways to fail and one of them cleaned "
            "up. __hash_open() writes the NULL return over its only "
            "reference to the table, so error1's _close(hashp->fp) is "
            "skipped and error0's free(hashp) is a free(NULL): both the "
            "HTAB and the descriptor opened three statements earlier "
            "are gone. Two of the three uncleaned exits are argument "
            "validation on a caller-supplied HASHINFO, so dbopen(3) in "
            "a loop with a bad bsize or lorder leaks a descriptor per "
            "call",
        ),
    ],
    "hbsd/src/lib/libc/db/hash/hash_page.c": [
        (
            "PBSD: freep is assigned only inside the search loop",
            None,
            "overflow_page() writes SETBIT(freep, free_bit) on a path "
            "where the loop that assigns freep never ran. It needs "
            "LAST_FREED past the last in-use bitmap page, and "
            "LAST_FREED is hdr.last_freed, _read() out of the database "
            "file with only MAGIC, VERSION and H_CHARKEY checked - so "
            "a crafted .db writes through an indeterminate pointer at "
            "an offset the file also chooses. Same shape as the "
            "run-time linker's DT_RELR bitmap",
        ),
    ],
    "hbsd/src/libexec/rtld-elf/aarch64/reloc.c": [
        (
            ("goto done;", 5),
            None,
            "reloc_non_plt() calloc'd a per-object symbol cache and freed "
            "it on no path at all - not the five error returns, not the "
            "success return. amd64, i386, arm and powerpc all free it at "
            "a `done' label; aarch64 and riscv were the two that did not. "
            "dynsymcount * 16 bytes per shared object, leaked again on "
            "every dlopen(3), for the life of the process",
        ),
    ],
    "hbsd/src/libexec/rtld-elf/riscv/reloc.c": [
        (
            ("goto done;", 6),
            None,
            "the same leaked symbol cache as aarch64/reloc.c, six error "
            "paths here. This is the one clang reported, because riscv "
            "was also the one per-architecture reloc.c that COMPILED "
            "under the sweep - against amd64's machine headers",
        ),
    ],
    "hbsd/src/sys/powerpc/ofw/ofw_real.c": [
        (
            "PBSD: instance is an OUT cell",
            "|| args.instance == 0) {",
            "ofw_real_open() tested a firmware OUT cell before "
            "ofw_real_unmap() copied the bounce page back over args, so "
            "a successful open was reported as a failure whenever the "
            "stack happened to hold zero there",
        ),
        (
            "nreturns + 2 > (int)nitems(args.slot)",
            None,
            "ofw_real_interpret() told the firmware to write nreturns "
            "cells into a 16-cell slot[] that fits fourteen, and "
            "OF_interpret()'s own slots[16] makes fifteen and sixteen "
            "look legal to the caller",
        ),
    ],
    "hbsd/src/sys/dev/usb/wlan/if_run.c": [
        (
            "None of the 70 call sites in this driver inspects the return",
            None,
            "run_rt3070_rf_read() and run_bbp_read() write *val only on "
            "the success path; 97 of their 98 call sites ignore the "
            "return and read the object straight back",
        ),
        (
            "The three `return (error)' paths and the EFSROM_KICK timeout",
            None,
            "run_efuse_read() write *val only on the success path, and "
            "neither run_iq_calib() nor sc->sc_srom_read()'s callers "
            "inspect the return",
        ),
    ],
    "hbsd/src/sys/dev/usb/wlan/if_mtw.c": [
        (
            "and none of mtw_srom_read()'s 19",
            None,
            "mtw_efuse_read_2(), mtw_bbp_read() and mtw_rf_read() write "
            "*val only on the success path; 33 of their 35 call sites "
            "ignore the return and read the object straight back",
        ),
        (
            "PBSD: derive ctl_ridx for both arms",
            "\t\t\tridx = rn->fix_ridx;\n\n\t\t} else {",
            "mtw_tx() is a copy of run_tx() with the ctl_ridx assignment "
            "moved inside the else arm, so a fixed unicast rate indexed "
            "a 44-entry table with an uninitialised uint8_t",
        ),
    ],
    "hbsd/src/sys/dev/usb/wlan/if_urtw.c": (
        "uint8_t data8 = 0;",
        "\tint ret;\n\tuint8_t data8;\n",
        "urtw_get_rfchip() reads data8 under URTW_RTL8187B but writes it "
        "only in that flag's arm - and the else arm can set the flag, so "
        "the two conditions are not complementary",
    ),
    "hbsd/src/sys/dev/firmware/arm/scmi_shmem.c": (
        "\tssize_t len;",
        "\tsize_t len;",
        "OF_getencprop_alloc_multi() returns ssize_t and -1 on failure; "
        "in a size_t that is SIZE_MAX, so `len <= 0` was a dead check "
        "and a missing shmem property dereferenced NULL at attach",
    ),
    "hbsd/src/sys/geom/raid3/g_raid3.c": (
        "if (md->md_all < 2)",
        "if (md->md_all < 1)",
        "md_all is a uint16_t off the medium and `sc_ndisks - 1` is a "
        "divisor in fifteen places, two of them inside "
        "g_raid3_check_metadata() itself; the bound was < 1",
    ),
    "hbsd/src/sys/geom/eli/g_eli_ctl.c": (
        "\t\texplicit_bzero(&md, sizeof(md));\n\t\tsnprintf(param,",
        None,
        "g_eli_ctl_configure() scrubbed md on one path of fifteen and "
        "read it uninitialised on the ONETIME path; md holds md_mkeys",
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

    # The DPAA ethernet - NXP's NetCommSw drop and the FreeBSD driver on
    # top of it. Fifty-five of its sixty-six translation units had never
    # been analysed, for want of one makeoptions; see includes.py.
    "hbsd/src/sys/contrib/ncsw/inc/xx_ext.h": (
        "XX_Exit(int status) __dead2",
        "void    XX_Exit(int status);",
        "XX_Exit panics, and ASSERT_COND - live here, dflags.h does not "
        "set DISABLE_ASSERTIONS - ends in it, so it is this driver's "
        "null check; undeclared noreturn, every caller past a failed "
        "assertion was a reachable path dereferencing what the assertion "
        "rejected",
    ),
    "hbsd/src/sys/contrib/ncsw/Peripherals/FM/fm_ncsw.c": [
        (
            "bool tmp = FALSE;",
            "            bool tmp;\n",
            "FM_IS_PORT_STALLED replies with a stack byte FmIsPortStalled "
            "does not write on its three error paths, to another "
            "partition",
        ),
        (
            "t_FmCtrlCodeRevisionInfo        fmanCtrlRevInfo = { 0 };",
            None,
            "FM_GET_FMAN_CTRL_CODE_REV, same shape: the reply body is "
            "memcpy'd whatever FM_GetFmanCtrlCodeRevision returned",
        ),
        (
            "t_FmDmaStatus       dmaStatus = { 0 };",
            None,
            "FM_DMA_STAT, same shape, and FM_GetDmaStatus returns void - "
            "there was not even an error to ignore",
        ),
        (
            "t_FmPhysAddr        physAddr = { 0 };",
            None,
            "FM_GET_PHYS_MURAM_BASE, same shape; FmGetPhysicalMuramBase "
            "returns early on two IPC failures",
        ),
    ],
    "hbsd/src/sys/contrib/ncsw/Peripherals/BM/bm.c": (
        "t_BmRevisionInfo    revInfo = { 0 };",
        None,
        "BM_GET_REVISION replies with a stack struct BmGetRevision does "
        "not write when it fails",
    ),
    "hbsd/src/sys/contrib/ncsw/Peripherals/QM/qm.c": [
        (
            "t_QmRevisionInfo    revInfo = { 0 };",
            None,
            "QM_GET_REVISION, the twin of BM_GET_REVISION",
        ),
        (
            'RETURN_ERROR(MAJOR, err, ("Can\'t read QMan revision"))',
            "        QmGetRevision(p_Qm, &revInfo);\n\n        if ((revInfo",
            "QmGetSetPortalParams discarded the return and then read "
            "revInfo to choose which of two layouts the portal's LIODN "
            "registers are programmed in",
        ),
    ],
    "hbsd/src/sys/contrib/ncsw/Peripherals/FM/Port/fm_port.h": (
        "uint8_t                     deqPipelineDepth;",
        None,
        "the depth the port reserved with, kept where it survives "
        "FmPortDriverParamFree",
    ),
    "hbsd/src/sys/contrib/ncsw/Peripherals/FM/Port/fm_port.c": [
        (
            "fmParams.deqPipelineDepth = p_FmPort->deqPipelineDepth;",
            "tx_fifo_deq_pipeline_depth;\n\n    FmFreePortParams",
            "FM_PORT_Free read p_FmPortDriverParam->dfltCfg one line "
            "after FmPortDriverParamFree() freed it and set it to NULL - "
            "and FM_PORT_Init had already nulled it, so every free of an "
            "initialised port dereferenced NULL",
        ),
        (
            "p_FmPort->deqPipelineDepth = fmParams.deqPipelineDepth;",
            None,
            "the other half: record it AFTER the override to 2 that "
            "FM_PORT_Init applies to the OH ports, because "
            "FmFreePortParams subtracts this from the FM's accumulated "
            "dequeue TNUM count",
        ),
    ],
    "hbsd/src/sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c": (
        "free(dofbuf, M_SOLARIS);",
        "\tfree(dof, M_SOLARIS);\n",
        "dtrace_dof_property()'s FreeBSD half has five `goto doferr' "
        "sites: two before the malloc, two after `dof = "
        "(dof_hdr_t *)dofbuf', and one in the hex-decoding loop between "
        "them, where dof is still NULL. free(dof) was free(NULL) there "
        "and the whole buffer leaked - on a bad character in a "
        "preloaded DOF blob, at a size the blob chooses",
    ),
    "hbsd/src/sys/contrib/dev/iwlwifi/mvm/sta.c": (
        # TWO: iwl_mvm_fw_baid_op_cmd() already ended with this exact
        # test, so a presence marker would be satisfied by the twin that
        # was already right - the very asymmetry being fixed.
        ("if (baid < 0 || baid >= ARRAY_SIZE(mvm->baid_map))", 2),
        "\t\treturn u32_get_bits(status, IWL_ADD_STA_BAID_MASK);\n",
        "iwl_mvm_fw_baid_op_sta() returned a seven-bit firmware field "
        "(IWL_ADD_STA_BAID_MASK is 0x7F00, so 0..127) as an index into "
        "mvm->baid_map, which has 32 entries, and its only caller checks "
        "just `baid < 0' before writing baid_map[baid]. Its twin behind "
        "the same dispatcher bounds it; this one did not",
    ),
    "hbsd/src/sys/contrib/dev/rtw89/core.c": (
        "dtim = bss_conf->dtim_period ?: 1;",
        "\tdtim = bss_conf->dtim_period;\n",
        "dtim_period comes out of the AP's beacon, is 0 until one has "
        "been parsed, and is the second divisor of `period / beacon_int "
        "/ dtim' two lines below. beacon_int on the line above already "
        "had the ?:, and the FreeBSD-local WARN saw the zero and only "
        "logged it",
    ),
    "hbsd/src/sys/contrib/dev/iwlwifi/mvm/rxmq.c": (
        # TWO, not one. iwl_mvm_rx_mpdu_mq() already had this exact
        # declaration, so a plain presence marker was satisfied by the
        # twin that never needed fixing and did not bite at all.
        ("struct iwl_mvm_rx_phy_data phy_data = {};\n\tu32 format;", 2),
        "\tstruct iwl_mvm_rx_phy_data phy_data;\n\tu32 format;",
        "iwl_mvm_rx_monitor_no_data() left d2/d3/d4/d5/eht_d4 "
        "uninitialised while setting phy_info to TSF_OVERLOAD, so "
        "info_type came out of the firmware's d1 and "
        "iwl_mvm_decode_he_phy_data() - which, unlike the EHT decoders, "
        "does not check ->with_data - read those bytes into the "
        "radiotap header. Its twin iwl_mvm_rx_mpdu_mq() has the = {}",
    ),
    "hbsd/src/sys/netinet/tcp_ratelimit.c": (
        "if (rs->rs_rate_cnt == 0) {",
        # Nothing was removed - the guard is an insertion - so there is
        # no upstream text to forbid.
        None,
        "rs_rate_cnt comes straight from the driver at :607 and :618 with "
        "no zero check, and at zero malloc(0) succeeds, both population "
        "loops have no iterations, and the `did we get at least 1 rate' "
        "test at :747 reads rs_rlt[-1] - out of bounds, before the "
        "allocation. The same file already guards it on the other path, "
        "at :349",
    ),
    "hbsd/src/sys/dev/xilinx/xlnx_pcib.c": (
        # THREE functions used sc->bst and sc->bsh, so a count of the
        # replacement; plus the kmem_alloc_contig cast, which is a
        # separate drift in the same file.
        ("bus_read_4(sc->res, ", 5),
        "\tt = sc->bst;",
        "struct generic_pcie_core_softc has no bst or bsh - the driver "
        "was not updated when they went - and kmem_alloc_contig() "
        "returns void * where msi_page is a vm_offset_t. Two API drifts "
        "in a file sys/conf/files.riscv:28 names and no kernel could "
        "compile",
    ),
    "hbsd/src/sys/dev/pci/pci_host_generic_acpi.c": (
        "off = res->Data.Address16.Address.TranslationOffset;",
        None,
        "the ADDRESS16 arm of the _CRS parser set restype, min and max "
        "and not off, while ADDRESS32, ADDRESS64 and FIXED_MEMORY32 all "
        "set it and :187 computes phys_base = min + off for all four - "
        "so a 16-bit address descriptor in firmware programmed a PCI "
        "range's physical base from an uninitialised stack slot",
    ),
    "hbsd/src/lib/libc/include/nscache.h": (
        "\tfree(mp_state);",
        "__close_cached_mp_read_session(mp_state->mp_read_session);\\\n}",
        "NSS_MP_CACHE_HANDLING's pthread key destructor closed the cached "
        "sessions and never freed the state itself, leaking one "
        "struct <db>_mp_state per non-main thread per NSS database that "
        "used nscd - five of them: passwd, group, services, protocols, "
        "rpc. Every one of the sixteen hand-written _endstate functions "
        "in lib/libc frees its state; this one, generated by a macro, "
        "was the seventeenth and did not",
    ),
    "hbsd/src/libexec/atrun/atrun.c": (
        # TWO declarations, so a count: perr and perrx both end in
        # exit(EXIT_FAILURE), and the `static void usage(void) __dead2'
        # one line below them shows the file already knows the idiom.
        # This is the same one-of-three shape as the mlx5 SRCS and the
        # arm64 atomics: the attribute exists on the third declaration
        # in the block and on neither of the other two.
        # `usage(void) __dead2' does not match this - the `...' is the
        # point - so the count is the two that were missing it.
        ("...) __dead2;", 2),
        "void perrx(const char *fmt, ...);\n",
        "perr() and perrx() never return - both end in exit() - but only "
        "usage() said so, so a caller's `if (pentry == NULL) perrx(...)' "
        "did not stop the analyser reading pentry->pw_name two lines "
        "later as a NULL dereference",
    ),
    "hbsd/src/libexec/atrun/gloadavg.c": (
        "void perr(const char *fmt, ...) __dead2;",
        "void perr(const char *fmt, ...);\n",
        "the same declaration in the program's other translation unit, "
        "where upstream's usr.bin/at/panic.h has carried __dead2 on its "
        "perr() all along",
    ),
    "hbsd/src/bin/ed/main.c": (
        # TWO sites, so a count: the `%' expansion and the `f' command
        # both call it, and fixing one is this document's most common
        # shape of half-fix.
        ("strip_escapes(old_filename)) == NULL", 2),
        None,
        "strip_escapes() returns NULL when its REALLOC fails - the macro "
        "returns the caller's `err' argument and this caller passes NULL "
        "- and the `%' expansion passed that straight to strlen()",
    ),
    "hbsd/src/bin/ed/io.c": (
        ("strip_escapes(fn)) == NULL", 2),
        None,
        "read_file() and write_file() passed a possibly-NULL "
        "strip_escapes() result to fopen(), which reports EFAULT and so "
        "printed `Bad address' for what is an out-of-memory condition",
    ),
    "hbsd/src/sys/modules/mlx5/Makefile": (
        # Two lines, one block. Counting a string common to both is not
        # possible, so this marks the CFLAGS - the one whose absence made
        # every source in the block fail rather than only the link.
        "CFLAGS+= -DCONFIG_MLX5_FPGA",
        None,
        "the CONFIG_BUILD_FPGA block listed six of the seven sources in "
        "sys/dev/mlx5/mlx5_fpga and set none of the CFLAGS its two "
        "sibling modules set in the identical block. The missing source "
        "defines the four mlx5_fpga_conn_* functions that two of the "
        "listed six call; the missing -DCONFIG_MLX5_FPGA is what "
        "mlx5_fpga/core.h puts its entire body behind",
    ),
    "hbsd/src/sys/arm64/include/atomic.h": (
        # Six lines in two hunks, and this counts a string common to
        # none of them, so it marks the one that matters most: the
        # release store cxgbe asks for. A merge that drops either hunk
        # loses this or the set/clear pair; the comment above each in the
        # header says why they are there, which is what a reader needs
        # when the diff looks like a no-op.
        "#define\tatomic_store_rel_8\tatomic_store_rel_8",
        None,
        "arm64 implements atomic_set_8, atomic_clear_8 and "
        "atomic_store_rel_8 as inline functions, so `#ifdef "
        "atomic_store_rel_8' was false there and sys/dev/cxgbe/t4_main.c "
        "took its fallback - a PLAIN store where the header has a release "
        "store - on a weak-memory architecture; the same for "
        "sys/vm/vm_page.c's byte path. The file already spells "
        "atomic_cmpset_8, atomic_fcmpset_8 and atomic_load_acq_8 as "
        "themselves for exactly this reason",
    ),
    "hbsd/src/sys/powerpc/include/atomic.h": (
        "#define\tatomic_subtract_acq_64\tatomic_subtract_acq_long",
        "atomic_subract_acq_long",
        "atomic_subtract_acq_64 expanded to a name with no `t\' in "
        "subtract, so any powerpc64 use of it was a call to an undeclared "
        "function; the atomic_subtract_acq_ptr two lines below has the "
        "spelling right",
    ),
    "hbsd/src/sys/dev/dpaa/if_dtsec_rm.c": (
        "t_DpaaFD fd = { 0 };",
        "\tt_DpaaFD fd;\n",
        "every DPAA_FD_SET_* that touches fd.length read-modify-writes "
        "it and each clears only its own mask, so the first read the "
        "descriptor before anything wrote it, and it is enqueued to the "
        "QMan",
    ),

    # The NFS client and server, from the sweep that read them as parsers
    # of whatever the other end sends.
    "hbsd/src/sys/fs/nfsclient/nfs_clrpcops.c": [
        (
            "} else if (nd->nd_repstat == 0 && gotattr != 0) {",
            "} else if (nd->nd_repstat == 0) {\n\t\t\t\tndp->nfsdl_change =",
            "openrpc: the KASSERT states `Getattr OK implies repstat 0\' and "
            "the code fourteen lines down used the converse, so a server "
            "that grants a delegation and fails the trailing Getattr had "
            "an uninitialised nfsvattr copied into it",
        ),
        (
            "if (NFSHASNFSV3(nmp) && NFSHASNFSV4(nmp) == 0) {",
            "if (NFSHASNFSV3(nmp)) {\n\t\tsbp->sf_tbytes",
            "statfs: tl is dissected only on the non-NFSv4 path, and the V2 "
            "arm below already carried the guard this one was missing",
        ),
        (
            'NFSCL_DEBUG(4, "aft parseg=%d\\n",\n\t\t\t\t\t    error);',
            'NFSCL_DEBUG(4, "aft parseg=%d\\n",\n\t\t\t\t\t    grp);',
            "parselayoutget: the debug line printed grp before the second "
            "nfsrv_parseug had filled it",
        ),
    ],
    "hbsd/src/sys/fs/nfsclient/nfs_clvnops.c": [
        (
            "\t\t    np = dnp;\n\t\t    newvp = dvp;",
            "\t\t    VREF(dvp);\n\t\t    newvp = dvp;\n\t\t} else {",
            "lookitup: the arm where the server answered with the "
            "directory\'s own filehandle set newvp and not np, and the tail "
            "published np to the caller",
        ),
        # TWO sites, so a count: nfs_mknodrpc() and nfs_create() both
        # reached NFSTOV(np) with np NULL. Losing one twin has to fail.
        (
            ("\t\telse\n\t\t\terror = ENOENT;\t/* No file handle, and none "
             "looked up. */", 2),
            None,
            "mknodrpc and create: NFSTOV(NULL) when neither the reply nor "
            "the fallback lookup produced a filehandle",
        ),
    ],
    "hbsd/src/sys/fs/nfsclient/nfs_clvfsops.c": [
        (
            "\tfree(nam, M_SONAME);\n\tfree(tlscertname, M_NEWNFSMNT);\n"
            "\tfree(hst, M_TEMP);",
            "newflag, tlscertname, aconn);\nout:",
            "nfs_mount: 37 goto out between the tlscertname malloc and "
            "mountnfs(), which is the only thing that owns it, and nine "
            "more after nam is set",
        ),
    ],
    "hbsd/src/sys/fs/nfsserver/nfs_nfsdsocket.c": [
        (
            "\tif (taglen < 0) {\n\t\tNFSM_BUILD(tl, u_int32_t *, "
            "2 * NFSX_UNSIGNED);",
            "\tif (taglen == -1) {\n\t\tNFSM_BUILD(tl, u_int32_t *,",
            "compound: the two spellings of `the tag did not parse\' now "
            "agree, so retopsp cannot be NULL where it is written",
        ),
    ],

    # Four more from the same sweep, in the network stack and one syscall.
    "hbsd/src/sys/kern/kern_resource.c": [
        (
            "\trtp->prio = 0;\n\tswitch (PRI_BASE(td->td_pri_class)) {",
            None,
            "pri_to_rtp: the switch writes prio on three of four arms and "
            "`default\' is PRI_ITHD, so rtprio_thread(2)\'s RTP_LOOKUP "
            "copyout()s two bytes of kernel stack for an interrupt thread",
        ),
    ],
    "hbsd/src/sys/netinet/in_mcast.c": [
        (
            "\tnims = NULL;\n\tschanged = 0;",
            "\t\tstruct ip_msource *bims;\n\n\t\tRB_FOREACH_REVERSE_FROM(",
            "inm_merge: inm_get_source() writes *pims only when it returns "
            "0, so a first-iteration ENOSPC left the rollback walking the "
            "tree from an uninitialised node",
        ),
    ],
    "hbsd/src/sys/netinet6/in6_mcast.c": [
        (
            "\tnims = NULL;\n\tschanged = 0;",
            "\t\tstruct ip6_msource *bims;\n\n\t\tRB_FOREACH_REVERSE_FROM(",
            "in6m_merge: the same function with in6 spellings",
        ),
    ],
    "hbsd/src/sys/netipsec/key.c": [
        (
            "\tstruct seclifetime lt = { 0 };",
            "\tstruct seclifetime lt;\n",
            "key_setdumpsp: two of four fields set and all four copied into "
            "the sadb_lifetime extension the SPD dump sends to every PF_KEY "
            "listener",
        ),
    ],

    # The lib/ half of the same sweep, and the one mount helper it reaches.
    "hbsd/src/lib/libbluetooth/hci.c": [
        # TWO error returns after wait_for_more, so a count: both were
        # freeing the cursor rather than the array.
        (
            ("\t\tfree(*ii);\n\t\t*ii = NULL;", 2),
            # NOT just `free(i); bt_devclose(s);' - the call BEFORE the
            # wait_for_more label has that exact shape and is correct,
            # because i still equals *ii there. The marker has to name
            # the site by what precedes it.
            "if (n < 0) {\n\t\tfree(i);",
            "bt_devinquiry: i walks forward one 256-byte bt_devinquiry per "
            "device reported and control returns to wait_for_more, so after "
            "one device both error arms free()d a pointer into the middle "
            "of the calloc block",
        ),
    ],
    "hbsd/src/lib/lib80211/lib80211_regdomain.c": [
        (
            # The marker is the free() itself, not the comment above it:
            # the first version matched the comment and passed with
            # free(dp) deleted. Caught by reverting, which is what the
            # reverting is for.
            "\t\tfree(dp);\n\t}",
            None,
            "lib80211_regdomain_cleanup: the first of three unlink-and-free "
            "loops freed the domain\'s bands and name and not the domain, "
            "while the two below it end in free(cp) and free(fp)",
        ),
    ],
    "hbsd/src/lib/libradius/radlib.c": [
        (
            "if (mlen < SALT_LEN + 16 || mlen % 16 != SALT_LEN) {",
            "\tif (mlen % 16 != SALT_LEN) {",
            "rad_demangle_mppe_key: mlen == SALT_LEN passes the modulus "
            "test, and then alloca(0) is read past by `*len = *P\'",
        ),
    ],
    "hbsd/src/lib/libc/resolv/res_debug.c": [
        (
            "static const unsigned int poweroften[10]",
            "static unsigned int poweroften[10]",
            "poweroften is a table of powers of ten that nothing writes; "
            "const puts it in .rodata and lets the analyser fold the "
            "divisor",
        ),
    ],
    "hbsd/src/lib/libdevstat/devstat.c": [
        (
            "} const devstat_arg_list[] = {",
            "} devstat_arg_list[] = {",
            "devstat_arg_list is the table devstat_compute_statistics() "
            "switches on to pick destu64 or destld and then switches on "
            "metric to write through - writable and externally linked, the "
            "analyser had to assume it changed between the two switches",
        ),
    ],
    "hbsd/src/lib/libc/gen/getpwent.c": [
        (
            "\tkeynum = st->keynum;\n\tif (how == nss_lt_all && st->keynum < 0) {",
            "\t\tkeynum = st->keynum;\n\t\tstayopen = 1;",
            "compat_passwd: three goto fin jump over the only assignment to "
            "keynum and fin: writes it back to the thread state, so the "
            "getpwent() after the last one overwrote the cursor with a "
            "stack value",
        ),
        (
            "int\t\t\t rv, from_compat, stayopen = 0, *errnop;",
            "int\t\t\t rv, from_compat, stayopen, *errnop;",
            "compat_passwd: stayopen has the identical bug one line down - "
            "the same three goto fin jump over it and fin: reads it to "
            "decide whether to close the database. files_passwd() at :805 "
            "already declares its own `stayopen = 0'",
        ),
    ],
    "hbsd/src/lib/libc/gen/syslog.c": [
        (
            "char hostname[MAXHOSTNAMELEN], tbuf[MAXLINE], *stdp = tbuf,",
            "char hostname[MAXHOSTNAMELEN], *stdp, tbuf[MAXLINE],",
            "vsyslog1: stdp is written and read under two reads of LogStat, "
            "a global openlog() writes",
        ),
    ],
    "hbsd/src/lib/libc/db/btree/bt_delete.c": [
        # TWO identical loops, so a count.
        (
            ("\t\tif (parent == NULL)\n\t\t\treturn (1);", 2),
            None,
            "__bt_stkacq: running off the top of the stack left idx unset "
            "and h already mpool_put(), and the restore loop subscripts "
            "that page",
        ),
    ],
    "hbsd/src/lib/libutil/mntopts.h": [
        (
            "const char *fmt, ...) __printflike(4, 5);",
            "const char *name, const char *fmt, ...);",
            "build_iovec_argf hands fmt to vsnprintf and never said so, "
            "which is why three call sites passing a runtime string never "
            "warned",
        ),
    ],
    "hbsd/src/lib/libutil/mntopts.c": [
        (
            "\t*iov = NULL;\n\t*iovlen = 0;\n}",
            "\tfree(*iov);\n}",
            "free_iovec: the reset its own comment documents, without which "
            "the documented `call nmount in a loop\' hands realloc() a "
            "freed pointer",
        ),
    ],
    "hbsd/src/sbin/mount_msdosfs/mount_msdosfs.c": [
        (
            'build_iovec_argf(&iov, &iovlen, "cs_local", "%s", quirk);',
            'build_iovec_argf(&iov, &iovlen, "cs_local", quirk);',
            "the -L argument, reached through kiconv_quirkcs() which returns "
            "its input unchanged when no quirk matches",
        ),
        (
            'build_iovec_argf(&iov, &iovlen, "cs_dos", "%s", cs_dos);',
            'build_iovec_argf(&iov, &iovlen, "cs_dos", cs_dos, (size_t)-1);',
            "the -D argument, strdup(optarg) with nothing between the getopt "
            "case and the call",
        ),
        (
            'build_iovec_argf(iov, iovlen, "cs_dos", "%s", cs_local);',
            'build_iovec_argf(iov, iovlen, "cs_dos", cs_local);',
            "and the kiconv helper\'s copy of the same string",
        ),
    ],
    "hbsd/src/sbin/route/route.c": [
        (
            "\tif (i != 2) {",
            None,
            "fiboptlist_range: one field leaves fib[1] unwritten and the "
            "test below reads it; the contract lived only in the caller",
        ),
    ],
    "hbsd/src/usr.sbin/bsnmpd/modules/snmp_hostres/hostres_fs_tbl.c": [
        (
            "\tmemset(entry, 0, sizeof(*entry));",
            None,
            "fs_entry_create: the line the other nine tables in this "
            "module have; without it `entry->flags |= HR_FS_FOUND\' reads "
            "indeterminate heap, and flags decides what survives a refresh",
        ),
    ],
    "hbsd/src/usr.sbin/bsnmpd/modules/snmp_hostres/hostres_processor_tbl.c": [
        (
            "\tif (cplen == 0)\n\t\treturn;",
            None,
            "refresh_processor_tbl: both failure arms of the kern.cp_times "
            "lookup set cplen = 0, and `long pcpu_cp_times[cplen]\' is then "
            "a zero-length VLA",
        ),
    ],
    "hbsd/src/usr.sbin/bsnmpd/modules/snmp_hast/hast_snmp.c": [
        (
            "\t\t\tnv_free(nvout);\t/* PBSD: not leaked on the way out */",
            None,
            "update_resources: the calloc-failure return dropped nvout",
        ),
        (
            "\t\t\tfree(res);\n\t\t\tcontinue;",
            "\t\tif (error != 0)\n\t\t\tcontinue;",
            "and the error%u arm dropped res, which is only linked in at "
            "the bottom of the loop -- once per resource in an error state, "
            "on every refresh, for the life of the daemon",
        ),
    ],
    "hbsd/src/usr.sbin/bsnmpd/modules/snmp_pf/pf_snmp.c": [
        (
            # `} else {' alone occurs twice in this file, so removing
            # the fix left the other one standing and the check passed.
            "INSERT_OBJECT_INT_LINK_INDEX(e, &pfq_table, link, index);\n"
            "\t\t} else {",
            None,
            "pfq_refresh: an altq with qid 0 is a parent discipline, was "
            "never inserted, and the next iteration overwrote e",
        ),
    ],
    "hbsd/src/usr.sbin/bsnmpd/modules/snmp_wlan/wlan_snmp.c": [
        (
            "\t*wif = NULL;\n\n\tif (wlan_mac_index_decode(oid, sub, wname, mac) < 0)",
            None,
            "wlan_get_acl_mac: the out-parameter was written only on the "
            "paths that reach wlan_find_interface(), and "
            "wlan_acl_mac_set_status() reads it after a NULL result",
        ),
        (
            "\t*wif = NULL;\t/* PBSD: as in wlan_get_acl_mac() above. */",
            None,
            "and wlan_get_next_acl_mac, which short-circuits the same way",
        ),
    ],
    "hbsd/src/usr.sbin/rpc.lockd/lockd.c": [
        (
            "\tint fd = -1;",
            "\tint fd;\n\tint nhostsbak;",
            "create_service: fd is assigned only inside "
            "`if (!kernel_lockd)\' and kernel_lockd is a global with "
            "syslog(), inet_pton() and getaddrinfo() between the two "
            "tests; -1 is this function\'s own `invalid\'",
        ),
    ],
    "hbsd/src/usr.sbin/rpc.tlsservd/rpc.tlsservd.c": [
        (
            "\tint gethostret = 0, ret;",
            "\tint gethostret, ret;",
            "rpctls_server: gethostret is written under "
            "`if (rpctls_verbose)\' and under `if (!rpctls_verbose)\', "
            "with three SSL_* calls between; 0 means no hostname and "
            "takes the RPCTLS_FLAGS_DISABLED arm -- fail closed",
        ),
    ],
    "hbsd/src/usr.bin/netstat/inet.c": [
        (
            "if (istcp && cflag) {",
            "\t\t\tif (cflag) {\n\t\t\t\txo_emit(\" {T:/%-*.*s}\"",
            "protopr: the Stack column header, which read fnamelen on a "
            "non-TCP pass where nothing had assigned it",
        ),
        (
            "if (istcp && Cflag)",
            None,
            "and the CC column header, the same way with cnamelen",
        ),
    ],
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


def _duplicate_keys() -> list[str]:
    """Names this file's own tables list twice.

    A dict literal with the same key twice is not an error in Python: the
    second entry silently replaces the first, and every marker in the one
    that lost goes with it. It happened here -- a second
    `"hbsd/src/sys/netinet/tcp_stacks/rack.c"` entry, added six hundred
    lines below the first, took six existing markers out of the file
    without changing a single check's result. Nothing in this checker
    could have noticed: the table it validates was already short.

    So the file reads itself. Same rule as everywhere else in this
    repository -- the invariant a gate depends on is checked, not
    assumed.
    """
    import ast

    dupes = []
    tree = ast.parse(Path(__file__).read_text())
    for node in ast.walk(tree):
        if not isinstance(node, ast.Dict):
            continue
        seen = set()
        for k in node.keys:
            if not isinstance(k, ast.Constant) or not isinstance(k.value, str):
                continue
            if k.value in seen:
                dupes.append(f"{k.value} (line {k.lineno})")
            seen.add(k.value)
    return dupes


def main() -> int:
    missing = []
    dupes = _duplicate_keys()
    if dupes:
        for d in dupes:
            print(f"FAIL  duplicate table entry: {d}")
        print("\nA repeated dict key silently replaces the first one, and "
              "every marker\nin it. Merge the two entries.")
        return 1
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
            # `want' may be (text, n): the marker has to appear at least n
            # times. Plain substring presence cannot see a lost sibling --
            # one marker covering a pair of twins is satisfied by either,
            # so a merge that ate one of the two passed silently. That is
            # the guard-on-one-of-a-pair defect, in the file whose whole
            # job is catching it; it was found by reverting one twin and
            # watching this not fail. Six identical NFSVNO_ATTRINIT sites
            # are the same problem at greater width.
            #
            # A count marker also has to be counted in the file AFTER the
            # fix, not in the diff: tegra210_xusbpadctl.c's comment quoted
            # the corrected line, so the file held seven copies of a
            # marker registered as six, and deleting one of the six still
            # passed. Found the same way, by reverting one and watching
            # this not fail. Do not quote the marker text in the comment
            # the fix adds -- describe it instead.
            need = 1
            if isinstance(want, tuple):
                want, need = want
            seen = text.count(want)
            if seen < need:
                missing.append((rel, what, f"fix is gone: {want!r} found "
                                           f"{seen} time(s), needs {need}"))
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
