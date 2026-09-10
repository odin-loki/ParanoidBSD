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
    "hbsd/src/usr.bin/gencat/gencat.c": [
        (
            "while ((msg = set->msghead.lh_first) != NULL) {",
            "while (msg) {\n\t\t\tfree(msg->str);",
            "MCDelSet()'s loop never advanced msg -- LIST_REMOVE unlinks "
            "it and does not change it -- so $delset for an existing set "
            "hung gencat and freed msg->str again on the second pass",
        ),
        (
            "static\tvoid\terror(const char *) __dead2;",
            None,
            "error() ends in exit(1); xmalloc(), xrealloc() and xstrdup() "
            "all return the pointer they only reach when it is not NULL",
        ),
        (
            "void\tusage(void) __dead2;",
            None,
            "usage() ends in exit(1)",
        ),
    ],
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
    "hbsd/src/sys/fs/nfsserver/nfs_nfsdport.c": [
        (
            "PBSD: iv and cnt are out-parameters, so read them only",
            "\t\terror = nfsrv_createiovecw(len, m, cp, &iv, &cnt);\n"
            "\t\tuiop->uio_iov = iv;",
            "nfsvno_setxattr() stored nfsrv_createiovecw()'s "
            "out-parameters into the uio without checking the return, "
            "while nfsvno_write() - the only other caller - returns on "
            "error before touching them",
        ),
        (
            "PBSD: zero the object, because all of it goes to disk.",
            "vp\");\n\tdsattr.dsa_filerev = nap->na_filerev;",
            "nfsrv_setextattr: the five assignments cover every named "
            "member but vn_extattr_set() writes sizeof(dsattr), so any "
            "padding the ABI puts in the struct or in its two struct "
            "timespec is stack that lands in the pnfsd.dsattr extended "
            "attribute and comes back to any pNFS client",
        ),
        (
            "PBSD: see nfsrv_setextattr() -- all of this goes to disk "
            "too.",
            "\tstruct pnfsdsattr dsattr;\n\tstruct vattr va;",
            "nfsrv_pnfscreate: the same struct, written to the same "
            "extended attribute with the same sizeof",
        ),
        (
            "PBSD: savbits too.  This arm was initialising for the",
            "\t} else {\n\t\tNFSZERO_ATTRBIT(&attrbits);\n\t}\n"
            "\tfullsiz = siz;",
            "nfsrvd_readdirplus: savbits is filled only under "
            "ND_NFSV4 and the non-V4 arm zeroed attrbits alone, while "
            "the entry loop's `(nd->nd_flag & ND_NFSV3) || "
            "NFSNONZERO_ATTRBIT(&savbits)' reads it whenever the "
            "request is not V3",
        ),
    ],
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
        (
            "PBSD: pcm_init() stores scp as the sound layer's devinfo",
            "\tif (bus_alloc_resources(dev, ssi_spec, sc->res)) {\n"
            '\t\tdevice_printf(dev, "could not allocate resources\\n");\n'
            "\t\treturn (ENXIO);\n\t}\n",
            "ssi_attach() dropped sc, sc->conf, scp, the mutex, the bus "
            "resources and the whole DMA tag/memory/map stack on seven "
            "returns; the analyser named only scp",
        ),
    ],
    "hbsd/src/sys/arm/freescale/vybrid/vf_sai.c": [
        (
            "PBSD: err, checked. This assignment was dead",
            "\t    &sc->dma_tag);\n\n\terr = bus_dmamem_alloc(",
            "the same dead assignment as imx6_ssi.c, in the driver it "
            "was copied from or to",
        ),
        (
            "PBSD: pcm_init() stores scp as the sound layer's devinfo",
            "\tif (bus_alloc_resources(dev, sai_spec, sc->res)) {\n"
            '\t\tdevice_printf(dev, "could not allocate resources\\n");\n'
            "\t\treturn (ENXIO);\n\t}\n",
            "sai_attach() is imx6_ssi.c one SoC over and leaked the same "
            "way, minus sc->conf, which this driver does not have",
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
    "hbsd/src/sys/dev/ntb/ntb_hw/ntb_hw_intel.c": [
        (
            "PBSD: check these. intel_ntb_spad_read() returns EINVAL",
            "\tintel_ntb_spad_read(ntb->device, NTB_MSIX_GUARD, &val);\n",
            "intel_ntb_spad_read() returns EINVAL without writing *val "
            "when the scratchpad index is past ntb->spad_count, and "
            "intel_ntb_exchange_msix() discarded that on four calls - "
            "two of which write the peer's MSI-X address and data",
        ),
    ],
    "hbsd/src/sys/dev/iwn/if_iwn.c": [
        (
            ("PBSD: the three reads below are checked", 1),
            "\tvoid\t\t(*read_eeprom)(struct iwn_softc *);\n",
            "iwn_read_prom_data() returns ETIMEDOUT or EIO without "
            "writing the caller's buffer, and seventeen of its eighteen "
            "call sites discarded that; read_eeprom is now int and "
            "every one is checked",
        ),
        (
            # The read_eeprom method itself, in the other file. If a
            # resync brings back `void (*read_eeprom)', the .c will not
            # compile - but the marks check should say so first.
            "PBSD: every iwn_read_prom_data() here is checked",
            None,
            "iwn4965_read_eeprom() checks all four of its reads and the "
            "channel loop",
        ),
        (
            "PBSD: checked, and the method returns int.",
            None,
            "iwn_read_eeprom_enhinfo() fills 35 structures on the stack "
            "and walks every one of them to set per-channel transmit "
            "power",
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
    "hbsd/src/sys/compat/linux/linux_ioctl.c": [
        (
            "PBSD: store, then check.",
            "\tcase DVD_STRUCT_PHYSICAL:\n\t\tif (bp->layer_num >= 4)\n"
            "\t\t\treturn (EINVAL);\n"
            "\t\tbp->layer_num = lp->physical.layer_num;\n",
            "linux_to_bsd_dvd_struct() tested bp->layer_num before "
            "anything wrote it and then took the userland value without "
            "checking it at all",
        ),
    ],
    "hbsd/src/sys/compat/linux/linux_misc.c": [
        (
            "PBSD: error, initialised.  With args->pid == 0",
            "\tint flags;\n\tint error;\n\tbool exec_blocked;\n",
            "linux_prlimit64(0, resource, NULL, NULL) reached "
            "`return (error)' with error never assigned, handing the "
            "syscall layer a stack word as the errno",
        ),
    ],
    "hbsd/src/sys/compat/linux/linux_signal.c": [
        (
            "PBSD: only when it succeeded.",
            "\terror = linux_do_sigaction(td, args->sig, &nsa, &osa);\n"
            "\ttd->td_retval[0] = (int)(intptr_t)osa.lsa_handler;\n",
            "linux_signal() read osa.lsa_handler whether or not "
            "linux_do_sigaction() had filled it",
        ),
    ],
    "hbsd/src/sys/dev/hyperv/pcib/vmbus_pcib.c": [
        (
            "PBSD: a default, because `size' is the LENGTH",
            "\t\tsize = sizeof(ctxt.int_pkts.v3);\n\t\tbreak;\n\t}\n",
            "the protocol-version switch had no default, and `size' is "
            "the byte count handed to vmbus_chan_send()",
        ),
    ],
    "hbsd/src/sys/dev/hyperv/netvsc/hn_rndis.c": [
        (
            "PBSD: and check how much came back.",
            "\t    &in, NDIS_RSS_CAPS_SIZE, &caps, &caps_len, "
            "NDIS_RSS_CAPS_SIZE_6_0);\n\tif (error)\n\t\treturn (error);"
            "\n\n\t/*\n\t * Preliminary verification.\n\t */\n",
            "hn_rndis_query2() returns 0 having copied nothing when the "
            "completion reports no info buffer, and the two callers that "
            "use it directly read the struct anyway",
        ),
    ],
    "hbsd/src/sys/dev/hyperv/netvsc/if_hn.c": [
        (
            "PBSD: hash_value too",
            "\tinfo.hash_info = NULL;\n\tinfo.pktinfo_id = NULL;\n",
            "four of the five struct hn_rxinfo members were cleared "
            "before the per-packet-info walk; hn_rsc_add_data() copies "
            "the fifth unconditionally",
        ),
    ],
    "hbsd/src/sys/dev/igc/if_igc.c": [
        (
            "PBSD: packets too.",
            "\t\tbytes = bytes_per_packet = 0;\n",
            "igc_neweitr() zeroed two of its three accumulators; "
            "`packets' was assigned only inside the two `!= 0' arms and "
            "read in the lmax() and three times in the latency state "
            "machine",
        ),
    ],
    "hbsd/src/sys/dev/igc/igc_phy.c": [
        (
            "PBSD: say what happened.",
            '\tDEBUGFUNC("igc_phy_has_link_generic");\n\n'
            "\tif (!hw->phy.ops.read_reg)\n\t\treturn IGC_SUCCESS;\n",
            "igc_phy_has_link_generic() returned IGC_SUCCESS without "
            "writing *success when the PHY has no read_reg method, and "
            "all four callers branch on a stack `bool link' right after",
        ),
    ],
    "hbsd/src/sys/dev/ice/if_ice_iflib.c": [
        (
            "PBSD: rid = 1, the administrative vector's",
            "\tint rid;\n\tfor (i = 0, vector = 1; "
            "i < vsi->num_rx_queues; i++, vector++) {\n",
            "rid was assigned only inside the queue loop and read after "
            "it as `sc->last_rid = rid + sc->irdma_vectors'",
        ),
    ],
    "hbsd/src/sys/dev/ice/ice_nvm.c": [
        (
            "PBSD: checked.  ice_read_sr_word() leaves checksum_sr",
            "\tice_read_sr_word(hw, ICE_SR_SW_CHECKSUM_WORD, "
            "&checksum_sr);\n",
            "ice_nvm_validate_checksum() compared its computed checksum "
            "against a stack word when the shadow RAM read failed",
        ),
    ],
    "hbsd/src/sys/dev/ice/ice_common.c": [
        (
            "PBSD: clear the two fields this function ACCUMULATES into.",
            "\tldo->fec_options = buf & ICE_LINK_OVERRIDE_FEC_OPT_M;\n\n"
            "\t/* PHY types low */\n",
            "ice_get_link_default_override() builds phy_type_low and "
            "phy_type_high with |= and one caller's tlv is not zeroed",
        ),
    ],
    "hbsd/src/sys/dev/ixl/if_ixl.c": [
        (
            "PBSD: the out-parameter, before anything can return without it.",
            "\tu16 opcode;\n\tu32 loop = 0, reg;\n\n"
            "\tevent.buf_len = IXL_AQ_BUF_SZ;\n",
            "ixl_process_adminq() returned ENOMEM, and broke out of its "
            "loop, without writing *pending; the caller discards the "
            "return and schedules itself again on `pending > 0'",
        ),
    ],
    "hbsd/src/sys/dev/cxgbe/tom/t4_tls.c": [
        (
            "PBSD: m != NULL first.",
            "\tif (sb->sb_flags & SB_AUTOSIZE &&\n\t    V_tcp_do_autorcvbuf "
            "&&\n\t    sb->sb_hiwat < V_tcp_autorcvbuf_max &&\n"
            "\t    m->m_pkthdr.len > (sbspace(sb) / 8 * 7)) {\n",
            "the control-mbuf arm does `m_freem(m); m = tls_data;' and "
            "tls_data is NULL for a record with no payload, so a "
            "zero-length TLS application-data record reached this "
            "dereference",
        ),
    ],
    "hbsd/src/sys/dev/cxgbe/cudbg/fastlz_api.c": [
        (
            "PBSD: an error, not success.",
            "\tif (byte_r == 0)\n\t\treturn 0;\n",
            "read_chunk_header() returned 0 having written none of its "
            "five out-parameters, and decompress_buffer() reads all of "
            "them on a 0 - chunk_size sizes a scratch allocation and "
            "bounds an adler32 over the buffer",
        ),
    ],
    "hbsd/src/sys/dev/cxgbe/cudbg/cudbg_lib.c": [
        (
            "PBSD: this function owns its out-parameter.",
            "\tif (nelem != (CTXT_CNM + 1))\n\t\treturn -EINVAL;\n\n"
            "\tfor (i = 0; i < meminfo->mem_c; i++) {\n",
            "get_max_ctxt_qid() writes an entry only for a region it "
            "finds; the caller's array has no initialiser and the "
            "entries are used as counts",
        ),
    ],
    "hbsd/src/sys/dev/cxgbe/cxgbei/cxgbei.c": [
        (
            "PBSD: ip, for the combination neither block above covered.",
            "\t\ticp = ip_to_icp(ip);\n\t}\n"
            "\tpdu_len = G_ISCSI_PDU_LEN(be16toh(cpl->pdu_len_ddp));\n",
            "do_rx_iscsi_cmp() sets ip in the non-DDP block and in the "
            "allocation block; a DDP-placed PDU arriving while "
            "toep->ulpcb2 already holds one reached m_copydata() with it "
            "unassigned",
        ),
    ],
    "hbsd/src/sys/dev/cxgbe/t4_sge.c": [
        (
            "PBSD: 0, as alloc_ctrlq(), alloc_rxq(), alloc_txq() and",
            "\t\tMPASS(ofld_rxq->iq.flags & IQ_HW_ALLOCATED);\n\t}\n"
            "\treturn (rc);\n}\n",
            "alloc_ofld_rxq() was the only one of the five idempotent "
            "allocators ending `return (rc)', and rc is assigned only "
            "inside the two not-yet-allocated blocks - so the idempotent "
            "case it exists for returned a stack word as an errno",
        ),
    ],
    "hbsd/src/sys/dev/cxgbe/common/t4_hw.c": [
        (
            "PBSD: checked.  t4_seeprom_read() returns without writing",
            "\t\tt4_seeprom_read(adapter, EEPROM_STAT_ADDR, &stats_reg);\n"
            "\t} while ((stats_reg & 0x1) && --max_poll);\n",
            "t4_seeprom_write()'s completion poll spun on a stack word "
            "when the status read failed, and could report the VPD write "
            "finished without having read anything that says so",
        ),
    ],
    "hbsd/src/sys/dev/cxgbe/t4_sched.c": [
        (
            "PBSD: and the mode.",
            "\t\t} else if (p->rateunit == SCHED_CLASS_RATEUNIT_PKTS) {\n"
            "\t\t\t/* maxrate is the absolute value in pps. */\n"
            "\t\t\tcheck_pktsize = true;\n"
            "\t\t\tfw_rateunit = FW_SCHED_PARAMS_UNIT_PKTRATE;\n"
            "\t\t} else\n",
            "the packet-rate branch set the unit and not the mode, so "
            "fw_ratemode reached tc->ratemode and t4_sched_params() "
            "unwritten",
        ),
    ],
    "hbsd/src/sys/dev/mlx5/mlx5_core/mlx5_vsc.c": [
        (
            "PBSD: = 0, as mlx5_vsc_write() and mlx5_vsc_set_space()",
            "\tint err;\n\tu32 in;\n",
            "MLX5_VSC_SET() is a read-modify-write, so mlx5_vsc_read() "
            "built the VSC address word - flag bit included - out of "
            "the stack and wrote it to PCI config space",
        ),
    ],
    "hbsd/src/sys/dev/mlx5/mlx5_core/mlx5_port.c": [
        (
            "PBSD: checked.  mlx5_query_port_admin_status() leaves ps",
            "\tmlx5_query_port_admin_status(dev, &ps);\n"
            "\tmlx5_set_port_status(dev, MLX5_PORT_DOWN);\n",
            "mlx5_toggle_port_link() decided whether to bring the port "
            "back up on a stack word when the admin-status query failed",
        ),
    ],
    "hbsd/src/sys/dev/mlx5/mlx5_core/mlx5_fwdump.c": [
        (
            "PBSD: fw_data, not fake_fw.data.",
            "\t\tif (fake_fw.data == NULL) {\n",
            "the MLX5_FW_UPDATE ioctl checked a member of a local struct "
            "firmware that is bzero'd four lines later, instead of the "
            "allocation on the line above",
        ),
    ],
    "hbsd/src/sys/dev/mlx5/mlx5_core/mlx5_fs_core.c": [
        (
            "PBSD: dest != NULL.  This function tests dest for NULL",
            "\t\tif (dest->type == MLX5_FLOW_DESTINATION_TYPE_FLOW_TABLE "
            "&&\n\t\t    ft->type != dest->ft->type)\n",
            "dest_is_valid() tests dest for NULL above and below this "
            "line and dereferenced it here",
        ),
    ],
    "hbsd/src/sys/dev/mlx5/mlx5_en/mlx5_en_main.c": [
        (
            "PBSD: checked.  mlx5e_get_wqe_sz() returns -ENOMEM",
            "\tmlx5e_get_wqe_sz(priv, &wqe_sz, &nsegs);\n"
            "\tMLX5_SET(wq, wq, wq_type, MLX5_WQ_TYPE_LINKED_LIST);\n",
            "mlx5e_build_rq_param() is void and discarded the return, so "
            "a failure built the receive queue's stride and size from "
            "two unwritten stack words",
        ),
    ],
    "hbsd/src/sys/dev/mana/hw_channel.c": [
        (
            "PBSD: the NULL arm printed the pointer it had just found NULL",
            "\tif (!hwc_txq || hwc_txq->gdma_wq->id != gdma_txq_id) {\n",
            "the tx event handler's `!hwc_txq' short-circuited into a "
            "body that read hwc_txq->gdma_wq->id, and then fell through "
            "to bus_dmamap_sync() on it either way",
        ),
    ],
    "hbsd/src/sys/dev/bwn/if_bwn.c": [
        (
            "PBSD: memcpy, not `*((uint32_t *)noise) = ...'.",
            "\t*((uint32_t *)noise) = htole32(bwn_jssi_read(mac));\n",
            "bwn_intr_noise() wrote a uint32_t through a pointer to a "
            "uint8_t[4] object and read the bytes back",
        ),
    ],
    "hbsd/src/sys/dev/aic7xxx/aic7xxx_pci.c": [
        (
            "PBSD: the six bit masks, which this path never set.",
            "\t\tsd.sd_dataout_offset = SEECTL;\t\t\n\n"
            "\t\tahc_acquire_seeprom(ahc, &sd);\n",
            "ahc_pci_resume() built a seeprom_descriptor with only the "
            "four offsets set and handed it to code that writes sd_MS "
            "to the SEECTL register and masks with sd_RDY and sd_CS",
        ),
    ],
    "hbsd/src/sys/dev/axgbe/xgbe-phy-v1.c": [
        (
            "PBSD: `amd,speed-set' comes out of the device tree with no",
            "\t\t\tmode = XGBE_MODE_KX_2500;\n\t\t\tbreak;\n"
            "\t\t}\n\t} else {\n",
            "xgbe_an73_outcome()'s switch on an unvalidated device tree "
            "property had no default, and returned an unwritten `mode' "
            "to the caller that programs the PHY",
        ),
    ],
    "hbsd/src/sys/dev/axgbe/if_axgbe.c": [
        (
            "PBSD: and check its range.",
            "\t\t    XGBE_SPEEDSET_PROPERTY);\n\t\treturn (EINVAL);\n"
            "\t}\n\n\terror = axgbe_get_optional_prop(",
            "the `amd,speed-set' property was taken verbatim from the "
            "device tree and switched on in three places",
        ),
    ],
    "hbsd/src/sys/dev/axgbe/xgbe-phy-v2.c": [
        (
            "PBSD: memcpy, not a store through `(__be16 *)&redrv_data[2]'.",
            "\tredrv_val = (__be16 *)&redrv_data[2];\n"
            "\t*redrv_val = cpu_to_be16(val);\n",
            "two of the five redriver bytes and the checksum built from "
            "them were written through a __be16 lvalue into a uint8_t "
            "array",
        ),
    ],
    "hbsd/src/sys/dev/axgbe/xgbe-txrx.c": [
        (
            "PBSD: buf2_len = 0 on the no-split-header path.",
            "\t\t\tlen += buf1_len;\n\t\t\tif (pdata->sph_enable) {\n",
            "buf2_len was assigned only inside the sph_enable arm and "
            "printed unconditionally whenever packet->errors was set",
        ),
    ],
    "hbsd/src/sys/dev/usb/input/wmt.c": [
        (
            "PBSD: err, initialised to a FAILURE.",
            "\tsize_t i;\n\tint err;\n",
            "wmt_attach() assigns err only inside two guarded feature "
            "report fetches and reads it as `err == 0' afterwards; a "
            "device with no Contact Count Maximum report and a shared "
            "Button Type report id skips both",
        ),
    ],
    "hbsd/src/sys/contrib/vchiq/interface/vchiq_arm/vchiq_arm.c": [
        (
            "PBSD: service1, not service.",
            "\t\t\t\tUSER_SERVICE_T *user_service =\n"
            "\t\t\t\t\tservice->base.userdata;\n",
            "the closed-service block read base.userdata off `service', "
            "the variable of the loop above it, which is NULL because "
            "that loop ended when next_service_by_instance() returned "
            "NULL",
        ),
    ],
    "hbsd/src/sys/dev/sbni/if_sbni_isa.c": [
        (
            "PBSD: a union, not `*(u_int32_t *)&flags",
            "\t*(u_int32_t*)&flags = device_get_flags(dev);\n",
            "the ISA attach wrote a struct sbni_flags object through a "
            "u_int32_t lvalue - a strict aliasing violation the compiler "
            "may discard - and then passed the struct by value",
        ),
    ],
    "hbsd/src/sys/dev/adlink/adlink.c": [
        (
            "PBSD: reject an empty ring.",
            "\t\t\tsc->nchunks = sc->p0->ringsize / sc->p0->chunksize;\n"
            "\t\t\tif (sc->nchunks * sizeof (*pg->sample) +\n",
            "ADLINK_START applies its chunk-size default after the "
            "ioctl that would have rejected the combination, so a ring "
            "smaller than the default chunk gives nchunks == 0, "
            "malloc(0, M_ZERO) and a NULL sample pointer - from a "
            "0444 device",
        ),
    ],
    "hbsd/src/sys/dev/acpi_support/acpi_asus_wmi.c": [
        (
            "PBSD: both reads below are checked.",
            "\t\t\tacpi_wpi_asus_get_devstate(sc,\n"
            "\t\t\t    ASUS_WMI_DEVID_TOUCHPAD, &val);\n",
            "acpi_asus_wmi_evaluate_method() returns -EINVAL without "
            "writing *retval, and the hotkey handler wrote the "
            "resulting garbage back to the firmware as a backlight "
            "level or a touchpad state",
        ),
    ],
    "hbsd/src/sys/dev/acpi_support/acpi_asus.c": [
        (
            "PBSD: check the evaluation.",
            '\tAcpiEvaluateObject(sc->handle, "INIT", &Args, &Buf);\n'
            "\tObj = Buf.Pointer;\n",
            "a failed AcpiEvaluateObject() leaves Buf.Pointer NULL and "
            "the next statement read Obj->String.Pointer off it",
        ),
        (
            "PBSD: and stop here.",
            None,
            "the Samsung/EeePC block fell through to a strncmp() whose "
            "first argument is the NULL pointer the block exists "
            "because of",
        ),
    ],
    "hbsd/src/sys/dev/iommu/busdma_iommu.c": [
        (
            "PBSD: this function is reachable only as bus_dma_iommu_impl's",
            "\terror = common_bus_dma_tag_create(parent != NULL ?\n"
            "\t    &((struct bus_dma_tag_iommu *)parent)->common : NULL,",
            "iommu_bus_dma_tag_create() tested parent for NULL in its "
            "first statement and dereferenced it unconditionally three "
            "lines later; bus_dma_tag_create() reaches a tag's impl "
            "only when parent is not NULL, so the test was the wrong "
            "half",
        ),
    ],
    "hbsd/src/sys/dev/sound/macio/i2s.c": [
        (
            "PBSD: past this point the softc is no longer ours alone.",
            '\tport = of_find_firstchild_byname(sc->node, "i2s-a");\n'
            "\tif (port == -1)\n\t\treturn (ENXIO);\n",
            "i2s_attach() dropped the softc, the port mutex and up to "
            "three bus resources on seven returns, every one of them "
            "before anything else had seen the softc",
        ),
    ],
    "hbsd/src/sys/dev/sound/macio/davbus.c": [
        (
            "PBSD: from here the softc is the sound layer's as well",
            "\tsc->reg = bus_alloc_resource_any(self, SYS_RES_MEMORY, "
            "&rid, RF_ACTIVE);\n\tif (sc->reg == NULL) \n"
            "\t\treturn (ENXIO);\n",
            "davbus_attach() dropped the softc and the resources mapped "
            "so far on four returns; the mutex is initialised later "
            "here than in i2s.c, so the unwind has one stage fewer",
        ),
    ],
    "hbsd/src/sys/dev/sdhci/sdhci_fdt.c": [
        (
            "PBSD: clknode_create() copies both the name and the parent",
            '\t\t\tdevice_printf(sc->dev, "cannot create clknode\\n");\n'
            "\t\t\treturn;\n",
            "sdhci_export_clocks() allocated a one-entry parent name "
            "array inside the loop and freed none of them, and let the "
            "clock-output-names array go on all three returns",
        ),
    ],
    "hbsd/src/sys/arm64/rockchip/rk_usb2phy.c": [
        (
            "PBSD: a malformed property can still hand back an array;",
            "\tif (nclocks != 1)\n\t\treturn (ENXIO);\n",
            "rk_usb2phy_export_clock() leaked the clock-output-names "
            "array on five paths and def.parent_names on four, "
            "including the success path",
        ),
    ],
    "hbsd/src/sys/riscv/sifive/sifive_prci.c": [
        (
            "PBSD: this arm reached 'fail', which is past the free of",
            '\t\tdevice_printf(dev, "Couldn\'t create clock domain\\n");\n'
            "\t\tgoto fail;\n",
            "prci_attach() jumped past its own fail1: free on two "
            "arms and never freed clkdef.parent_names on the success "
            "path; the clkdom arm also returned the 0 left in error",
        ),
    ],
    "hbsd/src/sys/dev/cyapa/cyapa.c": [
        (
            "PBSD: error = 0. It is assigned only inside",
            # NB: narrowed. `struct cyapa_softc *sc;' followed by
            # `int error;' also opens cyaparead() at :696 and
            # cyapaioctl() at :1215, so the first version of this
            # marker was red the moment it was added - and was
            # committed anyway, because only `| tail -1' was read.
            # `int cmd_completed;' is what makes it cyapawrite()'s.
            "\tstruct cyapa_softc *sc;\n\tint error;\n\tint cmd_completed;\n",
            "cyapawrite() assigns error only inside the copy-in loop "
            "and reads it in the command loop's condition and at the "
            "return; a full FIFO or a zero-length write runs no body",
        ),
    ],
    "hbsd/src/sys/dev/hid/hmt.c": [
        (
            "PBSD: rsize = 0, and the two guards below now test it.",
            "\thid_size_t d_len, fsize, rsize;\n",
            "the Button-type guard reads rsize whether or not the fetch "
            "above it ran, and `= 0' alone would make the guard PASS "
            "rather than fail, because (0 - 1) * 8 converts to unsigned "
            "against hid_location's uint32_t fields",
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
    "hbsd/src/usr.sbin/ctladm/ctladm.c": [
        (
            "PBSD: only when getoption() wrote it.",
            "\t\t\t\t\t\t   &err_type, &argnum, &subopt);\n"
            "\t\t\t\terr_desc.lun_error = err_type;",
            "cctl_error_inject: getoption() writes *cmdnum, *argnum and "
            "*subopt inside the match branch only, so CC_OR_NOT_FOUND "
            "writes none of them -- and both arms stored the local into "
            "err_desc BEFORE the CC_OR_NOT_FOUND test.  This file's two "
            "other getoption() call sites, at :508 and :4341, check first",
        ),
        (
            "\tif (delayloc == NULL) {",
            'you must specify the delaytime with -t", __func__);\n'
            "\t\tretval = 1;\n\t\tgoto bailout;\n\t}\n\n"
            '\tif (strcasecmp(delayloc, "datamove") == 0)',
            "cctl_delay() starts delayloc at NULL and only -l sets it, "
            "but only delaytime was checked -- so `ctladm delay -t 5' "
            "with no -l reached strcasecmp(NULL, \"datamove\")",
        ),
    ],
    "hbsd/src/usr.bin/systat/netstat.c": (
        "PBSD: read the socket on both paths.",
        "\t\tif (istcp) {\n\t\t\tKREAD(inpcb->inp_socket, &sockb, "
        "sizeof (sockb));",
        "fetchnetstat_kvm: the KREAD into sockb was inside the `istcp' "
        "arm and the UDP arm passed the same &sockb unread, so every "
        "UDP socket was displayed with the last TCP socket's queue "
        "counts -- UDP is the second pass, after `goto again'",
    ),
    "hbsd/src/usr.sbin/bsdinstall/partedit/gpart_ops.c": [
        (
            "PBSD: per provider, as the `start = end = 0' above does",
            "\tmaxsize = 0;\n\tfor (i = 0; i < nparts; i++) {",
            "gpart_max_free: partstart and partend are written only "
            "when the provider's config names them, so one that names "
            "neither silently reused the previous partition's extent "
            "and the first reused the frame -- this decides where the "
            "installer offers to write.  maxstart had no initialiser "
            "either",
        ),
        (
            ("const char *errstr, *scheme = NULL;", 2),
            None,
            "gpart_activate() and gpart_bootcode() search the geom's "
            "config list for `scheme' and used the result whether or "
            "not the search found one -- strcmp() and bootcode_path() "
            "on an uninitialised pointer",
        ),
        (
            "const char *scheme = NULL;\n\tconst char *indexstr = NULL;",
            "const char *scheme;\n\tconst char *indexstr;",
            "gpart_partcode(): the same, twice -- scheme into "
            "partcode_path() and indexstr into the gpart command line",
        ),
        (
            "const char *errstr, *oldtype = NULL, *scheme = NULL;",
            "const char *errstr, *oldtype, *scheme;",
            "gpart_edit(): scheme into scheme_supports_labels(), and "
            "oldtype into three strcmp()s after the dialog",
        ),
    ],
    "hbsd/src/usr.sbin/gssd/gssd.c": [
        (
            "PBSD: clamp.  FreeBSD's getgrouplist() sets",
            "\t\t\tgetgrouplist(pw->pw_name, pw->pw_gid,\n"
            "\t\t\t    groups, numgroups);",
            "_gss_get_unix_cred: getgrouplist() sets *grpcnt to the "
            "number of groups FOUND and returns -1 when that exceeds "
            "the array, and the caller then walks `groups' that far -- "
            "a gid_t[NGROUPS] on root's stack whose tail goes to the "
            "kernel GSS layer as a credential's supplementary groups",
        ),
        (
            "PBSD: clamp; see _gss_get_unix_cred().",
            "\t\t\t\tint len = NGROUPS;\n\t\t\t\tint groups[NGROUPS];",
            "gssd_pname_to_uid_1_svc: the same, with a memcpy of "
            "len * sizeof(int) out of the same stack array -- and the "
            "array is now gid_t, copied element by element, which is "
            "what this file already does in "
            "gssd_accept_sec_context()",
        ),
    ],
    "hbsd/src/lib/librpcsec_gss/svc_rpcsec_gss.c": (
        "PBSD: clamp.  getgrouplist() sets *grpcnt to the number",
        "\t\tgetgrouplist(pw->pw_name, pw->pw_gid, uc->gidlist, &len);",
        "svc_rpc_gss_build_ucred: `uc->gidlen = len' with no bound, so "
        "a user in more than NGRPS groups left gidlen larger than "
        "cl_gid_storage -- in the library every RPCSEC_GSS server uses",
    ),
    "hbsd/src/usr.bin/id/id.c": [
        (
            "PBSD: clamp.  getgrouplist() reports the number of groups",
            "\t\tgetgrouplist(pw->pw_name, gid, groups, &ngroups);",
            "id: the group database can list more groups than "
            "_SC_NGROUPS_MAX + 1, and the printing loop walks ngroups "
            "out of an array that holds ngroups_max",
        ),
        (
            "PBSD: clamp; see id().",
            "\t\t(void) getgrouplist(pw->pw_name, pw->pw_gid, groups, "
            "&ngroups);",
            "group: the same, with the ignoring made explicit by a "
            "(void) cast",
        ),
    ],
    "hbsd/src/bin/ed/main.c": [
        (
            # TWO sites, so a count: the `%' expansion and the `f'
            # command both call it, and fixing one is this document's
            # most common shape of half-fix.
            ("strip_escapes(old_filename)) == NULL", 2),
            None,
            "strip_escapes() returns NULL when its REALLOC fails - the "
            "macro returns the caller's `err' argument and this caller "
            "passes NULL - and the `%' expansion passed that straight "
            "to strlen()",
        ),
        (
            "PBSD: terminate.",
            "\t\ts++;\n\treturn file;",
            "strip_escapes: the loop stops either on the NUL it copied "
            "or on running out of buffer, and in the second case "
            "nothing wrote one -- so the caller's strlen() ran off a "
            "PATH_MAX allocation",
        ),
    ],
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
            "PBSD: start the rollback cursors for EVERY operation.",
            "\t\t}\n\n\t\tbinuptime(&start_time);",
            "nfsrvd_compound: the six ERELOOKUP rollback cursors are "
            "saved inside the op switch's `default:' case only, and "
            "the rollback that reads them is AFTER the switch -- so "
            "the five explicit cases reach `nd->nd_md = md' with "
            "nothing having written it",
        ),
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
        (
            "_Static_assert(nitems(devstat_arg_list) == DSM_MAX,",
            None,
            "and the const alone was not the whole invariant. "
            "devstat_compute_statistics() takes the type to pull off its "
            "va_list from devstat_arg_list[metric].argtype and the "
            "pointer to write through from a `switch (metric)' one "
            "screen further down; the two agree only while row i of the "
            "table belongs to metric i. The enum is in devstat.h and the "
            "table in devstat.c, with nothing connecting them. Add a "
            "metric to the middle of the enum without adding its row and "
            "every metric after it reads the WRONG argtype: va_arg() "
            "takes a `long double *' where the caller passed a "
            "`u_int64_t *', and a 16-byte write lands in 8 bytes of the "
            "caller's storage. This is the length half, which a compiler "
            "can check; tools/verify/devstat_metric_table.py reads the "
            "order, which it cannot. Fifty-three clang "
            "core.NullDereference findings in this one function rest on "
            "the invariant, and it was nowhere written down.",
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
    "hbsd/src/sys/contrib/openzfs/module/zfs/zvol.c": [
        (
            "int total = 0, done = 0, last_error = 0, error;",
            "\tint total = 0, done = 0, last_error, error;\n",
            "zvol_create_minors_impl: last_error reached "
            "zvol_task_update_status() unwritten when the `@' arm did not "
            "run and the prefetch list was empty",
        ),
        (
            "int total = 0, done = 0, last_error = 0, error = 0, oldnamelen;",
            None,
            "zvol_rename_minors_impl: error is assigned only for a zvol "
            "whose name matches, so any unrelated zvol in the list read a "
            "stack word in `if (error)'",
        ),
        (
            "PBSD: a switch over a value that came off DISK",
            None,
            "zvol_set_volmode_impl: no default arm, and zt_value is the "
            "volmode property as dsl_prop_get_int_ds() read it",
        ),
    ],
    "hbsd/src/sys/contrib/openzfs/module/zfs/zil.c": (
        "boolean_t slog = B_FALSE;",
        "\tboolean_t slog;\n",
        "zil_lwb_write_issue: slog is written only inside "
        "`if (error == 0)', and an lwb carrying an allocation failure "
        "skips that block and then reads it",
    ),
    "hbsd/src/sys/contrib/openzfs/module/zfs/vdev.c": (
        "uint64_t ivalue = vdev_prop_default_numeric(prop);",
        "\tuint64_t ivalue;\n\n\terr = vdev_prop_get_int(vd, prop, &ivalue);",
        "vdev_prop_get_bool: vdev_prop_get_int() returns EINVAL without "
        "writing *value, and vdev_load() stores the result into "
        "vd->vdev_slow_io_events regardless",
    ),
    "hbsd/src/sys/contrib/openzfs/module/zfs/vdev_raidz.c": [
        (
            "ntgts > 2 ? ltgts[2] : -1, ntgts);",
            "\"ntgts=%u\", zio, ltgts[0], ltgts[1], ltgts[2], ntgts);\n",
            "raidz_reconstruct: the debug line printed ltgts[1] and "
            "ltgts[2] whatever ntgts was, and the caller writes only "
            "num_failures of them",
        ),
        (
            "ASSERT(nmissing == 0 ||",
            None,
            "vdev_raidz_matrix_reconstruct: the assertion indexed "
            "missing[0] when nmissing may be 0, which is the "
            "all-parity-targets case reconstruct_general() is reached with",
        ),
    ],
    "hbsd/src/sbin/ping/ping.c": (
        "PBSD: F_TTL only.",
        "if (!(options & (F_TTL | F_MTTL))) {",
        "the F_HDRINCL path skipped the default-TTL sysctl when -T was "
        "given, and -T assigns `mttl', a different variable - so "
        "`ip.ip_ttl = ttl' put a stack word in every header ping built",
    ),
    "hbsd/src/usr.sbin/ppp/async.c": (
        "struct mbuf *nbp = NULL, **last;",
        "  struct mbuf *nbp, **last;\n",
        "async_LayerPull returned nbp having decoded no bytes - a bp that "
        "is NULL on entry, or whose mbufs are all m_len 0 - so the caller "
        "walked and freed a wild mbuf pointer",
    ),
    "hbsd/src/usr.sbin/ppp/radius.c": (
        "if (mlen < SALT_LEN + 16 || mlen % 16 != SALT_LEN) {",
        "if (mlen % 16 != SALT_LEN) {",
        "demangle(): mlen == SALT_LEN passes the modulus test, Clen is 0, "
        "alloca(0) is a zero-sized object and `*len = *P' reads past it - "
        "lib/libradius/radlib.c's copy of this function was fixed and "
        "this one was not",
    ),
    "hbsd/src/usr.sbin/rtsold/rtsol.c": (
        "PBSD: advance p, not addr.",
        # No "must be absent" string: the DNSSL block this fixes was a
        # copy of the RDNSS block at :435-441, byte for byte, and there
        # `addr' IS the cursor and `addr++' is right. Any string short
        # enough to be worth writing matches both sites, and a marker
        # that matches the site it does not guard is worse than none.
        None,
        "rtsol_input's DNSSL arm incremented `addr', the RDNSS cursor, "
        "which it had never written - and skipped the `p += len' at the "
        "bottom of the loop, so a second strdup() failure decoded the "
        "same name again",
    ),
    "hbsd/src/usr.bin/mdo/mdo.c": (
        "PBSD: the bound is tested BEFORE the index",
        "\t\t\tcand = set->groups[++from];\n\t\t\tif (from == set->nb)\n",
        "remove_groups read one element past the end of a heap array at "
        "all four sites before asking whether the index had reached nb - "
        "in a setuid program",
    ),
    "hbsd/src/usr.sbin/ppp/mp.c": (
        "PBSD: a rejected header is zeroed",
        "      log_Printf(LogWARN, \"Oops - MP header without required "
        "zero bits\\n\");\n      return 0;\n",
        "mp_ReadHeader returned 0 without writing header->begin or "
        "->end, and four of its six callers ignore the return - so a "
        "peer setting the MP header's reserved bits chose which "
        "fragments ppp dropped",
    ),
    "hbsd/src/sbin/routed/if.c": (
        "PBSD: start the alias prototype.",
        "\t}\n\n\t/* XXX: thanks to malloc(3), alignment can be presumed "
        "OK */",
        "ifinit: ifs0 is filled only in the RTM_IFINFO arm, which then "
        "continues, so an RTM_NEWADDR not preceded by one copies this "
        "frame into the interface record and ORs alias flags into it",
    ),
    "hbsd/src/sys/cam/ctl/ctl_frontend_iscsi.c": [
        (
            "PBSD: the rest of this function assumes ext_data_filled is "
            "within",
            "\t    expected_len - io->scsiio.kern_rel_offset);\n\n"
            "\ttarget_transfer_tag =",
            "cfiscsi_datamove_out: expected_len is the initiator's own "
            "bhssc_expected_data_transfer_length, and both the "
            "scatter-gather walk and the uint32_t "
            "`datamove_len - ext_data_filled' rest on it bounding "
            "ext_data_filled",
        ),
        (
            "PBSD: test before loading, so consuming the list exactly "
            "does not",
            "\t\tif (r2t_off >= cdw->cdw_sg_len) {",
            "cfiscsi_datamove_out: the walk advanced cdw_sg_index and "
            "LOADED the entry before re-testing r2t_off, so consuming "
            "the list exactly read one entry past its end -- which on "
            "the kern_sg_entries == 0 path is a single stack "
            "ctl_sg_entry, and what came back became the address the "
            "next Data-Out is copied to",
        ),
    ],
    "hbsd/src/usr.bin/env/envopts.c": (
        "PBSD: the first entry may BE the terminator.",
        "\t\tfprintf(stderr, \"#env      into:\\t'%s'\\n\", *oldarg);\n"
        "\t\tfor (oldarg++;",
        "split_spaces: `*nextarg = NULL' writes newargv[1] when the -S "
        "string produced no arguments, and the -v -v dump printed that "
        "NULL and stepped past it into the rest of the malloc",
    ),
    "hbsd/src/usr.bin/mkimg/mkimg.c": (
        "PBSD: and error, which neither switch below assigns on",
        "\t\tbyteoffset = blkoffset = 0;\n\t\tabs_offset = false;\n\n"
        "\t\t/* Look for an offset.",
        "mkimg: a PART_KIND_SIZE partition falls through both switches "
        "untouched, so `if (error)' tested whatever the previous "
        "partition left",
    ),
    "hbsd/src/usr.sbin/bhyve/amd64/fwctl.c": (
        "PBSD: 0xffffffff, which is what the default arm below already",
        "\tuint32_t retval;\n\n\tswitch (be_state) {",
        "fwctl_inl: fwctl_response()'s default arm writes *retval only "
        "when remlen is positive and returns anyway, and this value "
        "goes straight out of the fwctl I/O port to the guest",
    ),
    "hbsd/src/usr.sbin/pmcstudy/eval_expr.c": (
        "PBSD: say so, as the two `rest' arms below already do.",
        "\tif (op == NULL) {\n\t\treturn (val1);\n\t}",
        "run_expr: the one return that left *lastone unwritten, and "
        "gather_exp_to_paren_close() returns it for the caller to walk "
        "as a struct expression *",
    ),
    "hbsd/src/usr.bin/col/col.c": (
        "PBSD: parenthesise, and clear what was ALLOCATED.",
        "memset(count, 0, sizeof(int) * l->l_max_col + 1);",
        "flush_line: `sizeof(int) * l_max_col + 1' is three bytes short "
        "of the sizeof(int) * count_size the realloc asks for, so the "
        "top three bytes of count[l_max_col] survived from a previous "
        "line and the running total sent sorted[count[..]++] past the "
        "end of sorted -- the input stream decides how far",
    ),
    "hbsd/src/usr.bin/fmt/fmt.c": (
        "PBSD: terminate the buffer.",
        "\t}\n\t*lengthp = len;",
        "get_line returns the length out of band and never wrote a NUL; "
        "might_be_header() walks the line as a wide STRING, so a line "
        "shorter than the longest seen so far read into what the "
        "previous one left there",
    ),
    "hbsd/src/usr.bin/gprof/arcs.c": [
        (
            "PBSD: nothing to pick.",
            "    } else {\n\t/*\n\t *\tlast choice is edge leading to node "
            "with only this arc as",
            "compresslist: the three max*arcp are written only when their "
            "max*cnt rises above 0, and the last arm was an unconditional "
            "`else' -- so a list whose arcs all have arc_cyclecnt 0 WROTE "
            "through a pointer nothing had set",
        ),
        (
            "\t\tnextclp = clp -> next;\n\t\tfree( clp );\n\t\tclp = nextclp;",
            "\t\tcyclecnt--;\n\t\tclp = clp -> next;\n\t\tfree( clp );",
            "cycleanalyze() advanced clp to the next node and then freed "
            "THAT one -- cyclehead itself was never released, the node "
            "still linked behind it was, and the next iteration read "
            "clp -> list and clp -> size through the freed pointer",
        ),
        (
            "\tnextclp = clp -> next;\n\t*prev = nextclp;\n\tfree( clp );",
            "\t*prev = clp -> next;\n\tclp = clp -> next;\n\tfree( clp );",
            "compresslist() unlinked clp, advanced, and freed the node it "
            "had advanced to -- the unlinked node leaked, the freed one "
            "was still on the list through *prev, and the next iteration "
            "walked it",
        ),
    ],
    "hbsd/src/usr.bin/patch/pch.c": (
        "PBSD: n == 0 means neither the copy loop above nor the "
        "blank-line",
        "\t\tn++;\n\t}\n\tif (p_char[0] != '=')",
        "pch_swap: a hunk with no replacement half leaves p_line[0] and "
        "p_char[0] fresh from set_hunkmax()'s malloc, and if that byte "
        "happened to be '=' the loop after it walked an unwritten "
        "char * to its first NUL",
    ),
    "hbsd/src/usr.bin/rpcgen/rpc_parse.c": [
        (
            "PBSD: start the fields this function does not always "
            "write.",
            "\ttoken tok;\n\n\tget_type(&dec->prefix, &dec->type, "
            "dkind);",
            "get_declaration leaves name unwritten on the `void' early "
            "return and array_max unwritten for every non-array "
            "declaration, and def_typedef() copies both",
        ),
        (
            "PBSD: `typedef void x;' makes get_declaration() return "
            "after the",
            "\tget_declaration(&dec, DEF_TYPEDEF);\n\tdefp->def_name = "
            "dec.name;",
            "def_typedef: there is no name to define after `void', and "
            "check_type_name() strcmp()s it against every reserved word",
        ),
        (
            "PBSD: the same gap as get_declaration(), found by reading "
            "it.",
            "\tchar name[10];\t\t/* argument name */\n\n\tif (dkind == "
            "DEF_PROGRAM) {",
            "get_prog_declaration: array_max unwritten on the "
            "no-arguments early return and for every non-array argument",
        ),
    ],
    "hbsd/src/usr.bin/rpcgen/rpc_util.h": (
        "void error(const char *msg) __dead2;",
        "void error(const char *msg);\n",
        "error() and expected1/2/3() all end in crash(), which is "
        "already __dead2; saying so is what makes a caller's "
        "`if (x == NULL) error(...)' protect the line below it",
    ),
    "hbsd/src/usr.bin/top/commands.c": (
        "PBSD: reject before negating.",
        "    if (procnum == -1 || prio < PRIO_MIN || prio > PRIO_MAX)",
        "renice_procs: scanint() returns -1 without writing *prio, and "
        "the negation ran before the validity test",
    ),
    "hbsd/src/usr.sbin/bluetooth/rtlbtfw/rtlbt_hw.c": (
        "PBSD: -1, so a loop that runs no iterations fails closed.",
        "\tint i, j;\n\tint ret, transferred;",
        "rtlbt_load_fwfile: frag_num is a size_t expression converted "
        "to int, and `return (ret)' hands the caller a stack word when "
        "the loop runs no iterations",
    ),
    "hbsd/src/usr.sbin/fdread/fdread.c": (
        "PBSD: 0 is the driver's actual state, and nothing else reads "
        "it.",
        "\tint rv, fdopts, recoverable, nerrs = 0;",
        "doread: fdopts is OR-ed with FDOPT_NOERROR and handed to "
        "FD_SOPTS, which writes the whole option word -- and there is "
        "no FD_GOPTS anywhere in this program",
    ),
    "hbsd/src/usr.sbin/sa/main.c": (
        "PBSD: ci_flags is only ever OR-ed into, never started.",
        "\t\t/* decode it */\n\t\tci.ci_calls = 1;",
        "acct_load: ci is reused per record and handed whole to "
        "pacct_add() and usracct_add(), so the flag word entering the "
        "accounting databases came off the frame",
    ),
    "hbsd/src/usr.sbin/bhyve/pci_e82545.c": [
        (
            "PBSD: zero all of ckinfo, not just ck_valid.",
            "\tckinfo[0].ck_valid = ckinfo[1].ck_valid = 0;",
            "e82545_transmit: ck_start, ck_off and ck_len are written only "
            "in the arms that set ck_valid or under `|| tso', and the VLAN "
            "insertion correction adds to all six with no ck_valid test -- "
            "which cannot BE guarded on ck_valid, because a TSO packet "
            "without IXSM has ck_valid 0 and still uses ck_start",
        ),
        (
            "if (hdrlen != 0 && iovcnt > 0 && iov[0].iov_len > hdrlen &&",
            "if (hdrlen != 0 && iov[0].iov_len > hdrlen &&",
            "e82545_transmit: a guest TX chain of all-zero-length "
            "descriptors leaves iovcnt at 0 and iov[0] untouched, and the "
            "guest sets hdrlen non-zero by asking for a checksum offload",
        ),
    ],
    "hbsd/src/usr.sbin/bhyve/tpm_intf_crb.c": [
        (
            "union tpm_crb_reg_loc_ctrl loc_ctrl = { 0 };",
            "\t\t\tunion tpm_crb_reg_loc_ctrl loc_ctrl;\n",
            "tpm_crb_mem_handler: the guest picks the MMIO width and "
            "tpm_crb_mmiocpy() copies exactly that many bytes into a "
            "four-byte union, whose bitfields are then read",
        ),
        (
            "union tpm_crb_reg_ctrl_req req = { 0 };",
            "\t\t\tunion tpm_crb_reg_ctrl_req req;\n",
            "tpm_crb_mem_handler: the ctrl_req case, same shape",
        ),
        (
            "union tpm_crb_reg_ctrl_start start = { 0 };",
            "\t\t\tunion tpm_crb_reg_ctrl_start start;\n",
            "tpm_crb_mem_handler: the ctrl_start case, same shape -- "
            "this one decides whether a TPM command runs",
        ),
    ],
    "hbsd/src/usr.sbin/bhyve/pci_emul.c": (
        "PBSD: `value = 0' used to sit inside the baridx == 0",
        "\tuint32_t value;\n\tint i;",
        "pci_emul_dior: the memory branch has an `unknown size' arm "
        "too, and returned a value nothing wrote",
    ),
    "hbsd/src/usr.bin/sdiotool/cam_sdio.c": [
        (
            "\tuint8_t val = 0;\n\t*ret = sdio_rw_direct",
            "\tuint8_t val;\n\t*ret = sdio_rw_direct",
            "sdio_read_1 returns val whatever *ret says, and the CAM "
            "transfer that fills it does not run when the ccb fails",
        ),
        (
            "\tuint16_t val = 0;\n\t*ret = sdio_rw_extended",
            "\tuint16_t val;\n\t*ret = sdio_rw_extended",
            "sdio_read_2, same shape",
        ),
        (
            "\tuint32_t val = 0;\n\t*ret = sdio_rw_extended",
            "\tuint32_t val;\n\t*ret = sdio_rw_extended",
            "sdio_read_4, same shape",
        ),
        (
            "char *cis1_info[4] = { NULL, NULL, NULL, NULL };",
            "\tchar *cis1_info[4];\n",
            "sdio_func_read_cis fills cis1_info[0..count-1] and stops "
            "at the first 0xff the card returns; the print loop reads "
            "all four and hands each to printf as %s",
        ),
    ],
    "hbsd/src/usr.sbin/mptutil/mpt_config.c": [
        (
            "PBSD: returns 0, or an errno VALUE -- never -1.",
            "state->nsdisks, &i) < 0) {",
            "build_raid_volume: mpt_lookup_standalone_disk() returns 0 "
            "or an errno value, and all three callers tested `< 0', so "
            "EINVAL and ENOENT read as success and disks[] was indexed "
            "with the *index it had not written",
        ),
        (
            "error = mpt_lookup_standalone_disk(av[1], sdisks, nsdisks,",
            "if (mpt_lookup_standalone_disk(av[1], sdisks, nsdisks, &i) <",
            "add_spare: the second of the three `< 0' call sites",
        ),
        (
            "error = mpt_lookup_standalone_disk(av[1], disks, ndisks, "
            "&i);",
            "if (mpt_lookup_standalone_disk(av[1], disks, ndisks, &i) < "
            "0) {",
            "create_physdisk: the third of the three `< 0' call sites",
        ),
        (
            "PBSD: errno, as the two other mpt_vol_info() failures",
            "\tif (info == NULL)\n\t\treturn (error);",
            "find_volume_spare_pool: the last mpt_vol_info() failure "
            "returned `error', which on every path that reaches it is "
            "0, so a failed volume page read reported success with "
            "*pool never written",
        ),
        (
            ("if (mpt_lock_volume(vol->VolumeBus, vol->VolumeID) != 0) {", 1),
            "if (mpt_lock_volume(vol->VolumeBus, vol->VolumeID) < 0) {",
            "mpt_lock_volume(), mpt_lock_physdisk(), "
            "mpt_create_physdisk(), mpt_delete_physdisk() and "
            "mpt_lookup_drive() all return 0 or a POSITIVE errno, as "
            "mpt_raid_action() under them does -- so all eleven `< 0' "
            "error tests in this file were dead, and the arms behind "
            "them then read an errno nobody had set",
        ),
        (
            ("error = mpt_create_physdisk(", 3),
            "if (mpt_create_physdisk(fd, &sdisks[i], &PhysDiskNum) < 0) {",
            "the three mpt_create_physdisk() call sites, whose dead "
            "arms left PhysDiskNum unwritten and then passed it to "
            "mpt_pd_info()",
        ),
        (
            ("error = mpt_delete_physdisk(", 2),
            "if (mpt_delete_physdisk(fd, PhysDiskNum) < 0) {",
            "and the delete side",
        ),
    ],
    "hbsd/src/usr.bin/kdump/kdump.c": [
        (
            "PBSD: a record is only as long as the file says it is.",
            '\t\t\terrx(1, "data too short");\n'
            "\t\tif (fetchprocinfo(&ktr_header, (u_int *)m) != 0)",
            "main: every fixed-layout record type was dispatched to a "
            "handler that reads the whole struct with no test that "
            "ktr_len is that big -- a zero-length record read the "
            "malloc(1025) buffer this run never wrote, and a crafted "
            "ktr_narg walked print_number() off the end of it",
        ),
        (
            "\t\tif ((trpoints & (1<<type)) == 0)",
            "\t\tif ((trpoints & (1<<ktr_header.ktr_type)) == 0)",
            "main: the dispatch switches on a local copy of "
            "ktr_type, because findabi(), dumpheader() and "
            "fetchprocinfo() all take &ktr_header by non-const "
            "pointer and the analyser forgets the length guard's case "
            "across them -- none of the three writes to the header",
        ),
    ],
    "hbsd/src/sbin/init/init.c": [
        (
            "PBSD: stop at the terminator rather than at "
            "SCRIPT_ARGV_SIZE.",
            "\tfor (i = 0; i != SCRIPT_ARGV_SIZE; ++i)\n"
            "\t\tsh_argv[i + sh_argv_len] = argv[i];",
            "execute_script: the copy loop ran a fixed count rather "
            "than to the NULL, so replace_init()'s argv[2] -- which "
            "it never fills -- was loaded in pid 1",
        ),
        (
            "PBSD: the other two callers fill all three",
            "\targv[0] = path;\n\targv[1] = NULL;\n\n"
            "\texecute_script(argv);",
            "replace_init: fills argv[0] and argv[1] where "
            "run_script() and run_rc_shutdown() fill all three",
        ),
    ],
    "hbsd/src/usr.bin/gzip/unpack.c": (
        "PBSD: start the accumulator.",
        "\tunpack_descriptor_t unpackd;\n\n\tin = dup(in);",
        "unpack() is the one decompressor in gzip(1) whose byte count "
        "ACCUMULATES into the caller's variable - accepted_bytes() does "
        "`(*bytes_in) += newbytes' - and handle_stdin()'s `off_t usize, "
        "gsize' at gzip.c:1750 is not zeroed, so `gzip -d' on a pack(1) "
        "file reported a stack word as the compressed size",
    ),
    "hbsd/src/usr.sbin/setaudit/setaudit.c": (
        "term_port = 0;",
        "\tbzero(&hints, sizeof(hints));\n\tterm_type = AU_IPv4;\n",
        "term_port is assigned only under `-p' and read whenever `-U' "
        "was not given, so the ordinary invocation set the audit "
        "terminal ID's port from the frame",
    ),
    "hbsd/src/sys/fs/ext2fs/ext2_bmap.c": (
        "daddr_t blkno = -1;",
        "\tdaddr_t blkno;\n\tint error;\n",
        "ext2_bmap: the store to *ap->a_bnp is unconditional and "
        "ext2_bmaparray() returns ext2_getlbns()'s error having written "
        "nothing, so a caller reading the block number before the error "
        "got a stack word",
    ),
    "hbsd/src/sys/ufs/ufs/ufs_bmap.c": (
        "ufs2_daddr_t blkno = -1;",
        "\tufs2_daddr_t blkno;\n\tint error;\n",
        "ufs_bmap: the same shape as ext2_bmap, with three returns in "
        "ufs_bmaparray() that write no *bnp - found by reading the "
        "sibling, not by the analyser",
    ),
    "hbsd/src/sys/geom/concat/g_concat.c": (
        "int disk_candelete = 0;",
        "\tint disk_candelete;\n",
        "g_concat_ctl_append: the `disk_candelete = 0' fallback is "
        "inside the arm where g_access() succeeded, so a consumer that "
        "could not be opened left it a stack word - and it decides "
        "whether BIO_DELETE reaches that member",
    ),
    "hbsd/src/sys/sys/mbuf.h": (
        "M_EXTPG mbuf %p has no linear data area",
        "\tint adjust;\n\tKASSERT(m->m_data == M_START(m),\n",
        "m_align: M_START() is NULL for an M_EXTPG mbuf, so the "
        "assertion that follows was read as BINDING m_data to NULL and "
        "every one of the 61 M_ALIGN call sites reported a null "
        "dereference on `p = m->m_data'",
    ),
    "hbsd/src/sys/contrib/openzfs/module/os/freebsd/zfs/spa_os.c": (
        "nvlist_t *best_cfg = NULL;",
        "\tnvlist_t *best_cfg, *nvtop, *nvroot;\n",
        "spa_generate_rootconf: best_cfg was assigned only under "
        "`txg > best_txg' with best_txg starting at 0, and the next line "
        "passed it to nvlist_lookup_uint64() - at boot, on labels read "
        "off the root pool's disks",
    ),
    "hbsd/src/sys/contrib/openzfs/module/lua/llimits.h": (
        "defined(__COVERITY__) || defined(__clang_analyzer__))",
        "#if defined(__GNUC__) && !defined(_KERNEL)\n#define l_noret",
        "l_noret dropped __attribute__((noreturn)) in every kernel build "
        "for a Linux objtool warning, so an analyser walked out of "
        "luaX_syntaxerror() and read the caller's uninitialised locals; "
        "restored under the same two macros debug.h:85 uses for spl_panic",
    ),
    "hbsd/src/sys/cddl/dev/dtrace/x86/dis_tables.c": [
        (
            "PBSD: the outputs are written BEFORE either early return",
            "\tint byte;\n\n\tif (x->d86_error)\n\t\treturn;\n",
            "dtrace_get_SIB: both early returns wrote nothing, and "
            "dtrace_get_modrm() passes &mode, &reg and &r_m through it "
            "and then sets d86_got_modrm so nothing retries",
        ),
        (
            "uint_t reg = 0;\t\t/* reg value from ModRM byte */",
            "\tuint_t reg;\t\t/* reg value from ModRM byte */\n",
            "dtrace_disx86: reg and r_m left uninitialised beside a "
            "`mode' that was not, and the table-indirection path takes "
            "the ModRM byte into opcode3 so a later call writes nothing",
        ),
        (
            "const instable_t *dp = NULL; /* decode table being used */",
            "\tconst instable_t *dp;\t/* decode table being used */\n",
            "dtrace_disx86: the zero-padding arm does `goto done' from "
            "above the only assignment of dp, and the DIS_MEM block at "
            "done: dereferences it",
        ),
    ],

    "hbsd/src/sys/vm/vm_page.c": (
        "\tm = NULL;\n\tif (__predict_false((req & VM_ALLOC_NOFREE) != 0)) {",
        "again:\n\tif (__predict_false((req & VM_ALLOC_NOFREE) != 0)) {",
        "vm_page_alloc_noobj_domain: m is only written inside the three "
        "allocation arms, and the ordinary out-of-memory path takes none "
        "of them, so the `m == NULL' test read an uninitialised local and "
        "a garbage non-zero was returned as a page",
    ),
    "hbsd/src/sys/kern/link_elf.c": [
        (
            ("\terror = link_elf_symbol_values(lf, sym, &symval);", 2),
            "\tlink_elf_symbol_values(lf, sym, &symval);\n\tif (symval.value == 0) {",
            "link_elf_lookup_set: both calls ignored the return, and "
            "link_elf_debug_symbol_values() returns ENOENT without writing "
            "*symval, so symval.value was read uninitialised and walked as "
            "a linker set",
        ),
        (
            "&& sym != NULL &&\n\t    off == 0) {",
            "if (link_elf_search_symbol(lf, val, &sym, &off) == 0 && off == 0) {",
            "link_elf_ifunc_symbol_value: link_elf_search_symbol() always "
            "returns 0 and sets *sym NULL when it matches nothing, with "
            "*diffp the raw address -- so `off == 0' is also true for a "
            "resolver that returned NULL and es->st_value dereferences it",
        ),
    ],
    "hbsd/src/sys/net/route/route_helpers.c": (
        "\twhile (cp < ep)\n\t\t*cp++ = 0;",
        "\t\t*cp = htonl(mask ? ~((1 << (32 - mask)) - 1) : 0);",
        "ip6_writemask left the words past the prefix at whatever the "
        "caller had there; rt_get_inet6_parent() passes an uninitialised "
        "local and then reuses it across a widening loop",
    ),
    "hbsd/src/sys/kern/sys_generic.c": (
        "\tobits[0] = obits[1] = obits[2] = NULL;",
        "\tsbp = selbits;\n#define\tgetbits(name, x)",
        "kern_select: a getbits() copyin failure goes to done:, which "
        "swizzle_fdset()s all three obits -- and on big-endian LP64 that "
        "macro writes through the pointer it was handed",
    ),
    "hbsd/src/sys/net/iflib.c": (
        "\tint i, err = 0;\n\tiflib_dma_info_t *dmaiter;",
        "\tint i, err;\n\tiflib_dma_info_t *dmaiter;",
        "iflib_dma_alloc_multi: for count <= 0 the loop never runs and "
        "err is returned uninitialised",
    ),
    "hbsd/src/sys/net/if_lagg.c": (
        "\t\tif_type = IFT_INFINIBANDLAG;\n\t\tbreak;\n\tdefault:\n\t\t/*",
        "\t\tif_type = IFT_INFINIBANDLAG;\n\t\tbreak;\n\tdefault:\n\t\tbreak;",
        "lagg_port_create: the default arm fell through with if_type "
        "unset, and it is stored into the member interface's if_type",
    ),
    "hbsd/src/sys/kern/uipc_sockbuf.c": (
        "\tcase SO_RCVLOWAT:\n\t\tbreak;\n\tdefault:\n\t\treturn (EINVAL);\n\t}",
        "\tcc = optval;\n\n\tsb = NULL;",
        "sbsetopt: neither switch had a default, so an unmatched name "
        "left wh undefined and sb NULL for SOCK_BUF_LOCK() and the three "
        "loads above it",
    ),
    "hbsd/src/sys/kern/coredump_vnode.c": (
        "\tif (error == 0 && nextvp == NULL)\n\t\terror = EINVAL;",
        "\tnextvp = oldvp = NULL;\n\tcmode = S_IRUSR | S_IWUSR;",
        "corefile_open_last: debug.ncores can be 0, the loop then never "
        "runs, and both arms of the tail read error uninitialised -- a "
        "garbage zero publishes a NULL *vpp as a successful open",
    ),
    "hbsd/src/sys/kern/kern_shutdown.c": (
        "\tif (dip == NULL || di_template == NULL || di_template->blocksize == 0)",
        "\tif (dip == NULL)\n\t\treturn (EINVAL);",
        "dumper_create accepted a zero blocksize from the driver "
        "template, which dump_check_bounds() divides by mid-dump",
    ),
    "hbsd/src/sys/kern/kern_timeout.c": (
        "\tif (count == 0) {\n\t\tprintf(\"Scheduled callouts statistic snapshot:",
        "\t\tCC_UNLOCK(cc);\n\t}\n\n\tfor (i = 0, tcum = 0;",
        "sysctl_kern_callout_stat: st / count and spr / count have no "
        "zero guard, and nothing establishes that count is non-zero",
    ),

    "hbsd/src/sys/kern/kern_descrip.c": (
        "\tif (ret != 0)\n\t\tsigiofree(sigio);\n\treturn (ret);",
        "\t\tsigiofree(osigio);\n\treturn (ret);",
        "fsetown: every failing path left the new sigio neither stored "
        "nor freed, so an unprivileged fcntl(F_SETOWN) against a pid in "
        "another session leaked the allocation and a ucred reference, "
        "once per call and without bound",
    ),
    "hbsd/src/sys/kern/kern_proc.c": (
        "\t\tstack_destroy(st);\n\t\tfree(kkstp, M_TEMP);\n\t\treturn (error);",
        "\t\tPROC_UNLOCK(p);\n\t\treturn (error);\n\t}\n\tdo {",
        "sysctl_kern_proc_kstack: the p_candebug() early return took "
        "neither of the two allocations with it, so an unprivileged read "
        "of kern.proc.kstack.<pid> leaked both per call",
    ),
    "hbsd/src/sys/kern/uipc_accf.c": (
        "\t\tif (error != 0)\n\t\t\tfree(p, M_ACCF);\n\t\tbreak;",
        "\t\terror = accept_filt_add(p);\n\t\tbreak;",
        "accept_filt_generic_mod_event: accept_filt_add() returns EEXIST "
        "without freeing its argument, and neither did the caller",
    ),
    "hbsd/src/sys/kern/uipc_usrreq.c": (
        "\t\tuio = NULL;\n\t\tresid = 0;\n\t\tuipc_reset_kernel_mbuf(m, &mc);",
        "\t} else\n\t\tuipc_reset_kernel_mbuf(m, &mc);",
        "uipc_sosend_stream_or_seqpacket: uio and resid were left "
        "undefined on the kernel-mbuf send path, which then tests uio "
        "and writes uio->uio_resid through it",
    ),
    "hbsd/src/sys/netinet/ip_mroute.c": (
        "\t\tfree(u, M_MRTABLE);\n\t}",
        'cannot enqueue upcall\\n");\n\tif (buf_ring_count',
        "bw_meter_prepare_upcall: a full ring does not take the pointer "
        "and nothing freed it, so the leak is worst under the load that "
        "fills the ring",
    ),
    "hbsd/src/sys/kern/link_elf_obj.c": (
        "&& sym != NULL &&\n\t    off == 0) {",
        "if (link_elf_search_symbol(lf, val, &sym, &off) == 0 && off == 0) {",
        "link_elf_ifunc_symbol_value: the same NULL *sym as link_elf.c, "
        "in the sibling linker class",
    ),
    "hbsd/src/sys/netinet/libalias/alias_sctp.c": (
        "\t\t\t\tsn_free(G_addr);\n\t\t\t\treturn (0);",
        "s_addr == iter_G_Addr->g_addr.s_addr)\n\t\t\t\treturn (0);",
        "Add_Global_Address_to_List declined a duplicate address without "
        "freeing it, and num_Gaddr is only bumped on success, so the "
        "caller's limit never trips and a repeated address parameter "
        "leaks per repeat",
    ),
    "hbsd/src/sys/netinet/in_fib_dxr.c": (
        "\t\tda->updates_low = DIRECT_TBL_SIZE - 1;\n\t\tda->updates_high = 0;",
        "malloc(sizeof(*dxr->aux), M_DXRAUX, M_NOWAIT);",
        "dxr_build: the aux struct was allocated without M_ZERO and the "
        "init block named neither updates_low nor updates_high, which "
        "index the chunk walk and the updates_mask bzero",
    ),

    "hbsd/src/sys/net/if_bridge.c": (
        "\t\t\tif_inc_counter(ifp, IFCOUNTER_IERRORS, 1);\n\t\t\tm_freem(m);",
        "m = m_pullup(m, ETHER_HDR_LEN);\n\t\tif (m == NULL) {\n\t\t\tif_inc_counter(sc->sc_ifp",
        "bridge_input: the m_pullup() failure path counted on sc->sc_ifp "
        "with sc declared NULL and not assigned until after that block",
    ),
    "hbsd/src/sys/netinet/cc/cc_newreno.c": (
        ("\t\tif (nreno != NULL &&\n\t\t    (nreno->newreno_flags & CC_NEWRENO_HYSTART_ENABLED)) {", 2),
        "\t\tif (nreno->newreno_flags & CC_NEWRENO_HYSTART_ENABLED) {",
        "newreno_cong_signal: the CC_NDUPACK and CC_ECN arms dereferenced "
        "nreno unguarded, in a function whose first three lines all test "
        "it for NULL because newreno_cb_init() can leave cc_data NULL",
    ),
    "hbsd/src/sys/kern/subr_firmware.c": (
        "\t\tif (fp->fw.name == NULL)\n\t\t\tcontinue;\n\t\tif (strcasecmp(name, fp->fw.name) == 0)",
        "\t\tif (fp->fw.name != NULL && strcasecmp(name, fp->fw.name) == 0)",
        "lookup(): the absolute-path test dereferenced fw.name on exactly "
        "the path where the NULL check above it had just failed",
    ),

    "hbsd/src/sys/dev/virtio/pci/virtio_pci_legacy.c": (
        "\tfor (i = 0; i < nitems(res_types); i++) {",
        "\tfor (i = 0; nitems(res_types); i++) {",
        "vtpci_legacy_alloc_resources: the loop condition was the "
        "constant nitems(res_types), so a device offering neither BAR0 "
        "type walked i past the end of the array, forever",
    ),
    "hbsd/src/sys/dev/ichiic/ig4_iic.c": [
        (
            "\tint error = 0;\n\n\tif (len == 0)",
            "\tint burst, target, lowat = 0;\n\tint error;",
            "ig4iic_read: error is written only by wait_intr(), so a "
            "transfer that never waited returned an uninitialised local",
        ),
        (
            "\tint error = 0, lowat;",
            "\tint error, lowat;",
            "ig4iic_write: same, for a write that fits the TX FIFO in "
            "one pass",
        ),
    ],
    "hbsd/src/sys/dev/liquidio/lio_sysctl.c": (
        "\tint\terr = 0;",
        "\tuint32_t\t\trx_max_pending = 0, tx_max_pending = 0;\n\tint\terr;",
        "lio_get_ringparam: the switch has no default, so an arg2 "
        "matching neither case returned an uninitialised errno",
    ),
    "hbsd/src/sys/dev/sdio/sdiob.c": [
        (
            "\tchar *cis1_info[4] = { NULL, NULL, NULL, NULL };",
            "\tchar *cis1_info[4];",
            "sdio_func_read_cis: the loop fills up to four entries and "
            "the print loop walks all four -- the kernel twin of the "
            "same defect already fixed in usr.bin/sdiotool/cam_sdio.c",
        ),
        (
            "\t\terror = EINVAL;\nerr:",
            "SD_IO_CIS_SIZE) {\nerr:",
            "sdio_get_common_cis_addr: the out-of-range arm leaves "
            "*addr unwritten and returns `error', which on that path is "
            "0, so the caller read the CIS from an uninitialised local",
        ),
        (
            "\tuint32_t a = 0;",
            "\tint error;\n\tuint32_t a;\n\tuint8_t val;",
            "sdio_get_common_cis_addr: the err: label sits inside the "
            "body of the `if (a < SD_IO_CIS_START ...)' that follows "
            "it, so the three `goto err' jump past the assignments "
            "that build the value CAM_DEBUG there prints",
        ),
    ],
    "hbsd/src/sys/dev/ath/ath_hal/ar9002/ar9280_olc.c": (
        "\t\t\tuint16_t diff = 0;",
        "\t\t\tuint16_t diff;",
        "ar9280ChangeGainBoundarySettings() returns *diff and writes it "
        "on only one of its paths, handing this variable straight back "
        "unwritten to bound a NUM_PDADC() loop",
    ),
    "hbsd/src/sys/dev/firewire/fwohci.c": [
        (
            "\t\t\tdb_tr->dbcnt++;\n\t\t} else if (db_tr->dbcnt == 0) {",
            "\t\t}\n\t\tdb_tr->dbcnt++;",
            "fwohci_add_rx_buf: dbcnt was bumped whether or not the "
            "dbuf slot was filled, so the OHCI descriptor's bus address "
            "came from an unwritten slot of a stack array",
        ),
        (
            "\t\t\tdbuf[0] = FWOHCI_DMA_READ(db[0].db.desc.addr);",
            "\t\t\t\treturn (ENOMEM);\n\t\t}\n\t\tdb_tr->dbcnt = 1;",
            "fwohci_add_rx_buf: fwdma_malloc_size() is the only writer "
            "of dbuf[0] and it is skipped when the buffer survives a "
            "stop/start, so a re-armed descriptor got its bus address "
            "from an unwritten stack slot",
        ),
    ],
    "hbsd/src/sys/dev/qlxgbe/ql_misc.c": (
        "\tif (ql_rd_flash32(ha, flash_off, &mac_hi) != 0)\n\t\treturn;",
        "\tql_rd_flash32(ha, flash_off, &mac_lo);",
        "ql_read_mac_addr dropped both ql_rd_flash32() returns, and "
        "none of that function's three failure paths writes *data -- so "
        "a failed flash read gave the interface a MAC of stack bytes",
    ),
    "hbsd/src/sys/dev/qcom_ess_edma/qcom_ess_edma_rx.c": [
        (
            "\t\tif (port_id >= 0 &&\n\t\t    port_id < (int)nitems(sc->sc_gmac_port_map) &&",
            "\t\tif (sc->sc_gmac_port_map[port_id] != -1) {",
            "qcom_ess_edma_rx_ring_complete: port_id is decoded only "
            "when the hardware marked the return descriptor valid, and "
            "the gmac lookup indexed two arrays with it regardless",
        ),
        (
            "\t\t\thash_type = EDMA_RRD_RSS_TYPE_NONE;\n\t\t\thash_val = 0;",
            "\t\t\tlen = 0;\n\t\t}\n",
            "qcom_ess_edma_rx_ring_complete: the same arm left vlan, "
            "priority, hash_type and hash_val undefined too, and the "
            "VLAN and hash blocks wrote them into m_pkthdr.ether_vtag "
            "and m_pkthdr.flowid",
        ),
    ],
    "hbsd/src/sys/dev/gve/gve_tx_dqo.c": (
        "\t\treturn (EINVAL);\n\t}\n\n\tPULLUP_HDR(mbuf, l4_off + sizeof(struct tcphdr *));",
        "\t\t    htons(IPPROTO_TCP));\n\t}\n\n\tPULLUP_HDR(mbuf, l4_off",
        "gve_prep_tso: csum is set on the IPv4 and IPv6 arms only, and "
        "a frame that is neither wrote two bytes of stack into the "
        "outgoing header as a TCP checksum",
    ),
    "hbsd/src/sys/dev/mlx4/mlx4_core/mlx4_main.c": (
        "\t\t\t*idx = MLX4_SINK_COUNTER_INDEX(dev);",
        "\t\t\t*idx = get_param_l(&out_param);\n\n\t\treturn err;",
        "mlx4_counter_alloc: the mfunc path returned -ENOSPC without "
        "writing *idx, which __mlx4_counter_alloc() does and which "
        "mlx4_allocate_default_counters() relies on",
    ),

    "hbsd/src/sys/dev/mpr/mpr_config.c": [
        (
            ("\tif (error || (cm == NULL) || (reply == NULL)) {", 24),
            "\tif (error || (reply == NULL)) {",
            "mpr_config_*: mpr_wait_command() writes *cmp = NULL when it "
            "reclaims a command, and `reply' is a function-scope local -- "
            "so on the second command of a pair the NULL test passes on "
            "the first command's reply and the bcopy dereferences the "
            "NULL cm; on the first, reply itself is uninitialised",
        ),
        (
            ("\tif (cm != NULL)\n\t\treply = (MPI2_CONFIG_REPLY *)cm->cm_reply;", 24),
            "\terror = mpr_wait_command(sc, &cm, 60, CAN_SLEEP);\n\treply = (MPI2_CONFIG_REPLY *)cm->cm_reply;\n",
            "two of the twenty-four sites read cm->cm_reply with no "
            "cm != NULL guard at all",
        ),
    ],
    "hbsd/src/sys/dev/mps/mps_config.c": (
        ("\tif (error || (cm == NULL) || (reply == NULL)) {", 18),
        "\tif (error || (reply == NULL)) {",
        "mps_config_*: the same stale-reply guard as mpr_config.c, "
        "eighteen times",
    ),
    "hbsd/src/sys/dev/aacraid/aacraid.c": (
        "\tif (sc->aac_max_msix == 0)\n\t\tsc->aac_max_msix = 1;",
        "\t}\n\tsc->aac_vector_cap = sc->aac_max_fibs / sc->aac_max_msix;",
        "aac_define_int_mode: the legacy arm floors aac_max_msix at 1 "
        "and the MSI-X arm only clamps it down, so firmware reporting "
        "zero vectors reaches the division with a zero divisor",
    ),

    "hbsd/src/sys/dev/gve/gve_tx.c": (
        "\tl4_off = 0;\n\tcsum_offset = 0;",
        "\tl3_off = ETHER_HDR_LEN;\n\tmbuf_next = m_getptr(mbuf, l3_off, &offset);\n\n\tif (is_ipv6) {",
        "gve_xmit: l4_off is written only by the IPv6 and IPv4 arms and "
        "csum_offset only under has_csum_flag, and both go to "
        "gve_tx_fill_pkt_desc() unconditionally",
    ),
    "hbsd/src/sys/dev/ufshci/ufshci_dev.c": [
        (
        ("\tparam.desc_size = 0;", 5),
        "\tparam.value = 0;\n\n\tstatus.done = 0;",
        "struct ufshci_query_param is passed BY VALUE to "
        "ufshci_ctrlr_cmd_send_query_request(), which does "
        "`upiu->length = param.desc_size' -- and only the descriptor "
        "reader set it, so every flag and attribute query carried an "
        "uninitialised local in the UPIU length field",
        ),
        (
            "\tuint32_t alloc_units = 0;",
            "\tuint32_t extended_ufs_feature_support;\n\tuint32_t alloc_units;",
            "ufshci_dev_config_write_booster: on the LU-dedicated path "
            "alloc_units is written only by an iteration whose "
            "descriptor read succeeded, and the `alloc_units == 0' test "
            "after the loop read it either way",
        ),
    ],
    "hbsd/src/sys/dev/usb/net/if_usie.c": (
        "\t\t\tgoto done;\n\t\t}",
        "\t\t\tDPRINTF(\"unsupported ether type\\n\");\n\t\t\terr++;\n\t\t\tbreak;",
        "usie_if_rx_callback: the unsupported-ether-type arm broke out "
        "of the SWITCH and fell into netisr_dispatch(ipv, ...) with ipv "
        "undefined, so the device chose a netisr protocol index out of "
        "the stack",
    ),

    "hbsd/src/sys/dev/fdc/fdc.c": [
        (
        ("if (fdc_sense_int(fdc, &st0, &cyl) != 0)", 4),
        "if (fdc_sense_int(fdc, &st0, &cyl) == FD_NOT_VALID)",
        "fdc_sense_int() has three failure returns and only one is "
        "FD_NOT_VALID; it writes *st0p after the first command and "
        "*cylp only after the second, and the `st0 & 0xc0 || cyl != ...' "
        "at all four sites read them either way",
        ),
        (
            "\tif (fd != NULL && bp != NULL && (fd->flags & FD_ISADMA)) {",
            "\tif (fd != NULL && (fd->flags & FD_ISADMA)) {",
            "fdc_worker: the ISADMA block read bp->bio_cmd for the DMA "
            "direction while testing only fd, and the retry check twelve "
            "lines above it tests bp because fdc->bp is NULL whenever no "
            "bio is queued",
        ),
    ],
    "hbsd/src/sys/dev/e1000/if_em.c": (
        "\t\tbytes = bytes_per_packet = packets = 0;",
        "\t\tbytes = bytes_per_packet = 0;",
        "em_newitr: packets was written only inside the two `if "
        "(txpackets != 0)' / `if (rxpackets != 0)' blocks, and the early "
        "return only covers both BYTE counters being zero",
    ),
    "hbsd/src/sys/dev/atkbdc/psm.c": (
        "\t\tif (get_mouse_status(sc->kbdc, stat, 0, 3) == 3 &&",
        "\t\tget_mouse_status(sc->kbdc, stat, 0, 3);\n\t\tif ((SYNAPTICS_VERSION_GE",
        "doopen: the byte count was dropped at this call and checked at "
        "the other one in the same function, so a device answering with "
        "fewer than three bytes left stat[1] and stat[2] uninitialised "
        "for the test",
    ),

    "hbsd/src/sys/dev/gpio/gpioc.c": (
        "\t\tfree(priv_link, M_GPIOC);\n\t\tmtx_unlock(&priv->mtx);\n\t\treturn (ENOMEM);",
        "\tif (pin_link == NULL) {\n\t\tmtx_unlock(&priv->mtx);",
        "gpioc_attach_priv_pin: a failed second allocation returned "
        "ENOMEM without freeing the first, which nothing has taken yet",
    ),
    "hbsd/src/sys/dev/ntb/ntb_transport.c": (
        "\t\t\tfree(nc, M_DEVBUF);\n\t\t\tbreak;",
        "device_printf(dev, \"Can not add child.\\n\");\n\t\t\tbreak;",
        "ntb_transport_attach: a failed device_add_child() broke out of "
        "the loop without freeing nc, whose ivars are set on the next "
        "line on the path where there is a child",
    ),
    "hbsd/src/sys/dev/sound/sndstat.c": (
        "\t\t\tfree(ud, M_DEVBUF);\n\t\t\tsx_unlock(&pf->lock);",
        "\t\tif (err) {\n\t\t\tsx_unlock(&pf->lock);\n\t\t\tgoto done;",
        "sndstat_add_user_devs: a failed unpack left the userdev "
        "allocation neither on the list nor freed, on an ioctl a user "
        "drives with a malformed nvlist",
    ),
    "hbsd/src/sys/dev/nvmf/nvmf_transport.c": (
        "\tstruct nvmf_qpair *qp = NULL;",
        "\tstruct nvmf_transport *nt;\n\tstruct nvmf_qpair *qp;",
        "nvmf_allocate_qpair: qp is written only by an iteration of the "
        "SLIST_FOREACH, and nvmf_supported_trtype() only says the type "
        "is in range -- not that a transport registered for it",
    ),
    "hbsd/src/sys/dev/bhnd/cores/chipc/pwrctl/bhnd_pwrctl_subr.c": [
        (
            "\t\t\tif (div == 0) {",
            "\t\tcase CHIPC_MC_M1:\t\n\t\t\treturn (clock / m1);",
            "bhnd_pwrctl_clock_rate: bhnd_pwrctl_factor6() returns 0 for "
            "every encoding outside its six-case table, and four of the "
            "five arms divided by it or by a product containing it",
        ),
        (
            "\tif (slowminfreq == 0)\n\t\treturn (fpdelay);",
            "\tslowminfreq = bhnd_pwrctl_slowclk_freq(sc, false);\n\n\tpll_on_delay",
            "bhnd_pwrctl_fast_pwrup_delay: bhnd_pwrctl_slowclk_freq() "
            "returns 0 on two paths it reports with a device_printf(), "
            "and that was the divisor",
        ),
    ],

    "hbsd/src/sys/dev/sfxge/common/siena_phy.c": (
        "\t\tif ((_stat) != NULL && (_esmp) != NULL &&\t\t\\",
        "\t\tif ((_stat) != NULL && !EFSYS_MEM_IS_NULL(_esmp)) {\t\\",
        "SIENA_SIMPLE_STAT_SET: EFSYS_MEM_IS_NULL() is "
        "((_esmp)->esm_base == NULL), so it dereferences the pointer it "
        "is asked about -- and the hand-written check in the same "
        "function tests esmp != NULL as well",
    ),
    "hbsd/src/sys/dev/sk/if_sk.c": [
        (
            ("&& ifp0 != NULL", 3),
            "\t\tif (status & SK_ISR_RX1_EOF) {",
            "sk_intr: six per-port arms took sk_if[] without the NULL test "
            "the top of the same function makes, and that the "
            "SK_ISR_EXTERNAL_REG block and the two if_sendq_empty() calls "
            "at the end also make",
        ),
        (
            "pkt_csum_data = m->m_pkthdr.csum_data;",
            "((offset + m->m_pkthdr.csum_data) & 0xffff)",
            "sk_txcksum() walks m forward and its sendit: label then read "
            "m->m_pkthdr.csum_data -- two of the three `goto sendit' sites "
            "establish m is NULL, and m_pkthdr lives on the head of the chain "
            "that m after the walk no longer is",
        ),
    ],

    "hbsd/src/sys/dev/pms/RefTisa/sat/src/smsatcb.c": [
        (
            ("    smIORequest  = smOrgIORequest;", 5),
            "    smIORequest  = smOrgIORequestBody->smIORequest;",
            "five SetFeatures/IDStart callbacks read smOrgIORequestBody "
            "outside the else arm that assigns it -- it is declared "
            "`= agNULL', so on the satIntIo == agNULL arm the line was "
            "agNULL->smIORequest, and it also overwrote the smIORequest "
            "that arm had just computed correctly",
        ),
        (
            ("agFirstDword != agNULL)", 2),
            "if (agIOInfoLen != 0 && agIOStatus == OSSA_IO_SUCCESS)\n"
            "    {\n"
            "      statDevToHostFisHeader =",
            "smsatSetFeaturesAACB() and "
            "smsatSetFeaturesVolatileWriteCacheCB() warn that agFirstDword "
            "may be agNULL, but pair that only with agIOStatus != "
            "OSSA_IO_SUCCESS -- and then read the frame under a guard that "
            "requires SUCCESS. smsatPassthroughCB() in the same file tests "
            "agFirstDword itself on its success path",
        ),
        (
            "smsatDecrementPendingIO(smRoot, smAllShared, satIOContext);\n"
            "      smsatFreeIntIoResource(smRoot, oneDeviceData, satIntIo);\n"
            "      return;\n"
            "    }\n"
            "    else\n"
            "    {\n"
            "      SM_DBG5((\"smsatIDStartCB: satOrgIOContext is NOT NULL",
            "SM_DBG5((\"smsatIDStartCB: satOrgIOContext is NULL\\n\"));\n"
            "    }\n",
            "smsatIDStartCB()'s satOrgIOContext == agNULL arm printed and "
            "fell through, leaving smOrgIORequestBody at its agNULL "
            "initialiser for the dereference below; the nested "
            "smOrgIORequestBody == agNULL arm three lines on already "
            "unwinds exactly this way",
        ),
    ],

    "hbsd/src/sys/dev/ath/ath_hal/ar5212/ar5212_ani.c": (
        "\tif (aniState == AH_NULL) {\n\t\tswitch (cmd) {",
        "\tOS_MARK(ah, AH_MARK_ANI_CONTROL, cmd);\n\n\tswitch (cmd) {",
        "ar5212AniControl: the comment says the function may be called "
        "before there is a current channel, which is when ah_curani and "
        "so params are AH_NULL -- and five of its commands dereference "
        "one or the other",
    ),
    "hbsd/src/sys/dev/ath/ath_hal/ar5416/ar5416_ani.c": (
        "\tif (aniState == AH_NULL) {\n\t\tswitch (cmd) {",
        "\tOS_MARK(ah, AH_MARK_ANI_CONTROL, cmd);\n\n\t/* These commands can't be disabled */",
        "ar5416AniControl: the same five commands and the same AH_NULL "
        "ANI state as ar5212_ani.c",
    ),

    "hbsd/src/sys/dev/msk/if_msk.c": (
        ("&& sc->msk_if[MSK_PORT_A] != NULL)", 1),
        "\tif (rxput[MSK_PORT_A] > 0)\n\t\tmsk_rxput(sc->msk_if[MSK_PORT_A]);",
        "msk_intr_task: the two msk_rxput() calls took msk_if[] without "
        "the NULL test msk_intr_hwerr()'s two arms and the two "
        "msk_txeof() calls twenty lines up all make",
    ),

    "hbsd/src/sys/dev/ath/ath_hal/ar5212/ar2413.c": (
        "for (tp = lp; tp + 1 < ep; tp++) {",
        "for (tp = lp; tp < ep; tp++) {",
        "GetLowerUpperIndex: the loop reads tp[1] one past the list on "
        "its last iteration, and falls out leaving both outputs unwritten "
        "if the list is not sorted ascending",
    ),

    "hbsd/src/sys/dev/ath/ath_hal/ar5212/ar2425.c": (
        "for (tp = lp; tp + 1 < ep; tp++) {",
        "for (tp = lp; tp < ep; tp++) {",
        "GetLowerUpperIndex: same one-past read and unwritten outputs "
        "as ar2413.c",
    ),

    "hbsd/src/sys/dev/ath/ath_hal/ar5212/ar5413.c": (
        "for (tp = lp; tp + 1 < ep; tp++) {",
        "for (tp = lp; tp < ep; tp++) {",
        "GetLowerUpperIndex: same one-past read and unwritten outputs "
        "as ar2413.c",
    ),

    "hbsd/src/sys/dev/ath/ath_hal/ar5212/ar2316.c": (
        "for (tp = lp; tp + 1 < ep; tp++) {",
        "for (tp = lp; tp < ep; tp++) {",
        "GetLowerUpperIndex: same one-past read and unwritten outputs "
        "as ar2413.c",
    ),

    "hbsd/src/sys/dev/ath/ath_hal/ar5212/ar2317.c": (
        "for (tp = lp; tp + 1 < ep; tp++) {",
        "for (tp = lp; tp < ep; tp++) {",
        "GetLowerUpperIndex: same one-past read and unwritten outputs "
        "as ar2413.c",
    ),

    "hbsd/src/sys/dev/ath/ath_hal/ar5212/ar5112.c": (
        "for (tp = lp; tp + 1 < ep; tp++) {",
        "for (tp = lp; tp < ep; tp++) {",
        "ar5212GetLowerUpperIndex: same one-past read and unwritten "
        "outputs as ar2413.c's GetLowerUpperIndex",
    ),

    "hbsd/src/sys/dev/ath/ath_hal/ar5212/ar5212_reset.c": (
        "for (; lp + 1 < ep; lp++) {",
        "for (; lp < ep; lp++) {",
        "ar5212GetLowerUpperValues: the loop reads lp[1] one past the "
        "list, and HALASSERT(AH_FALSE) is a no-op in a production kernel, "
        "so falling out left both outputs unwritten",
    ),

    "hbsd/src/sys/dev/ath/ath_hal/ar5211/ar5211_reset.c": (
        "for (i = 0; i < listSize - 1; i++) {",
        "for (i = 0; i < listSize; i++) {",
        "ar5211GetLowerUpperValues: the index form of the same one-past "
        "read, with no assert at all on the fall-through",
    ),

    "hbsd/src/sys/dev/gve/gve_qpl.c": (
        ("if (err == 0)\n\t\t\t\terr = rc;", 2),
        "\terr = gve_adminq_unregister_page_list(priv, com->qpl->id);",
        "gve_unregister_qpls: err was read after both loops without "
        "being written when the queue counts are zero, and each "
        "iteration overwrote the previous failure",
    ),

    "hbsd/src/sys/dev/gve/gve_adminq.c": (
        ("if (err == 0)\n\t\t\t\terr = rc;", 2),
        "\terr = gve_adminq_destroy_rx_queue(priv, i);",
        "gve_adminq_destroy_{rx,tx}_queues: the same unwritten err and "
        "swallowed failure, twice",
    ),

    "hbsd/src/sys/dev/usb/storage/cfumass.c": (
        "while (sg_count > 0 && sumlen >= sglist->len) {",
        "while (sumlen >= sglist->len && sg_count > 0) {",
        "cfumass_t_data_callback: sglist->len was evaluated before the "
        "sg_count test, so the iteration that empties the list reads "
        "one entry past its end",
    ),

    "hbsd/src/sys/dev/iicbus/rtc/nxprtc.c": (
        "uint8_t sec = 0, tmr1 = 0, tmr2 = 0;",
        "uint8_t sec, tmr1, tmr2;",
        "read_timeregs: a read_reg() failure breaks out of the loop "
        "with tmr1 unwritten, and the use_timer test after it does not "
        "short-circuit that read",
    ),

    "hbsd/src/sys/dev/cfi/cfi_core.c": (
        "if (sc->sc_width != 1 && sc->sc_width != 2 && sc->sc_width != 4) {",
        "if (sc->sc_width > 4) {",
        "cfi_attach: every switch on sc_width has cases 1, 2 and 4 and "
        "no default, and a device hint could set 3",
    ),

    "hbsd/src/sys/dev/flash/cqspi.c": (
        "ret = cqspi_cmd_read(sc, CMD_READ_STATUS, &data, 1);",
        "\t\tcqspi_cmd_read(sc, CMD_READ_STATUS, &data, 1);\n\t} while",
        "cqspi_wait_ready: the status read's failure path leaves data "
        "unwritten, and the loop spun on it",
    ),

    "hbsd/src/sys/dev/bhnd/nvram/bhnd_nvram_store_subr.c": (
        "\tpath->path_str = NULL;\n\n\tpath->pending = bhnd_nvram_plist_new();",
        "\tpath->num_vars = 0;\n\n\tpath->pending = bhnd_nvram_plist_new();",
        "bhnd_nvstore_path_new: the first goto failed is above the only "
        "assignment to path_str, and the label free()s it",
    ),

    "hbsd/src/sys/dev/psci/psci.c": (
        "phandle_t node = 0;",
        "\tphandle_t node;\n\n\t/* XXX: This is suboptimal",
        "psci_fdt_callfn: node is read after a loop that does not run "
        "on an empty compat_data",
    ),

    "hbsd/src/sys/dev/vt/vt_core.c": (
        "int\t\t grabbed = 0, i, idx0, idx;",
        "int\t\t grabbed, i, idx0, idx;",
        "vt_allocate_keyboard: grabbed is assigned and read under two "
        "separate tests of vd_curwindow, with kbd_allocate() between",
    ),

    "hbsd/src/sys/dev/sound/pcm/dsp.c": (
        "struct snddev_info *d = NULL;\n\tuint32_t fmts;\n\tint i, minch, maxch, unit;",
        "struct snddev_info *d;\n\tuint32_t fmts;\n\tint i, minch, maxch, unit;",
        "dsp_oss_audioinfo: d is tested against NULL after a loop that "
        "does not run when pcm_devclass has no units",
    ),

    "hbsd/src/sys/dev/xl/if_xl.c": (
        "if (xl_read_eeprom(sc, (char *)&xcvr, XL_EE_ICFG_0, 2, 0)) {",
        "\txl_read_eeprom(sc, (char *)&xcvr, XL_EE_ICFG_0, 2, 0);\n",
        "xl_attach: the transceiver read ignored the EEPROM failure the "
        "station-address read twenty lines up already checks",
    ),

    "hbsd/src/sys/dev/mpi3mr/mpi3mr_app.c": (
        "if (data_out_sz != sizeof(pel_enable)) {",
        "(pel_enable.pel_class > MPI3_PEL_CLASS_FAULT))) {",
        "mpi3mr_pel_enable: the pre-copyin test read pel_enable.pel_class "
        "off the stack, and repeats a check made correctly after it",
    ),

    "hbsd/src/sys/dev/syscons/syscons.c": (
        "if (kbdd_ioctl(sc->kbd, KDGKBSTATE,\n\t\t\t\t    (caddr_t)&f) != 0)",
        "(void)kbdd_ioctl(\n\t\t\t\t    sc->kbd, KDGKBSTATE, (caddr_t)&f);",
        "scgetc: the scroll-lock arm read f whether or not KDGKBSTATE "
        "wrote it, unlike save_kbd_state() and update_kbd_state()",
    ),

    "hbsd/src/sys/dev/pms/RefTisa/tisa/sassata/common/tdioctl.c": (
        "status = IOCTL_CALL_FAIL;\n#endif",
        "resetType);\n#endif",
        "tdsaSendTMFIoctl: the only assignment to status is inside a "
        "#if that excludes FreeBSD, so the ioctl returned the stack",
    ),

    "hbsd/src/sys/dev/bxe/bxe_elink.c": (
        "for (phy_index = ELINK_INT_PHY; phy_index < ELINK_MAX_PHYS;",
        "for (phy_index = ELINK_INT_PHY; phy_index < params->num_phys;\n\t      phy_index++) {\n\t\tphy_vars[phy_index].flow_ctrl = 0;",
        "elink_link_update: the link_up expression reads "
        "phy_vars[ELINK_INT_PHY] even when num_phys is zero and the "
        "init loop never wrote it",
    ),

    "hbsd/src/sys/dev/qlnx/qlnxe/ecore_mcp.c": (
        "rc = ecore_mcp_get_transceiver_data(p_hwfn, p_ptt, &transceiver_data);",
        "\tecore_mcp_get_transceiver_data(p_hwfn, p_ptt, &transceiver_data);\n",
        "ecore_mcp_trans_speed_mask: two of the callee's four exits do "
        "not write the transceiver word, and the call ignored the return",
    ),

    "hbsd/src/sys/dev/etherswitch/rtl8366/rtl8366rb.c": [
        (
            ("vg->es_vlangroup >= RTL8366_NUM_VLANS)", 2),
            None,
            "rtl_{get,set}vgroup: es_vlangroup came off the ioctl and "
            "indexed sc->vid[RTL8366_NUM_VLANS] -- and the VMCR bank -- "
            "with no bound check on either side",
        ),
        (
            "err = smi_read(dev, RTL8366_PLSR_BASE + (RTL8366_NUM_PHYS)/2,",
            "\t\tsmi_read(dev, RTL8366_PLSR_BASE + (RTL8366_NUM_PHYS)/2, &v, RTL_WAITOK);",
            "rtl_getport: smi_read() returns EBUSY without writing v, and "
            "the CPU-port arm ignored that before shifting it",
        ),
    ],

    "hbsd/src/sys/dev/etherswitch/ip17x/ip17x_vlans.c": (
        ("vg->es_vlangroup >= IP17X_MAX_VLANS)", 2),
        None,
        "ip17x_{get,set}vgroup: es_vlangroup indexed "
        "sc->vlan[IP17X_MAX_VLANS] with no bound check on either side",
    ),

    "hbsd/src/sys/dev/etherswitch/felix/felix.c": (
        ("vg->es_vlangroup >= sc->info.es_nvlangroups)", 2),
        None,
        "felix_{get,set}_dot1q_vlan: es_vlangroup indexed "
        "sc->vlans[FELIX_NUM_VLANS] with no bound check on either side",
    ),

    "hbsd/src/sys/dev/etherswitch/arswitch/arswitch_vlans.c": (
        ("vg->es_vlangroup >= sc->info.es_nvlangroups)", 2),
        "if (vg->es_vlangroup > sc->info.es_nvlangroups)",
        "ar8xxx_getvgroup's > test admitted es_nvlangroups itself and "
        "every negative index; ar8xxx_setvgroup, which writes "
        "sc->vid[], had no test at all",
    ),

    "hbsd/src/sys/dev/etherswitch/ar40xx/ar40xx_main.c": (
        ("vg->es_vlangroup >= sc->sc_info.es_nvlangroups)", 2),
        "if (vg->es_vlangroup > sc->sc_info.es_nvlangroups)",
        "ar40xx_getvgroup's > test was off by one and missed negatives; "
        "ar40xx_setvgroup, which writes vlan_id[], vlan_ports[] and "
        "vlan_untagged[], had no test at all",
    ),

    "hbsd/src/sys/dev/etherswitch/e6000sw/e6000sw.c": (
        ("if (port >= sc->num_ports)", 2),
        "if (port > sc->num_ports)",
        "e6000sw_{get,set}_port_vlan: the > test admitted num_ports "
        "itself",
    ),

    "hbsd/src/sys/dev/etherswitch/mtkswitch/mtkswitch_mt7620.c": (
        ("(v->es_vlangroup >= sc->info.es_nvlangroups))", 2),
        "(v->es_vlangroup > sc->info.es_nvlangroups))",
        "mtkswitch_vlan_{get,set}vgroup: the > test admitted "
        "es_nvlangroups itself and every negative register index",
    ),

    "hbsd/src/sys/dev/etherswitch/mtkswitch/mtkswitch_rt3050.c": (
        ("(v->es_vlangroup >= sc->info.es_nvlangroups))", 2),
        "(v->es_vlangroup > sc->info.es_nvlangroups))",
        "mtkswitch_vlan_{get,set}vgroup: the same off-by-one as "
        "mtkswitch_mt7620.c",
    ),

    "hbsd/src/sys/fs/fuse/fuse_internal.c": [
        (
            "err = fuse_internal_getattr(vp, &va, cred, td);\n\t\tif (err != 0)",
            "\t\tfuse_internal_getattr(vp, &va, cred, td);\n\t\treturn vaccess(",
            "fuse_internal_access: under FSESS_DEFAULT_PERMISSIONS the "
            "whole permission decision was made from a struct vattr a "
            "failed getattr never wrote",
        ),
        (
            "int gaerr = fuse_internal_getattr(vp, &va, cred, td);",
            "\t\t\tfuse_internal_getattr(vp, &va, cred, td);\n\t\t\tif (va.va_mode &",
            "fuse_internal_clear_suid_on_write: whether a setuid bit "
            "survives the write was decided by a stack byte on a failed "
            "getattr",
        ),
    ],

    "hbsd/src/sys/fs/fuse/fuse_vnops.c": (
        "access_e = fuse_internal_getattr(vp, &va, cred, td);",
        "\t\t\tfuse_internal_getattr(vp, &va, cred, td);\n\t\t\taccess_e = vaccess(",
        "fuse_close: the close-path atime update's VWRITE check ran "
        "against a struct vattr a failed getattr never wrote",
    ),

    "hbsd/src/sys/fs/tarfs/tarfs_io.c": (
        "error = -res;\n\t\tgoto bad;",
        "return (-res);",
        "tarfs_io_init: the read-failure path returned without freeing "
        "the iosize-sized block the bad: label frees on every other exit",
    ),

    "hbsd/src/sys/x86/x86/ucode_subr.c": (
        "const amd_10h_fw_header_t *selected_fw = NULL;",
        "\tconst amd_10h_fw_header_t *selected_fw;\n",
        "ucode_amd_find: selected_fw is tested before its first "
        "assignment and returned unset, so a malformed AMD microcode "
        "container handed the caller a stack pointer to apply",
    ),

    "hbsd/src/sys/compat/freebsd32/freebsd32_misc.c": (
        "\t\tif (error != 0)\n\t\t\tbreak;\n\t\tCP(r32.pc, r.pc, pc_fd);",
        "\t\t\terror = copyin(uap->addr, &r32.pc, uap->data);\n\t\tCP(r32.pc, r.pc, pc_fd);",
        "freebsd32_ptrace PT_COREDUMP: the CP macros ran over r32.pc "
        "whether or not the copyin filled it, unlike PT_VM_ENTRY above "
        "and PT_SC_REMOTE below",
    ),

    "hbsd/src/sys/arm64/rockchip/rk_pinctrl.c": (
        "\t\tpulldown = 2;\n\t\tbreak;\n\tdefault:",
        "\t\tpulldown = 2;\n\t\tbreak;\n\t}\n",
        "rk3399_parse_bias: a device-tree bank the switch has no case "
        "for left both locals unwritten and returned one of them",
    ),

    "hbsd/src/sys/netpfil/ipfilter/netinet/ip_sync.c": (
        "sl->sl_rev = sp->sm_rev;",
        "n->nat_sync = sl;\n\t\tn->nat_rev = sl->sl_rev;",
        "ipf_sync_nat SMC_CREATE: sl comes from KMALLOC() and this arm "
        "never fills sl_hdr, so sl_rev -- which IS sl_hdr.sm_rev -- was "
        "the allocator's leftovers",
    ),

    "hbsd/src/sys/netpfil/pf/pf_lb.c": (
        "\t\tidx = pd->didx;\n\t\tbreak;\n\tdefault:",
        "\t\tidx = pd->didx;\n\t\tbreak;\n\t}\n\tnaddr =",
        "pf_get_transaddr: the switch that picks the state-key index is "
        "now total -- defence in depth; the three-file chain that makes "
        "the default unreachable is in the comment",
    ),

    "hbsd/src/lib/libc/net/sourcefilter.c": (
        "if (tmpslist != NULL) {\n\t\tif (*numsrc != 0) {",
        "if (tmpslist != NULL && *numsrc != 0) {",
        "getipv4sourcefilter: the free() was inside the *numsrc != 0 "
        "test, so a group with no sources leaked the array -- "
        "setipv4sourcefilter() fifty lines up frees unconditionally",
    ),

    "hbsd/src/lib/libc/db/recno/rec_put.c": (
        "if (F_ISSET(t, R_FIXLEN))\n\t\t\t\t\t\tfree(tdata.data);\n\t\t\t\t\treturn (RET_ERROR);",
        "!= RET_SUCCESS)\n\t\t\t\t\treturn (RET_ERROR);",
        "__rec_put: a failure part-way through filling a record gap "
        "returned without freeing the bt_reclen pad record the success "
        "path frees",
    ),

    "hbsd/src/sys/dev/clk/xilinx/zynqmp_clock.c": [
        (
            "free(prev_clock_name, M_DEVBUF);\n\t\t\tprev_clock_name = strdup(clkname, M_DEVBUF);",
            "if (clkname != NULL)\n\t\t\tprev_clock_name = strdup(clkname, M_DEVBUF);",
            "zynqmp_clk_register: the loop overwrote the previous "
            "iteration's strdup()ed clock name without freeing it",
        ),
        (
            "if (prev_clock_name == NULL)\n\t\treturn (1);",
            None,
            "zynqmp_clk_register: strdup(prev_clock_name) faulted when "
            "the topology loop registered no node",
        ),
        (
            "free(__DECONST(char *, clkdef->clkdef.parent_names[0]),",
            "if (clknode == NULL)\n\t\treturn (1);",
            "zynqmp_clk_register: the clknode_create() failure return "
            "left the parent-name array and its string behind, and the "
            "loop's last clock name had no owner after the copy",
        ),
        (
            "if (zynqmp_fw_clk_get_name(sc, clk, i) != 0) {",
            "\t\tzynqmp_fw_clk_get_name(sc, clk, i);\n\t\tzynqmp_fw_clk_get_attributes",
            "zynqmp_fw_clk_get_all: an ignored get_name() failure left "
            "clkdef.name NULL for the strcmp() in the registration loop "
            "to dereference",
        ),
    ],

    "hbsd/src/sys/dev/mlx/mlx.c": (
        "if (result != NULL)\n\t    free(result, M_DEVBUF);",
        "if ((result != NULL) && (mc->mc_data != NULL))",
        "mlx_periodic_eventlog_poll: the free() was gated on mc->mc_data, "
        "which is set only after mlx_getslot() succeeds -- and read mc "
        "after mlx_releasecmd(), on a path where mc can be NULL",
    ),

    "hbsd/src/sbin/ipfw/nat.c": [
        (
            'if (sscanf (str, "%hu-%hu", &loPort, &hiPort) != 2)',
            'sscanf (str, "%hu-%hu", &loPort, &hiPort);\n\tSETLOPORT',
            "StrToPortRange: the sscanf() return was ignored, so a port "
            "range the shell handed it that is not two numbers around a '-' "
            "installed a NAT redirect over stack contents",
        ),
        (
            "\t*portRange = 0;",
            "\tu_short\t hiPort;\n\n\t/* First see if this is a service",
            "StrToPortRange: SETLOPORT and SETNUMPORTS each preserve the "
            "half of *portRange they do not write, so the first of the two "
            "reads the caller's uninitialised port_range",
        ),
    ],

    "hbsd/src/sbin/natd/natd.c": [
        (
            'if (sscanf (str, "%hu-%hu", &loPort, &hiPort) != 2)',
            'sscanf (str, "%hu-%hu", &loPort, &hiPort);\n\tSETLOPORT',
            "StrToPortRange: natd's copy of the same ignored sscanf()",
        ),
        (
            "\t*portRange = 0;",
            "u_short         hiPort;\n\t\n\t/* First see if this is a service",
            "StrToPortRange: natd's copy of the same indeterminate read of "
            "*portRange",
        ),
    ],

    "hbsd/src/sbin/ipf/ipsend/resend.c": (
        "\t\t\tpkt = mb.mb_buf;",
        "\t\t\teh = (ether_header_t *)mb.mb_buf;\n\t\t\tlen = i;",
        "ip_resend() assigned mb.mb_buf -- a member of the mb_t on its "
        "own stack -- over eh, which is the malloc()ed header free(eh) "
        "at the bottom releases, so with -R that free() was handed a "
        "stack address every time",
    ),

    "hbsd/src/usr.sbin/ndp/ndp_netlink.c": (
        'ifname = link->ifla_ifname != NULL ? link->ifla_ifname : "?";',
        "\tifname = link->ifla_ifname;\n\tifwidth = strlen(ifname);",
        "print_entry() walks ifla_ifname with strlen(), but IFLA_IFNAME "
        "is listed in _nla_p_link_s with no required flag and both "
        "callers declare snl_parsed_link_simple `= {}', so "
        "snl_parse_nlmsg() returning true says nothing about whether the "
        "name arrived",
    ),

    "hbsd/src/usr.sbin/jail/jail.c": (
        "((jp->jp_ctltype & CTLTYPE) ==\n\t\t\t\t    CTLTYPE_STRING\n"
        "\t\t\t\t    ? strncmp(rtjp->jp_value, jp_value,",
        "(CTLTYPE_STRING ? strncmp(rtjp->jp_value,",
        "rdtun_params() used the bare constant CTLTYPE_STRING as a "
        "ternary condition -- it is 3, so memcmp() was dead and every "
        "read-only jail parameter was compared with strncmp(), which "
        "stops at the first NUL: 10.0.0.1 and 10.0.5.9 are 0a 00 00 01 "
        "and 0a 00 05 09 and compare equal",
    ),

    "hbsd/src/usr.bin/chat/chat.c": [
        (
            "void fatal(int code, const char *fmt, ...) __dead2;",
            "void fatal(int code, const char *fmt, ...);",
            "dup_mem() calls fatal() on a failed malloc and then memcpy()s "
            "through the pointer; fatal() ends in terminate()",
        ),
        (
            "void terminate(int status) __dead2;",
            "void terminate(int status);",
            "and terminate() ends in exit()",
        ),
        (
            "static char buf [STR_LEN];",
            "\n    char buf [STR_LEN];",
            "do_file(): chat_expect() and chat_send() tokenise through "
            "expect_strtok(), which keeps a `static char *str' cursor "
            "into whatever string it was last handed -- and what it is "
            "handed here is a pointer into this buffer, so on the stack "
            "that cursor dangles into a dead frame the moment do_file() "
            "returns",
        ),
    ],

    "hbsd/src/usr.sbin/gstat/gstat.c": (
        "int head_printed = 0;",
        "max_flen, head_printed;",
        "head_printed was written only by the `-C' arm of getopt and "
        "read only under `flag_C && !head_printed' -- the same "
        "condition, so nothing goes wrong today, but the relation is "
        "stated nowhere and costs one initialiser to remove",
    ),

    "hbsd/src/usr.sbin/virtual_oss/virtual_oss/main.c": [
        (
            "if (mod == 0 || mod_internal == 0)\n\t\treturn (CUSE_ERR_INVALID);",
            None,
            "vclient_setup_buffers(): the sanity checks a dozen lines "
            "down reject both a zero format and a zero channel count, "
            "and they run AFTER the `size % mod' that needs them.  "
            "vclient_sample_bytes() returns 0 for a format naming no "
            "bit width",
        ),
        (
            ("temp = pvc->channels * vclient_sample_bytes(pvc);\n\t\tif "
             "(temp == 0) {", 2),
            "vclient_input_delay(pvc) / (pvc->channels * "
            "vclient_sample_bytes(pvc));",
            "SNDCTL_DSP_CURRENT_IPTR and _OPTR divide by that same "
            "product without checking it, resting on an invariant "
            "vclient_setup_buffers() establishes two functions away",
        ),
    ],

    "hbsd/src/sbin/fsck/fsck.c": (
        "\tvfstype = estrdup(pvfstype);",
        '\tvfstype = strdup(pvfstype);\n\tif (vfstype == NULL)\n'
        '\t\tperr("strdup(pvfstype)");',
        "checkfs() called perr() on a failed strdup and then strlen() on "
        "the result -- but perr() only exits when preen is set; vmsg() "
        "prints and returns otherwise, which is the contract devcheck() "
        "relies on when it returns origname after each of its three "
        "perr() calls",
    ),

    "hbsd/src/usr.sbin/yppush/yppush_main.c": (
        "static void __dead2\nyppush_exit(int now)",
        "static void\nyppush_exit(int now)",
        "yp_push() writes `if ((job = malloc(...)) == NULL) { "
        "yp_error(\"malloc failed\"); yppush_exit(1); }' and then "
        "job->stat = 0 -- yppush_exit() ends in exit() and the definition "
        "did not say so",
    ),

    "hbsd/src/sbin/dump/dump.h": (
        "void\tquit(const char *fmt, ...) __printflike(1, 2) __dead2;",
        "void\tquit(const char *fmt, ...) __printflike(1, 2);",
        "blkread() calls quit() when its scratch buffer cannot be "
        "allocated and then memcpy()s through it -- quit() ends in "
        "dumpabort(), which dump.h already declares __dead2, so the "
        "attribute was missing from exactly one link of the chain",
    ),

    "hbsd/src/sbin/pfctl/pfctl_table.c": (
        "extern void\tusage(void) __dead2;",
        "extern void\tusage(void);",
        "pfctl_table() opens with `if (command == NULL) usage();' and "
        "then strcmp()s command a dozen times",
    ),

    "hbsd/src/sbin/pfctl/pfctl.c": (
        "void\t usage(void) __dead2;",
        "void\t usage(void);",
        "the same declaration in pfctl's own translation unit",
    ),

    "hbsd/src/usr.sbin/lpr/lpc/extern.h": (
        "void\t quit(int _argc, char *_argv[]) __dead2;",
        "void\t quit(int _argc, char *_argv[]);",
        "cmdscanner() writes `if ((bp = el_gets(el, &num)) == NULL || "
        "num == 0) quit(0, NULL);' and then memcpy()s from bp -- quit() "
        "ends in exit() and the declaration did not say so",
    ),

    "hbsd/src/usr.sbin/lpr/common_source/lp.h": (
        "__printflike(2, 3) __dead2;",
        "__printflike(2, 3);\nint\t firstprinter",
        "lpr's fatal() ends in exit() too, and rmremote() calls it on the "
        "failure path and then uses what it was checking",
    ),

    "hbsd/src/usr.sbin/lpr/lpd/lpd.c": (
        ("__dead2;", 2),
        "static void\t mcleanup(int _signo);",
        "mcleanup() and fhosterr(), both ending in exit()",
    ),

    "hbsd/src/usr.sbin/lpr/lpd/recvjob.c": (
        "static void\t frecverr(const char *_msg, ...) __printf0like(1, 2) __dead2;",
        "static void\t frecverr(const char *_msg, ...) __printf0like(1, 2);",
        "frecverr() ends in exit()",
    ),

    "hbsd/src/usr.sbin/lpr/lpd/printjob.c": (
        "static void\t abortpr(int _signo) __dead2;",
        "static void\t abortpr(int _signo);",
        "abortpr() ends in exit()",
    ),

    "hbsd/src/usr.sbin/lpr/lpc/lpc.c": (
        "static void\t\t intr(int _signo) __dead2;",
        "static void\t\t intr(int _signo);",
        "intr() ends in exit()",
    ),

    "hbsd/src/usr.sbin/lpr/lpr/lpr.c": (
        "static void\t cleanup(int _signo) __dead2;",
        "static void\t cleanup(int _signo);",
        "cleanup() ends in exit(), and lprm's main() reached a NULL "
        "dereference past a call to its sibling",
    ),

    "hbsd/src/usr.bin/diff/diffdir.c": (
        "if (ignore_file_case && dp2 != NULL &&\n"
        "\t    strcasecmp(dp->d_name, dp2->d_name) == 0)",
        "if (ignore_file_case && strcasecmp(dp2->d_name, dp2->d_name) == 0)",
        "diffit() compared dp2's name with itself, so under -i the test "
        "was always true: `diff -i -N' over directories that are not "
        "identical dereferenced the NULL diffdir() passes for the missing "
        "side, and when both entries existed with different names path2 "
        "was built from the wrong one",
    ),

    "hbsd/src/usr.bin/tftp/main.c": [
        (
            "if (line == NULL || (size_t)len >= sz) {",
            "if ((size_t)len >= sz)\n\t\t\t\tline = realloc(line, sz = "
            "len + 1);",
            "command() assigned realloc()'s result back over the only "
            "pointer to the old buffer and copied into it unchecked, "
            "updating sz inside the call so it described a buffer that "
            "did not exist",
        ),
        (
            "if (res == NULL || peer < 0)",
            None,
            "setpeer0() tested the getaddrinfo loop's exhaustion through "
            "peer, a file-scope int that starts at zero rather than -1 "
            "and that the addrlen arm of the loop `continue's without "
            "touching -- so running off the end of res0 could take the "
            "else arm with res NULL",
        ),
    ],

    "hbsd/src/usr.sbin/route6d/route6d.c": [
        (
            "static void rtdexit(void) __attribute__((__noreturn__));",
            "static void rtdexit(void);",
            "rtdexit() ends in exit(1) and fatal() ends in rtdexit(), so "
            "every NOTREACHED after a fatal() call in this file is true -- "
            "but neither declaration said so, and the analyser read on "
            "past all seven into the malloc, realloc, sysctl and localtime "
            "results they were reporting",
        ),
        (
            "__attribute__((__format__(__printf__, 1, 2), __noreturn__));",
            "\t__attribute__((__format__(__printf__, 1, 2)));\nstatic void trace",
            "fatal()'s own declaration, the other half of the same",
        ),
    ],

    "hbsd/src/usr.sbin/ppp/main.h": (
        "extern void AbortProgram(int) __attribute__((__noreturn__));",
        "extern void AbortProgram(int);",
        "AbortProgram() ends in exit() and every caller treats it that "
        "way -- the allocation failures in physical2iov(), "
        "udp_iov2device() and their kin call it and then use the pointer "
        "that was NULL",
    ),

    "hbsd/src/usr.sbin/bhyve/pci_ahci.c": (
        "\tif (done >= len) {",
        "/* All remaining ranges were empty. */\n\tif (done == len) {",
        "ahci_handle_next_trim()'s \"all ranges empty\" test was "
        "`done == len' while the loop above it exits on `done < len' "
        "being false, so the test only matches the loop's own exit when "
        "done lands exactly on len -- and elba scales into "
        "breq->br_offset and elen into br_resid, so a pair the loop never "
        "wrote is a discard of a range of the backing store taken from "
        "the stack",
    ),

    "hbsd/src/usr.sbin/rpcbind/rpcb_svc_com.c": (
        "\treply_msg.rm_xid = 0;",
        "\tchar *uaddr;\n#endif\n\n\tbuffer = malloc(RPC_BUF_MAX);",
        "handle_reply()'s done: label reads reply_msg.rm_xid and hands a "
        "non-zero one to free_slot_by_xid(), which tears the forwarding "
        "slot at that index down -- and three paths reach done: before "
        "anything writes rm_xid, one of them xdr_replymsg() rejecting a "
        "datagram a remote host sent",
    ),

    "hbsd/src/usr.bin/tabs/tabs.c": (
        'errx(1, "no tab stops specified");',
        "\t\tlast = stops[(*nstops)++] = stop;\n\t}\n}",
        "`tabs \"\"' and `tabs ,' give strtok() nothing to return, so "
        "gettabs() leaves *nstops at 0 while main tests `nstops >= 0' -- "
        "-1 being \"no list given\" -- and prints stops[0] - 1 as a "
        "%*s field width out of a stack word",
    ),

    "hbsd/src/usr.sbin/vidcontrol/vidcontrol.c": (
        "} while (x > 0 && line[x] == ' ');",
        "} while (line[x] == ' ' && x != 0);",
        "dump_screen()'s trailing-space trim tested line[x] before it "
        "tested x, so a zero shot.xsize -- the console column count out "
        "of a CONS_GETINFO ioctl -- read line[-1] and, if that byte was a "
        "blank, wrote a NUL there and kept walking back",
    ),

    "hbsd/src/usr.sbin/pkg/ecc.c": [
        (
            "\t\tcbdata.key = NULL;\n\t\tcbdata.keylen = 0;",
            'warn("fopen: %s", sigfile);\n\t\t\treturn (false);\n'
            "\t\t}\n\t} else {",
            "ecc_verify_data()'s sigfile arm left key and keylen unwritten "
            "and ecc_verify_internal() passes both to "
            "ecc_extract_pubkey(), which opens with "
            "assert((keyfp != NULL) ^ (key != NULL)) -- so it read the "
            "uninitialised key and aborted whenever the stack word under "
            "it was non-NULL",
        ),
        (
            "memcmp(oidp, oid_ecpubkey, oidsz) != 0)\n\t\tgoto out;",
            "memcmp(oidp, oid_ecpubkey, oidsz) != 0)\n\t\treturn (1);",
            "the one failure arm in ecc_extract_pubkey() that returned "
            "rather than unwinding, leaking root and the libder context "
            "on a key whose algorithm OID is not id-ecPublicKey",
        ),
        (
            "\tkeysz = sizeof(keybuf);\n\tif (ecc_extract_pubkey(",
            "keysz = MIN(sizeof(keybuf), cbdata->keylen / 2);",
            "a dead store that computed MIN(sizeof(keybuf), keylen / 2) "
            "and discarded it on the next line -- and would have "
            "under-reported keybuf's capacity had it survived",
        ),
    ],

    "hbsd/src/usr.sbin/nscd/agents/group.c": (
        "\tmem_size = 0;\n\tif (grp->gr_mem != NULL) {",
        "\tif (grp->gr_mem != NULL) {\n\t\tmem_size = 0;",
        "group_marshal_func() sets mem_size under `grp->gr_mem != NULL' "
        "and reads it under `new_grp.gr_mem != NULL', which is the same "
        "predicate only because new_grp is a memcpy of *grp",
    ),

    "hbsd/src/usr.bin/ul/ul.c": [
        (
            "static wchar_t	*lnbuf;",
            "\twchar_t lbuf[256];",
            "overstrike() and iattr() each wrote maxcol + 1 wchar_t into a "
            "fixed wchar_t lbuf[256] while obuf, which maxcol indexes, "
            "grows by doubling -- so any line past 256 columns carrying a "
            "mode change ran off the end of a 1KB stack buffer",
        ),
        (
            ("\t\tif (cp == lbuf)\n\t\t\tbreak;", 2),
            "\tfor (*cp=' '; *cp==' '; cp--)\n\t\t*cp = 0;",
            "the trailing-blank trim walked back from the sentinel testing "
            "*cp before checking it was still inside lbuf, so a line whose "
            "every column came out blank read and wrote lbuf[-1]",
        ),
    ],

    "hbsd/src/usr.bin/usbhidctl/usbhid.c": [
        (
            ("cp = colls_append(colls, sizeof(colls), cp,", 2),
            "cp += sprintf(&colls[cp],",
            "parceargs() filled colls[1000] with an unbounded sprintf() "
            "from a report descriptor the USB device supplies, advancing "
            "cp by the return, so enough nested collections wrote past a "
            "stack buffer",
        ),
        (
            ("colls[0] = '\\0';", 2),
            "\t\tcp = 0;\n\t\tfor (d = hid_start_parse(r,",
            "colls[] is handed to asprintf() as a %s and tested at "
            "colls[0] for every item, but only a hid_collection item ever "
            "writes it -- a descriptor whose first item is an input, "
            "output or feature item printed uninitialised stack into the "
            "variable's name",
        ),
    ],

    "hbsd/src/usr.sbin/bhyve/pci_virtio_console.c": (
        "int i, ret = 1;",
        "\tint i, ret;\n",
        "pci_vtcon_sock_tx: the loop is the only writer of ret and the "
        "test after it runs regardless, so a zero-descriptor console "
        "buffer let the guest tear the connection down off a stack word",
    ),

    "hbsd/src/sbin/ifconfig/ifpfsync.c": [
        (
            "memset(&syncpeer, 0, sizeof(syncpeer));",
            "\tmemset((char *)&syncdev, 0, IFNAMSIZ);\n\tif (nvlist_exists_string",
            "pfsync_status() reads syncpeer.ss_family unconditionally to "
            "decide whether to call getnameinfo() on &syncpeer with "
            "syncpeer_sa->sa_len as the length, but fills syncpeer only if "
            "the ioctl's nvlist carries that key -- so a stack word that "
            "happens to read AF_INET6 sends a garbage length into it",
        ),
        (
            "\tint version = 0;",
            "\tint version;\n\tint error;",
            "and prints version unconditionally while setting it only if "
            "the nvlist carries the key. syncdev, maxupdates and flags in "
            "the same declaration block are all given defaults for exactly "
            "this reason",
        ),
    ],

    "hbsd/src/usr.sbin/makefs/makefs.h": (
        "timerclear(&(x));",
        "#define\tTIMER_START(x)\t\t\t\t\\\n\tif (debug & DEBUG_TIME)",
        "TIMER_START writes x only when debug & DEBUG_TIME and "
        "TIMER_RESULTS reads it under a second test of the same global, "
        "with the work being timed in between, so every caller's bare "
        "`struct timeval start;' is only defined as long as nothing in the "
        "gap changes debug -- eight findings across makefs.c, ffs.c, "
        "msdos.c and walk.c",
    ),

    "hbsd/src/usr.sbin/makefs/zfs/dsl.c": (
        "errx(1, \"filesystem `%s' names no parent dataset\", name);",
        "dirname, name);\n\t\t}\n\t}\n\n\tdir->fullname = estrdup(name);",
        "dsl_dir_alloc() breaks out of its walk the first time strsep() "
        "finds no further separator, so a name with no `/' leaves parent at "
        "NULL and never reaches the errx() inside the loop -- and the three "
        "uses below all dereference it",
    ),

    "hbsd/src/usr.sbin/makefs/msdos/msdosfs_denode.c": (
        "\tu_long chaintofree = 0;",
        "\tu_long eofentry;\n\tu_long chaintofree;",
        "detrunc() writes chaintofree on the length == 0 path only; on the "
        "other it is written by fatentry(), which runs only if pcbmap() "
        "left eofentry something other than ~0ul -- and it is read "
        "unconditionally at the bottom and handed to freeclusterchain()",
    ),

    "hbsd/src/sys/fs/msdosfs/msdosfs_denode.c": (
        "\tu_long chaintofree = 0;",
        "\tu_long eofentry;\n\tu_long chaintofree;",
        "the same detrunc() shape in the kernel copy, where the "
        "consequence of reading a stack word there is freeclusterchain() "
        "walking and freeing an arbitrary FAT cluster chain",
    ),

    "hbsd/src/usr.sbin/mlxcontrol/interface.c": (
        ("bzero(&cmd, sizeof(cmd));\n    cmd.mu_status = 0xffff;", 4),
        "    struct mlx_usercommand\tcmd;\n\n    /* build the command */\n"
        "    cmd.mu_datasize",
        "mlx_perform() does nothing when the control device cannot be "
        "opened -- its open() has no else -- so cmd is never written and "
        "`cmd.mu_status != 0' reads stack garbage in all four callers. "
        "And cmd goes to the driver whole through an _IOWR: mu_command is "
        "sixteen bytes, no caller sets more than three, and "
        "mlx_scsi_inquiry() never sets mu_bufptr at all",
    ),

    "hbsd/src/sys/dev/isp/isp_freebsd.c": (
        "\tif (ccb != NULL) {\n\t\tccb->ccb_h.status &= ~CAM_STATUS_MASK;",
        "\tisp_async(isp, ISPASYNC_TARGET_NOTIFY_ACK, inot);\n"
        "\tccb->ccb_h.status &= ~CAM_STATUS_MASK;",
        "the SRR handler's fail: label dereferenced ccb, and one of the "
        "six `goto fail' sites is the branch that logs \"SRR[0x%x] null "
        "ccb\" -- so it arrived having said the pointer was NULL and "
        "faulted three lines later",
    ),

    "hbsd/src/sys/dev/netmap/netmap_mem2.c": (
        "\tif (ptnmd == NULL)\n\t\treturn (NULL);",
        "\tif (ptnmd == NULL) {\n\t\terr = ENOMEM;\n\t\tgoto error;",
        "netmap_mem_pt_guest_create() jumped to a label that calls "
        "netmap_mem_pt_guest_delete(&ptnmd->up). That callee does test for "
        "NULL, so it did not fault -- but forming &ptnmd->up on a null "
        "ptnmd is undefined (C17 6.5.3.2) and reaches the guard as NULL "
        "only because `up' happens to be the first member",
    ),

    "hbsd/src/sys/dev/videomode/pickmode.c": (
        "\t\tif (mtemp == NULL)\n\t\t\treturn;",
        "\t\t}\n\t\taspect = mtemp->hdisplay * 100 / mtemp->vdisplay;",
        "sort_modes(): mtemp is assigned only by a mode with a positive "
        "hdisplay -- hbest starts at zero and the test is `>' -- so a mode "
        "list whose hdisplay all decode to zero left it at its NULL "
        "initialiser. These modes come from a parsed EDID, which the "
        "monitor supplies",
    ),

    "hbsd/src/sys/dev/wtap/if_medium.c": [
        (
            "M_WTAP, M_WAITOK | M_ZERO);",
            "M_WTAP, M_NOWAIT | M_ZERO);",
            "init_medium() dereferenced an M_NOWAIT allocation on the next "
            "line; it runs from the MOD_LOAD handler with no lock held, so "
            "M_WAITOK is available and cannot fail",
        ),
        (
            "\tif (p == NULL) {\n\t\tDWTAP_PRINTF(\"[%d] no memory,",
            "\t    M_WTAP_PACKET, M_ZERO | M_NOWAIT);\n\tp->id = id;",
            "medium_transmit() holds md_mtx, so its allocation has to stay "
            "M_NOWAIT and therefore has to be checked; dropping the frame "
            "is what the md->open == 0 arm above already does",
        ),
    ],

    "hbsd/src/sys/dev/wtap/wtap_hal/hal.c": (
        ("M_WTAP, M_WAITOK | M_ZERO);", 2),
        "M_WTAP, M_NOWAIT | M_ZERO);",
        "init_hal() and new_wtap() dereferenced M_NOWAIT allocations "
        "immediately; both run without a lock -- MOD_LOAD and the wtapctl "
        "cdev ioctl",
    ),

    "hbsd/src/sys/dev/wtap/if_wtap_module.c": (
        ("M_WAITOK | M_ZERO);", 2),
        "M_WTAP, M_NOWAIT | M_ZERO);",
        "the MOD_LOAD handler handed one M_NOWAIT allocation straight to "
        "init_hal(), which dereferences it, and dereferenced the other on "
        "the next line",
    ),

    "hbsd/src/sys/dev/ata/chipsets/ata-promise.c": (
        "\tif (hp != NULL) {\n\t    hp->addr = hpkt;",
        "M_NOWAIT | M_ZERO);\n\thp->addr = hpkt;",
        "ata_promise_queue_hpkt() holds hpktp->mtx, so M_NOWAIT is "
        "required and the result was not checked. No sweep ever reported "
        "this one: it was found by widening nowait_check.py to see a cast "
        "between `=' and the allocator",
    ),

    "hbsd/src/sys/dev/tws/tws_cam.c": (
        'printf("tws: null softc in interrupt handler\\n");',
        'device_printf(sc->tws_dev, "null softc!!!\\n");',
        "tws_intr() printed \"null softc\" by reading sc->tws_dev out of "
        "the null softc, inside `if (!(sc))'",
    ),

    "hbsd/src/sys/dev/wdatwd/wdatwd.c": (
        ("if (ret != NULL)", 2),
        "wdat->entry.Mask;\n\t\t\t*ret = (x == wdat->entry.Value)",
        "wdatwd_action() writes *ret for the two READ instructions, and "
        "six of its nine callers pass ret = NULL -- which instructions an "
        "action runs comes from the firmware's WDAT table, an arbitrary "
        "sequence, so a READ under a SET_* action is one table away from a "
        "kernel write to NULL",
    ),

    "hbsd/src/sys/dev/qlnx/qlnxe/ecore_rdma.c": [
        (
            "\tif (!rdma_cxt)\n\t\treturn ECORE_INVAL;\n\tif (!qp) {",
            "\tif (!rdma_cxt || !qp) {",
            "ecore_rdma_destroy_qp() reported a NULL rdma_cxt through "
            "DP_ERR(p_hwfn, ...), and p_hwfn IS rdma_cxt -- DP_ERR expands "
            "to (p_dev)->dp_ctx and (p_dev)->name",
        ),
        (
            "\tif (!rdma_cxt)\n\t\treturn OSAL_NULL;",
            "\tif (!rdma_cxt || !in_params || !out_params ||",
            "ecore_rdma_create_qp(): the same, through p_hwfn->p_dev",
        ),
    ],

    "hbsd/src/sys/dev/qlnx/qlnxe/ecore_roce.c": (
        ("\tif (!rdma_cxt)\n\t\treturn ECORE_INVAL;", 2),
        "\tif (!rdma_cxt || !out_params) {",
        "ecore_roce_destroy_ud_qp() and ecore_roce_create_ud_qp() report a "
        "NULL rdma_cxt through DP_ERR(p_hwfn->p_dev, ...), and p_hwfn is "
        "the cast of rdma_cxt three lines up",
    ),

    "hbsd/src/sys/dev/hyperv/hvsock/hv_sock.c": (
        "\tif (pcb == NULL)\n\t\treturn (0);\n\tif (pcb->chan == NULL) {",
        "\tif (pcb == NULL || pcb->chan == NULL) {\n"
        "\t\tpcb->so->so_error = EIO;",
        "hvsock_canread_check() wrote pcb->so->so_error inside a test "
        "whose first disjunct is pcb == NULL",
    ),

    "hbsd/src/lib/libsdp/session.c": (
        "\tif (ss == NULL)\n\t\treturn (-1);\n"
        "\tif (l == NULL || ss->flags & SDP_SESSION_LOCAL) {",
        "\tif (l == NULL || ss == NULL || ss->flags & SDP_SESSION_LOCAL) {",
        "sdp_get_lcaddr() had ss == NULL as one disjunct and then wrote "
        "ss->error -- as does the fail: label it jumps to, and the return "
        "after it, so a NULL session had no way out that did not fault. "
        "service.c and search.c already open with the standalone test",
    ),

    "hbsd/src/sys/dev/pms/freebsd/driver/common/osdebug.h": (
        "KASSERT(0, (\"%s: %s\", __func__, message));",
        None,
        "OS_ASSERT prints and returns, so every "
        "SA_ASSERT(NULL != p, ...) followed by SA_ASSERT(0 != p->field, "
        "...) -- the RefTisa house style -- printed its message and then "
        "dereferenced the pointer it had just called NULL, one line "
        "later. KASSERT makes an INVARIANTS kernel stop at the assertion; "
        "a kernel without INVARIANTS is unchanged, print and all",
    ),

    "hbsd/src/sys/dev/firewire/sbp.c": [
        (
            ("sbp_nameunit", 5),
            "\"Invalid target (no wildcard)\\n\",\n"
            "\t\t\t\tdevice_get_nameunit(sbp->fd.dev),",
            "sbp_action() printed device_get_nameunit(sbp->fd.dev) at four "
            "sites where sbp may be or IS NULL -- one of them inside a "
            "branch conditioned on `sbp == NULL' -- all live at debug > 0, "
            "which boot -v sets",
        ),
    ],

    "hbsd/src/sys/dev/mmc/mmc.c": [
        (
            ("struct mmc_ivars *ivar = NULL;", 2),
            None,
            "mmc_wait_for_request()'s retune search leaves ivar unset when "
            "child_count is zero, and the loop is not entered",
        ),
        (
            "if (ivar == NULL || ivar->rca != sc->last_rca)",
            "\t\t\tif (ivar->rca != sc->last_rca)",
            "the test after that loop then read ivar->rca through an "
            "uninitialised stack pointer",
        ),
    ],

    "hbsd/src/sys/dev/mpi3mr/mpi3mr.c": [
        (
            "} else if (target != NULL && target->io_divert) {",
            "\t\t\t} else if (target->io_divert) {",
            "mpi3mr_process_op_reply_desc(): the arm above tests target for "
            "NULL -- it is what gates tg and throttle_enabled_dev -- and "
            "this else arm did not",
        ),
        (
            "/* PBSD: scsi_reply is NULL on the status-descriptor path. */",
            None,
            "the same function's SCSI_*_TERMINATED arm traced "
            "scsi_reply->IOCLogInfo, which a status descriptor does not "
            "carry",
        ),
        (
            "\t\tif (scsi_reply == NULL) {\n"
            "\t\t\tmpi3mr_set_ccbstatus(ccb,",
            "\tcase MPI3_IOCSTATUS_SUCCESS:\n"
            "\t\tif ((scsi_reply->IOCStatus & MPI3_IOCSTATUS_STATUS_MASK) ==",
            "and its DATA_UNDERRUN/RECOVERED_ERROR/SUCCESS arm read the "
            "whole absent reply frame -- MPI3_IOCSTATUS_SUCCESS is also "
            "ioc_status' initial value, so that is the arm a status "
            "descriptor lands in",
        ),
    ],

    "hbsd/src/sys/dev/mpi3mr/mpi3mr_cam.c": (
        "\"Device (dev_handle: %d) is already removed from driver's list\\n\",\n"
        "\t\t\thandle);",
        "is already removed from driver's list\\n\",\n"
        "\t\t\ttarget->per_id, handle);",
        "mpi3mr_remove_device_from_os() printed target->per_id inside "
        "`if (!target)' -- the branch that tests for target being NULL",
    ),

    "hbsd/src/sys/dev/ntb/ntb_hw/ntb_hw_amd.c": (
        "\tif (sb == NULL)\n\t\treturn (ENOMEM);",
        "\t\treturn (sb->s_error);",
        "amd_ntb_hw_info_handler() reported an sbuf allocation failure by "
        "reading sb->s_error out of the NULL sbuf",
    ),

    "hbsd/src/sys/dev/ntb/test/ntb_tool.c": (
        ("\t\trc = ENOMEM;", 2),
        "\t\trc = sb->s_error;",
        "the same sbuf-is-NULL-so-read-its-error line, twice, in "
        "tool_mw_read_fn() and tool_mw_trans_read()",
    ),

    "hbsd/src/sys/amd64/pci/pci_cfgreg.c": [
        (
            ("(1U << slot & pcie_badslots) != 0", 2),
            "(1 << slot & pcie_badslots) != 0",
            "amd64's copy of the i386 pcie_badslots defect: PCI_SLOTMAX is "
            "31 and pcie_init_badslots() walks 0..31, so `1 << 31' on a "
            "signed int is executed on every PCIe boot - found by grepping "
            "for the i386 fix's shape, not by a finding",
        ),
        (
            "pcie_badslots |= (1U << slot);",
            "pcie_badslots |= (1 << slot);",
            "amd64 pcie_init_badslots(): the write side of the same "
            "`1 << 31' on a signed int",
        ),
    ],

    "hbsd/src/sys/dev/mpr/mpr_sas_lsi.c": [
        (
            "if (error || (cm == NULL) || (reply == NULL)) {",
            "\treply = (Mpi2SataPassthroughReply_t *)cm->cm_reply;\n"
            "\tif (error || (reply == NULL)) {",
            "mprsas_get_sata_identify(): mpr_wait_command() sets *cmp = NULL "
            "on a reinit that reallocated the command pool, and the KASSERT "
            "guarding cm compiles to nothing without INVARIANTS, so "
            "cm->cm_reply was read off a null command - the same file's "
            "IR_CONFIGURATION_CHANGE_LIST handler already writes it this way",
        ),
        (
            "\t} else if ((cm->cm_flags & MPR_CM_FLAGS_SATA_ID_TIMEOUT) == 0) {",
            "\tif ((cm->cm_flags & MPR_CM_FLAGS_SATA_ID_TIMEOUT) == 0) {",
            "the same function's out: label read cm->cm_flags "
            "unconditionally, so guarding only the cm_reply load would have "
            "moved the fault rather than removed it; the buffer still has to "
            "be freed on that path because the reinit never touches cm_data",
        ),
    ],

    "hbsd/src/sys/dev/mps/mps_sas_lsi.c": [
        (
            "if (error || (cm == NULL) || (reply == NULL)) {",
            "\treply = (Mpi2SataPassthroughReply_t *)cm->cm_reply;\n"
            "\tif (error || (reply == NULL)) {",
            "the mps twin of the mpr_sas_lsi.c null-command read",
        ),
        (
            "\t} else if ((cm->cm_flags & MPS_CM_FLAGS_SATA_ID_TIMEOUT) == 0) {",
            "\tif ((cm->cm_flags & MPS_CM_FLAGS_SATA_ID_TIMEOUT) == 0) {",
            "the mps twin of the mpr_sas_lsi.c out: label fault",
        ),
        (
            "memset(&mpi_reply, 0, sizeof(mpi_reply));",
            None,
            "mpssas_get_sas_address_for_sata_disk() decided whether to retry "
            "from mpi_reply.IOCStatus and .SASStatus, which "
            "mpssas_get_sata_identify() only writes on its success path - "
            "the mpr copy of this function already carried the memset and "
            "mps was never updated",
        ),
    ],

    "hbsd/src/usr.sbin/pciconf/cap.c": (
        "if (b < (int)nitems(dw))\n\t\t\t\tdw[b] = dwv;",
        "\t\t\tdw[b] = read_config(fd, &p->pc_sel, ptr, 4);",
        "cap_ea: the userland twin of pci_ea_fill_info() -- PCIM_EA_ES "
        "is three bits, so a device claiming five to seven dwords wrote "
        "past uint32_t dw[4] with PCI configuration space",
    ),

    "hbsd/src/sbin/ipfw/ipfw2.c": [
        (
            "if (sz < sizeof(req))\n\t\tsz = sizeof(req);",
            None,
            "ipfw_list_objects: req.size is filled in by the kernel and was "
            "used as the calloc() size with no floor, so a zero or short "
            "value gave a zero-sized allocation that olh->size wrote "
            "through",
        ),
        (
            ("if (sz < sizeof(req))\n\t\tsz = sizeof(req);", 2),
            None,
            "ipfw_get_tracked_ifaces: the same unfloored kernel-supplied "
            "size, four hundred lines further down the same file",
        ),
    ],

    "hbsd/src/sbin/ipfw/tables.c": (
        "sz = sizeof(*oh);",
        "\tsz = 0;\n\toh = NULL;",
        "table_do_get_list: sz was seeded at zero and only grown when it "
        "was below i->size, so a kernel reporting size zero left it at "
        "zero and table_fill_objheader() wrote an ipfw_obj_header through "
        "a zero-sized allocation",
    ),

    "hbsd/src/bin/ps/ps.c": (
        'if (path == NULL)\n\t\txo_errx(1, "calloc failed");',
        None,
        "descendant_sort: the calloc for the sibling bitmap was the one "
        "unchecked allocation in the function -- the malloc two lines "
        "below it is checked with xo_errx",
    ),

    "hbsd/src/usr.sbin/kbdmap/kbdmap.c": (
        'if (km_sorted == NULL)\n\t\terr(1, "malloc");',
        None,
        "menu_read: km_sorted was malloc'd and then indexed in the very "
        "next statement with no check",
    ),

    "hbsd/src/usr.sbin/bsdinstall/partedit/partedit.c": (
        'if (tobesorted == NULL)\n\t\t\terr(1, "malloc");',
        None,
        "apply_changes: the fstab sort array was malloc'd and filled in "
        "immediately, with no check",
    ),

    "hbsd/src/sbin/fsck/preen.c": (
        "free(p->p_mntpt);",
        None,
        "p_devname, p_mntpt and p_type are all estrdup()ed when the "
        "partition is added; only two of the three were freed",
    ),

    "hbsd/src/usr.sbin/jail/config.c": (
        "free(wj);",
        None,
        "load_config: each wildcard jail record was removed from the "
        "list and its name and parameters freed, but the record itself "
        "was left behind",
    ),

    "hbsd/src/usr.sbin/nfsuserd/nfsuserd.c": [
        (
            ('syslog(LOG_ERR, "Can\'t add user %s\\n", nid.nid_name);', 2),
            'syslog(LOG_ERR, "Can\'t add user %s\\n", pwd->pw_name);',
            "nfsuserdsrv: an unknown uid or user name falls back to "
            "defaultuser with pwd left NULL, and the nfssvc() error arm "
            "then logged pwd->pw_name -- a remote NFSv4 client naming an "
            "id the server does not know could crash the daemon",
        ),
        (
            ('syslog(LOG_ERR, "Can\'t add group %s\\n",\n\t\t\t    nid.nid_name);', 2),
            'Can\'t add group %s\\n",\n\t\t\t    grp->gr_name);',
            "the same on the gid and group-name arms",
        ),
    ],

    "hbsd/src/usr.sbin/pkg/pkg.c": (
        "pkgsign_verify_data(sctx, data, datasz, pubkey, NULL, 0, pk->sig,",
        "pkgsign_verify_data(sctx, data, datasz, r->pubkey, NULL, 0, "
        "pk->sig,",
        "verify_pubsignature() computes a local pubkey precisely because "
        "r is NULL on the bootstrap path, and then verified against "
        "r->pubkey -- so a PUBKEY-signed bootstrap dereferenced NULL "
        "instead of checking the signature",
    ),

    "hbsd/src/usr.sbin/efivar/efivar.c": [
        (
            "static void __dead2\nrep_err(int eval, const char *fmt, ...)",
            None,
            "rep_err() either exit()s or calls verr(); breakdown_name() "
            "walked out of its rep_errx() and back to `*cp = NUL' with cp "
            "still NULL",
        ),
        (
            "static void __dead2\nrep_errx(int eval, const char *fmt, ...)",
            None,
            "the same for rep_errx()",
        ),
        (
            "static void __dead2\nusage(void)",
            None,
            "usage() ends in errx()",
        ),
    ],

    "hbsd/src/usr.bin/column/column.c": (
        "if (t->cols > 0)",
        None,
        "maketbl(): input() drops a line that is all whitespace, not one "
        "that is all *separator*, so `printf \"a:b\\n:::\\n\" | column -t "
        "-s:' left t->cols zero -- cols - 1 is then -1, the column loop "
        "does not run, and the trailing print read t->list[0] out of a "
        "calloc(0) and printed through it as a string",
    ),

    "hbsd/src/sbin/dump/cache.c": (
        "if (DataBase == MAP_FAILED)",
        None,
        "cinit() checked none of its three allocations, and cread() tests "
        "`DataBase == NULL' to decide whether the cache is initialised -- "
        "mmap() reports failure as MAP_FAILED, so a failed mapping read "
        "back as a successful one and every cached block was written "
        "through (char *)-1",
    ),

    "hbsd/src/sbin/ipf/libipf/parseipfexpr.c": [
        (
            "if (temp[0] == '\\0' || temp[strlen(temp) - 1] != ';') {",
            "\tif (temp[strlen(temp) - 1] != ';') {",
            "an empty expression read temp[-1] to find its last character",
        ),
        (
            'if (oplist == NULL) {\n\t\terror = "no operands";',
            None,
            "oplist is allocated by the first operand, so an expression "
            "that is nothing but separators -- `expr \";\"' -- left it NULL "
            "and the shift at the end wrote oplist[0] through it",
        ),
    ],


    "hbsd/src/usr.sbin/nscd/nscd.c": (
        "if (qstate->use_alternate_io == 0 &&\n\t\t    qstate->process_func "
        "!= NULL) {",
        "\t\tif (qstate->use_alternate_io == 0) {\n\t\t\tdo {",
        "process_socket_event(): the EVFILT_READ error arm sets both "
        "use_alternate_io = 0 and process_func = NULL, which is exactly "
        "the state the block below called process_func in -- the "
        "do-while tests it only as a continuation condition, after the "
        "first call has already gone through NULL",
    ),

    "hbsd/src/usr.bin/mail/quit.c": [
        (
            "FILE *ibuf = NULL, *obuf, *fbuf, *rbuf, *readstat, *abuf;",
            "FILE *ibuf, *obuf, *fbuf, *rbuf, *readstat, *abuf;",
            "quit(): ibuf is opened only on the `value(\"append\") == NULL' "
            "path, and the sendmessage() error arm closed it "
            "unconditionally -- an indeterminate FILE * to fclose() "
            "whenever `set append' is in effect and the write to the "
            "mbox fails",
        ),
        (
            "if (ibuf != NULL) {\n\t\trewind(ibuf);",
            'if (value("append") == NULL) {\n\t\trewind(ibuf);',
            "and the copy-back below asked value() the same question a "
            "second time rather than asking the pointer it had already "
            "opened",
        ),
    ],

    "hbsd/src/sbin/pfctl/pfctl_altq.c": (
        ("ENTRY\t item = { NULL, NULL };", 3),
        "\tENTRY\t item;\n\tENTRY\t*ret_item;",
        "pfaltq_lookup(), qname_to_pfaltq() and qname_to_qid() all pass "
        "an ENTRY to hsearch_r() by value with only .key set -- a FIND "
        "copies .data too, and nothing ever writes it",
    ),

    "hbsd/src/sbin/pfctl/pfctl_parser.c": (
        ("ENTRY\t \t\t item = { NULL, NULL }", 1),
        "\tENTRY\t \t\t item;\n\tENTRY\t\t\t*ret_item;\n\n\titem.key = name;",
        "is_a_group() and ifa_add_groups_to_map(): the same ENTRY passed "
        "by value with .data never set",
    ),

    "hbsd/src/usr.bin/ktrdump/ktrdump.c": (
        "u_long parms[KTR_PARMS] = { 0 };",
        "\tu_long parms[KTR_PARMS];",
        "the parse fills parms[0..parm) from the conversions the "
        "record's format string names, and the fprintf at the bottom "
        "passes all KTR_PARMS of them -- a format with fewer than six "
        "handed varargs whatever the array held",
    ),

    "hbsd/src/sbin/devmatch/devmatch.c": [
        (
            'printf("Ignoring %s (%c) tomatch=%#x\\n",',
            None,
            "search_hints(): the integer branch's `Ignoring' message "
            "printed v as table=, and v is fetched on the line after "
            "the message, not before it",
        ),
        (
            'printf("Ignoring %s (%c) tomatch=\'%s\'\\n",',
            None,
            "and the string branch's copy of the same message printed "
            "v, the integer branch's variable, which that path never "
            "assigns at all",
        ),
    ],


    "hbsd/src/usr.bin/systat/convtbl.c": (
        "idx = scale >= SC_BYTE && scale < SC_AUTO ? scale : SC_AUTO;",
        "idx = scale < SC_AUTO ? scale : SC_AUTO;",
        "get_tbl_ptr()'s comment says \"if our index is out of range, "
        "default to auto-scaling\" and the test was half of that: a "
        "NEGATIVE scale passed it, and &convtbl[idx] is a wild pointer "
        "both callers dereference at once. The one in-tree caller checks "
        "get_scale()'s -1 first, so it is the comment being right and "
        "the test being half of it -- but convert() and get_string() are "
        "declared taking a plain int and this line is where that range "
        "is meant to be enforced.",
    ),


    "hbsd/src/sys/amd64/include/ieeefp.h": (
        ("((unsigned)~_m << ", 2),
        "(~_m << FP_MSKS_OFF) & FP_MSKS_FLD;",
        "__fpsetmask() left-shifted ~_m, and fp_except_t is `int' on "
        "x86 -- a #define in x86_ieeefp.h -- so ~_m is negative for "
        "every mask with the top bit clear, which is every mask anyone "
        "passes, fpsetmask(0) included. A left shift of a negative "
        "value is undefined by C11 6.5.7p4 whatever the distance. Done "
        "unsigned; the & discards everything the change could affect. "
        "CBMC: FAILED -> PROVED.",
    ),

    "hbsd/src/sys/i386/include/ieeefp.h": (
        "((unsigned)~_m << FP_MSKS_OFF) & FP_MSKS_FLD;",
        "(~_m << FP_MSKS_OFF) & FP_MSKS_FLD;",
        "The i386 copy of the same undefined left shift of a negative "
        "value in fpsetmask().",
    ),


    "hbsd/src/lib/libpam/modules/pam_ksu/pam_ksu.c": (
        "return (KRB5_PARSE_MALFORMED);",
        None,
        "get_su_principal() documents itself as \"Returns 0 for success, "
        "or a com_err error code on failure\" -- and one arm returned 0 "
        "for failure. The `root' path does p = strrchr(principal_name, "
        "'@') and, when there is no realm, logs \"malformed principal "
        "name\", frees the name and does `return (rv)'. rv there is "
        "krb5_unparse_name()'s return, which the four lines above "
        "checked non-zero and passed -- so rv is provably 0. The "
        "function returns SUCCESS having written neither "
        "*su_principal_name nor a principal, and pam_sm_authenticate() "
        "then prints that uninitialised stack pointer and passes it to "
        "free(). A free() of an indeterminate pointer on the su-to-root "
        "path of pam_ksu(8). KRB5_PARSE_MALFORMED is what the error "
        "table calls it. clang core.CallAndMessage, pam_ksu.c:120.",
    ),
    "hbsd/src/lib/libfetch/http.c": (
        ("if ((nbuf = realloc(", 2),
        "if ((hbuf->buf = realloc(",
        "http_next_header() had `hbuf->buf = realloc(hbuf->buf, n)' "
        "twice -- the copy of the first header line and the "
        "concatenation of each continuation line. realloc() returning "
        "NULL leaves the old block allocated, and assigning that NULL "
        "over the only pointer to it loses it; hbuf->bufsize goes on "
        "describing a buffer that no longer exists, and "
        "clean_http_headerbuf() then frees NULL. Every header line of "
        "every HTTP fetch goes through the first of the two. Through a "
        "temporary, which keeps both the block and the invariant. clang "
        "unix.cstring.NullArg, http.c:547.",
    ),
    "hbsd/src/lib/libthr/thread/thr_sig.c": (
        "bzero(&oldact, sizeof(oldact));",
        None,
        "__thr_sigaction() declares `struct sigaction newact, oldact, "
        "oldact2' and fills oldact only by passing &oldact to "
        "__sys_sigaction(), which it calls under `if (act != NULL)' and "
        "under `else if (oact != NULL)'. sigaction(sig, NULL, NULL) is a "
        "legal call that takes neither branch and returns 0 -- and the "
        "fixup block below reads oldact.sa_handler unconditionally. So "
        "that call read an indeterminate value off the stack every time. "
        "Nothing escaped (the read only decides an assignment whose "
        "result is copied out solely when oact != NULL and ret == 0), "
        "but it is a read of an object never written. Zeroing it first "
        "makes sa_handler SIG_DFL, which is what \"nothing was "
        "retrieved\" means, so the fixup declines -- the same decision "
        "on every path, now for a stated reason. clang "
        "core.UndefinedBinaryOperatorResult, thr_sig.c:638.",
    ),
    "hbsd/src/lib/libc/compat-43/killpg.c": (
        "if (pgid == 1 || pgid == INT32_MIN) {",
        "if (pgid == 1) {\n\t\terrno = ESRCH;",
        "killpg() returned kill(-pgid, sig), and -pgid is undefined for "
        "the most negative pid_t -- __int32_t on every architecture, so "
        "the answer would be +2147483648, which is not a pid_t value "
        "either. No process group has that id, so it is ESRCH for the "
        "same reason pgid == 1 is. CBMC: FAILED -> PROVED.",
    ),

    "hbsd/src/lib/libc/gen/nice.c": (
        "newprio = (long)prio + incr;",
        "setpriority(PRIO_PROCESS, 0, prio + incr)",
        "prio is in [PRIO_MIN, PRIO_MAX] but incr is whatever the caller "
        "passed, so nice(INT_MAX) overflowed -- undefined rather than "
        "merely out of range. Computed in long and saturated to the int "
        "range; setpriority(2) clamps anyway, so no result that did not "
        "already overflow changes. CBMC: FAILED -> PROVED.",
    ),

    "hbsd/src/lib/libc/gen/timezone.c": (
        'sign,azone / 60,azone % 60);',
        "zone = -zone;",
        "_tztab() negated its int zone argument in place, which is "
        "undefined for the most negative int, and timezone(3) names no "
        "domain for it. Done in long, where every int has an exact "
        "negation. CBMC: FAILED -> PROVED/BOUNDED.",
    ),


    "hbsd/src/lib/msun/src/e_sqrt.c": (
        "if(i!=0) {",
        "\t    ix0 |= (ix1>>(32-i));\n\t    ix1 <<= i;",
        "sqrt()'s subnormal normalisation did ix1>>(32-i) with i "
        "reachable at zero, which is a shift by the operand's whole "
        "width: undefined. The while loop above exits as soon as ix0 is "
        "nonzero and fills ix0 from ix1>>11, so an ix1 with bit 31 set "
        "puts bit 20 of ix0 in place on the first pass and the for loop "
        "never runs its body. x = 0x0000000080000000 -- the subnormal "
        "whose mantissa is 2^31 -- does exactly that. Guarded; the "
        "guarded form is what the unguarded one meant, and 4,051 "
        "subnormal inputs give bit-identical results either way.",
    ),

    "hbsd/src/sys/dev/speaker/spkr.c": [
        (
            "if (pitch < 0 || pitch > (int)nitems(pitchtab))",
            None,
            "playstring()'s `N' case was the one GETNUM of six with no "
            "range check after it. playtone() subtracts one and indexes "
            "pitchtab[] with the result, so a play string of `N500' read "
            "past the end of the table and handed what it found to "
            "tone() as a frequency. Five of the six cases already do "
            "exactly this check against their own bound; this is the "
            "sixth.",
        ),
        (
            "#define GETNUM_MAX\t1000000",
            "{v = v * 10 + (*++cp - '0'); slen--;}",
            "The GETNUM accumulator saturates. A play string carries as "
            "many digits as it likes and `v = v * 10 + digit' overflows "
            "an int at ten of them, undefined before any of the six "
            "range checks gets to look at the result. GETNUM_MAX is "
            "past every bound any caller applies, so every string that "
            "parsed to something meaningful still parses to the same "
            "thing.",
        ),
        (
            "if (sustain > MAX_SUSTAIN)",
            None,
            "playtone()'s dot count came straight from the play string. "
            "At twenty dots snum overflows and at thirty-one sdenom "
            "wraps to zero -- and the `sdenom == 0' test three lines "
            "below is that wrap already noticed and answered at the "
            "symptom rather than the cause. CBMC: the `sustain - 1' "
            "property is gone.",
        ),
    ],


    "hbsd/src/sys/geom/geom_flashmap.c": (
        "if (type >= nitems(g_flashmap_slicers))",
        # No `unwanted': the fix is an INSERTION, so every line the
        # unfixed version had is still there. The first draft used
        # "g_topology_lock(); if (g_flashmap_slicers[type].slicer" and
        # it matched the FIXED file too -- an over-broad unwanted
        # marker reports the bug as back on a tree that has the fix.
        None,
        "flash_register_slicer()'s type indexes g_flashmap_slicers[] "
        "and nothing checked it. It is an exported interface -- "
        "slicer.h declares it for any driver, in tree or out -- and "
        "what it writes at the index is a FUNCTION POINTER the taste "
        "path later calls. Every in-tree caller passes a "
        "FLASH_SLICES_TYPE_* constant, so the bound changes nothing "
        "that works today. CBMC: FAILED -> PROVED.",
    ),


    "hbsd/src/usr.sbin/rtadvd/timer.c": (
        "tm_limit.tv_sec = (time_t)(~(uintmax_t)0 &",
        "tm_limit.tv_sec = (-1) & ~((time_t)1 <<",
        "rtadvd_timer_init() built its maximum timespec with "
        "`(time_t)1 << 63', which moves a one INTO the sign bit of a "
        "signed 64-bit type -- C11 6.5.7p4 requires the result to be "
        "representable and 2^63 is not. It runs unconditionally at "
        "start-up, so it is every rtadvd(8), not an edge case. Shifted "
        "in uintmax_t and narrowed back; the value is unchanged. "
        "CBMC: FAILED -> PROVED.",
    ),

    "hbsd/src/sbin/ifconfig/af_inet.c": (
        "a.s_addr = htonl(plen > 0 ? ~((1U << (32 - plen)) - 1) : 0);",
        "a.s_addr = htonl(plen ? ~((1 << (32 - plen)) - 1) : 0);",
        "get_mask() shifted a signed 1 by 32 - plen. A /1 prefix shifts "
        "by 31 and `1 << 31' does not fit an int, so the most ordinary "
        "width this can be handed is the one that breaks it. plen is "
        "ifa_prefixlen widened from a uint8_t out of a netlink message, "
        "0 to 255 rather than 0 to 32, so a wider one gives a negative "
        "distance as well -- clamped to 32, the widest mask there is. "
        "CBMC: FAILED -> PROVED.",
    ),

    "hbsd/src/usr.sbin/apm/apm.c": [
        (
            "if (i < 0 || i >= 10000)",
            "if (i >= 10000)\n\t\treturn -1;",
            "int2bcd() guarded only the top. A negative i makes i % 10 "
            "negative and shifting a negative value left is undefined; "
            "worse, |i| can need more than eight digits, so base passes "
            "31 and the distance exceeds the width too.",
        ),
        (
            "if (bcd < 0 || bcd > 0x9999)",
            "if (bcd > 0x9999)\n\t\treturn -1;",
            "bcd2int() guarded only the top, and this one does not "
            "merely misbehave: `bcd >>= 4' on a negative int is an "
            "arithmetic shift, so -1 stays -1 and the loop never ends. "
            "args.edi is a uint32_t straight out of the BIOS reply and "
            "bcd2int() takes an int, so a reply with the top bit set "
            "hangs apm(8). CBMC: FAILED -> PROVED for both.",
        ),
    ],


    "hbsd/src/lib/libc/string/strsignal.c": (
        "signum = (num < 0) ? -(unsigned int)num : (unsigned int)num;",
        "\t\tsignum = num;\n\t\tif (num < 0)",
        "strsignal(3) takes a plain int and the else arm handles every "
        "value outside the signal range, so strsignal(INT_MIN) reached "
        "`signum = -signum'. Negating the most negative int is "
        "undefined, and it does not even come out positive: signum "
        "stayed negative and \"0123456789\"[signum % 10] read off the "
        "front of the string literal, into the buffer strsignal() "
        "returns. The file had carried an `XXX: negative num ?' above "
        "the definition. Negated in unsigned, which is exact.",
    ),

    "hbsd/src/lib/libc/string/strerror.c": (
        "uerr = (num >= 0) ? (unsigned int)num : -(unsigned int)num;",
        "uerr = (num >= 0) ? num : -num;",
        "errstr()'s destination was already unsigned but `-num' was "
        "computed in int before it got there, so errstr(INT_MIN, ...) "
        "was undefined. The same line and the same fix as strsignal().",
    ),


    "hbsd/src/lib/libc/locale/xlocale.c": (
        "if (type < 0 || type >= XLC_LAST)",
        "if (type >= XLC_LAST)\n\t\treturn (NULL);",
        "querylocale() computed its index as ffs(mask & "
        "~LC_VERSION_MASK) - 1 and checked only the upper bound. ffs() "
        "answers 0 when no bit is set, so a mask naming no component -- "
        "querylocale(0, loc), or LC_VERSION_MASK on its own -- made "
        "type -1, read components[-1] out of bounds and returned a "
        "pointer taken from whatever was there. NULL is the answer it "
        "already gives for a mask naming a component it does not know.",
    ),


    "hbsd/src/lib/libcalendar/easter.c": (
        "dt.d = mc[((y % 19) + 19) % 19];",
        "dt.d = mc[y % 19];",
        "C's % keeps the sign of the dividend, so y % 19 is negative "
        "for every negative year and mc[] was indexed out of bounds. "
        "easterog() and easteroj() are public entry points taking a "
        "plain int year with no stated domain, and the rest of this "
        "library does handle years before 1. The metonic cycle is "
        "periodic mod 19, so the Euclidean remainder is also the "
        "mathematically right index and nothing at or above zero "
        "changes.",
    ),


    "hbsd/src/lib/libcalendar/calendar.c": (
        "nd = (int)(((long long)nd - nmonday) % 7);",
        "nd = (nd - nmonday) % 7;",
        "weekday() subtracted a cached day number of 729652 from its "
        "int parameter, which overflows -- undefined, not merely wrong "
        "-- for every nd below INT_MIN + 729652. It is a public "
        "libcalendar entry point with no stated domain, so that is its "
        "whole int range. Found by the fuzzing engine, which replayed "
        "the input and got a UBSan report naming the line.",
    ),


    "hbsd/src/usr.sbin/crunch/crunchide/exec_elf32.c": (
        "if (xe16toh(ehdr.e_shentsize) != sizeof(Elf_Shdr)) {",
        "shdrsize = shnum * xe16toh(ehdr.e_shentsize);",
        "ELFNAMEEND(hide)() allocated shnum * e_shentsize bytes and then "
        "indexed the result as an Elf_Shdr[], so an object file "
        "declaring a smaller e_shentsize made every shdrp[i] past the "
        "first read past the allocation -- and the product is computed "
        "in int, which two Elf_Half at their maximum overflow",
    ),

    "hbsd/src/usr.sbin/bluetooth/iwmbtfw/main.c": (
        'iwmbt_err("Unsupported hardware variant (%d)", hw_variant);',
        None,
        "iwmbt_init_firmware(): the two arms that set header_len are "
        "`hw_variant <= 0x14' and `hw_variant >= 0x17', so 0x15 and "
        "0x16 fell through both -- header_len unwritten, and ret left "
        "at its initial -1, which the sbe_type block below overwrites "
        "on success.  A device reporting either made iwmbt_load_fwfile() "
        "skip a garbage number of bytes of the firmware image",
    ),

    "hbsd/src/usr.sbin/mptutil/mpt_cam.c": (
        ("if (ccb.cdm.matches == NULL || ccb.cdm.patterns == NULL) {", 3),
        None,
        "fetch_path_id(), mpt_query_disk() and mpt_fetch_disks() each "
        "calloc() a match buffer and a pattern buffer and then write "
        "patterns[0] on the next line; none of the six allocations was "
        "checked",
    ),

    "hbsd/src/usr.sbin/mptutil/mpt_drive.c": [
        (
            "error = mpt_lookup_drive(list, drive, &PhysDiskNum);",
            "if (mpt_lookup_drive(list, drive, &PhysDiskNum) < 0) {",
            "drive_set_state(): the same dead `< 0' test, which left "
            "PhysDiskNum unwritten for mpt_pd_info()",
        ),
        (
            "mpt_free_pd_list(list);\t/* PBSD: as the success path does */",
            None,
            "and the error arm the fix made reachable leaks the pd list "
            "the success path three lines down frees",
        ),
    ],

    "hbsd/src/usr.sbin/ppp/bundle.c": (
        "while (f > 0)\n        free(iov[--f].iov_base);",
        None,
        "bundle_ReceiveDatalink() allocates the scatter/gather array a "
        "segment at a time and returned on the first failure, dropping "
        "iov[0..f) -- this runs on every link handover, and the one "
        "case that reaches it is the one where holding onto them hurts "
        "most",
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
