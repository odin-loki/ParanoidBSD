# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Translation units the analyser is EXPECTED not to compile, and why.

A file that does not compile contributes no findings and looks, in every
total this sweep prints, exactly like a file that is clean. That is not a
hypothetical: libexec/rtld-elf/rtld.c came back ERROR on an unresolvable
`#include "notes.h"', and the defect it was hiding cost thirty-seven boot
runs. See docs/BUILDING.md.

So the ERROR set is inventoried. Every entry here is a decision on the
record; anything erroring that is NOT here is a new hole, and anything
here that has started compiling is a stale exemption. Both are reported
by analyze.py --check-errors.

Most entries are one of three honest reasons:

  not a translation unit   the file is #included by another (arch trap.c
                           includes subr_syscall.c) and has no business
                           compiling alone.
  not descended into      a userland directory with a Makefile of its
                           own that no parent SUBDIR reaches, so bmake
                           in it answers questions about a program the
                           build never makes. Marked NOT_SUBDIR, and
                           checked - the NOT_NAMED check cannot be used
                           for these, because asking bmake IN the
                           directory names the sources whether or not
                           anything descends into it. That distinction
                           is the whole content of the marker.
  option-gated             it needs a kernel option no config in this
                           tree sets - KASAN, KCSAN, KMSAN, TSLOG,
                           DEVICE_POLLING, COMPAT_43TTY.
  wrong architecture       32-bit-only or arch-private helpers, against
                           an amd64 sweep.

What is NOT an honest reason is "it needs a define nobody supplied".
sys/kern/subr_param.c was in this list for exactly that - MAXUSERS,
which config(8) fakes into an option - and it is not any more. That file
sets hz, maxfiles, nbuf and maxproc, and `hz' being clamped there is an
argument this repository's triage table leans on, made about a file the
analyser had never read. One -D and it compiles.
"""
from __future__ import annotations

EXPECTED = {
    # Named by no Makefile, in a directory whose other sources ARE named
    # - so a prefix here would absorb the daemon along with the probes.
    # Each is a `main()' that calls one function to see what the platform
    # does, from when that was worth checking; none is in bootpd's SRCS.
    "libexec/bootpd/trygetea.c":    "a hand-run probe, not in SRCS. NOT_NAMED",
    "libexec/bootpd/trygetif.c":    "a hand-run probe, not in SRCS. NOT_NAMED",
    "libexec/bootpd/trylook.c":     "a hand-run probe, not in SRCS. NOT_NAMED",

    # gzip(1) is a single-source PROG - usr.bin/gzip/Makefile has no SRCS
    # at all - and gzip.c #includes the decompressors by name at :2201
    # through :2216. unlz.c is the sixth of them and happens to compile
    # alone, so it is not here: the rule is what the build does, not what
    # the directory looks like.
    "usr.bin/gzip/unbzip2.c":     "INCLUDED_BY:usr.bin/gzip/gzip.c",
    "usr.bin/gzip/unpack.c":      "INCLUDED_BY:usr.bin/gzip/gzip.c",
    "usr.bin/gzip/unxz.c":        "INCLUDED_BY:usr.bin/gzip/gzip.c",
    "usr.bin/gzip/unzstd.c":      "INCLUDED_BY:usr.bin/gzip/gzip.c",
    "usr.bin/gzip/zuncompress.c": "INCLUDED_BY:usr.bin/gzip/gzip.c",

    # locate(1)'s SRCS is `util.c locate.c'; locate.c #includes
    # fastfind.c four times, at :326, :328, :335 and :337, each with a
    # different set of macros defined.
    "usr.bin/locate/locate/fastfind.c":
        "INCLUDED_BY:usr.bin/locate/locate/locate.c",

    # Not a source at all. Each ipfilter program seds common/lexer.c into
    # its own lexer -- `ipf_l.c: lexer.c' with s/yy/ipf_yy/g,
    # s/y.tab.h/ipf_y.h/ and s/lexer.h/ipf_l.h/ in sbin/ipf/ipf/Makefile
    # :21, and the same rule in ipmon, ipnat, ippool and ipftest -- so its
    # `#include "y.tab.h"' at :25 names a file that exists only after the
    # sed has run.
    "sbin/ipf/common/lexer.c":
        "a sed template, not a translation unit. NOT_NAMED",

    # usr.bin/lex's bootstrap copy of the scanner. The Makefile's
    # GENFILES is `parse.c parse.h scan.c skel.c' and its `bootstrap:'
    # target copies init<name> over each when they differ, so the init*
    # files are the checked-in fallback rather than sources; SRCS names
    # scan.c. Only initscan.c is an ERROR - it opens with scan.l's
    # `#include "parse.h"', which yacc makes from parse.y and no rule
    # makes for a file the build never compiles. initparse.c and
    # initskel.c are equally unnamed but compile clean, so they are not
    # in this inventory: EXPECTED is the list of translation units that
    # FAIL, and the --check-errors gate calls an entry that compiles
    # stale.
    "usr.bin/lex/initscan.c":  "a bootstrap copy, not in SRCS. NOT_NAMED",

    # Test programs no build walks.
    "usr.sbin/bhyve/mevent_test.c":
        "a hand-run probe, not in SRCS. NOT_NAMED",
    "sbin/setkey/test-pfkey.c":  "a hand-run probe, not in SRCS. NOT_NAMED",
    "sbin/setkey/test-policy.c": "a hand-run probe, not in SRCS. NOT_NAMED",
    "usr.sbin/rpc.lockd/test.c":
        "a hand-run probe, not in SRCS. NOT_NAMED",

    # usr.bin/tip/libacu holds ten ACU drivers and no Makefile of its
    # own; usr.bin/tip/tip/Makefile reaches them with
    #
    #     .PATH:  ${.CURDIR}/../libacu
    #     SRCS=   ... biz22.c courier.c df.c dn11.c hayes.c t3000.c
    #             v3451.c v831.c ventel.c
    #
    # which is nine of the ten. biz31.c is the one it does not name.
    "usr.bin/tip/libacu/biz31.c":
        "nine of its ten siblings are in tip's SRCS and this one is "
        "not. NOT_NAMED",

    # usr.sbin/traceroute/Makefile:6 is
    #     SRCS= as.c traceroute.c ifaddrlist.c findsaddr-udp.c
    # so the socket variant of findsaddr is the road not taken.
    "usr.sbin/traceroute/findsaddr-socket.c":
        "SRCS names findsaddr-udp.c instead. NOT_NAMED",

    # Named only under an option, and the option is off. bhyve's
    # Makefile:74 is
    #
    #     .if ${MK_BHYVE_SNAPSHOT} != "no"
    #     SRCS+= snapshot.c
    #     .endif
    #
    # and BHYVE_SNAPSHOT is in share/mk/src.opts.mk's
    # __DEFAULT_NO_OPTIONS, so bmake asked with the tree's own defaults
    # does not name it -- which is why this is NOT_NAMED and not the
    # DEFAULT_OFF marker below, where the file IS named and the
    # DIRECTORY is what the option gates.
    "usr.sbin/bhyve/snapshot.c":
        "SRCS+= only under MK_BHYVE_SNAPSHOT, which is "
        "__DEFAULT_NO. NOT_NAMED",

    # sbin/Makefile:85 is SUBDIR.${MK_VERIEXEC}+= veriexec, and
    # src.opts.mk:246 makes VERIEXEC depend on BEARSSL, which is
    # __DEFAULT_NO. bmake in the directory names nothing either, because
    # its own Makefile is inside the same .if.
    "sbin/veriexec/veriexec.c":
        "MK_VERIEXEC depends on BEARSSL, which is "
        "__DEFAULT_NO. NOT_NAMED",

    # Named by its own Makefile, in a directory the build only descends
    # into under an option that is off. usr.bin/Makefile:194 is
    #
    #     SUBDIR.${MK_DIALOG}+=  dpv
    #
    # and DIALOG is in __DEFAULT_NO_OPTIONS, so nothing builds dpv and
    # <dialog.h> - contrib/dialog/dialog.h, installed by gnu/lib/
    # libdialog only when that option is on - is not on any include
    # path. NOT_NAMED cannot say this: bmake IN the directory does name
    # dpv.c. NOT_SUBDIR cannot either: the parent's SUBDIR line does
    # list it, conditionally.
    "usr.bin/dpv/dpv.c":      "DEFAULT_OFF:DIALOG",

    # Built - MK_CUSE and MK_SOUND are both __DEFAULT_YES - and needing
    # a port. Its own Makefile says so in two lines:
    #
    #     CFLAGS+=  -I${SRCTOP}/usr.sbin/virtual_oss/virtual_oss \
    #               -I/usr/local/include
    #     LDFLAGS+= -L/usr/local/lib -lm -lfftw3
    #
    # /usr/local is the ports prefix; <fftw3.h> is math/fftw3 and is in
    # no part of this tree.
    "usr.sbin/virtual_oss/virtual_equalizer/equalizer.c":
        "NEEDS_LOCALBASE",

    # cxgbetool's four register tables. cxgbetool.c:92-95 is
    #
    #     #include "reg_defs_t4.c"
    #     #include "reg_defs_t5.c"
    #     #include "reg_defs_t6.c"
    #     #include "reg_defs_t4vf.c"
    #
    # each of which is a bare initialiser list continuing a declaration
    # the including file opened, so none of the four is a translation
    # unit and SRCS names none of them.
    "usr.sbin/cxgbetool/reg_defs_t4.c":
        "INCLUDED_BY:usr.sbin/cxgbetool/cxgbetool.c",
    "usr.sbin/cxgbetool/reg_defs_t5.c":
        "INCLUDED_BY:usr.sbin/cxgbetool/cxgbetool.c",
    "usr.sbin/cxgbetool/reg_defs_t6.c":
        "INCLUDED_BY:usr.sbin/cxgbetool/cxgbetool.c",
    "usr.sbin/cxgbetool/reg_defs_t4vf.c":
        "INCLUDED_BY:usr.sbin/cxgbetool/cxgbetool.c",

    # crunchgen's skeleton. Its Makefile:5-6 is
    #
    #     crunched_skel.c: crunched_main.c
    #         sh -e ${.CURDIR}/mkskel.sh ${.CURDIR}/crunched_main.c \
    #             >crunched_skel.c
    #
    # and mkskel.sh turns the file's TEXT into a C string literal, so
    # crunched_main.c is data to the build, never compiled here. SRCS is
    # `crunchgen.c crunched_skel.c'.
    "usr.sbin/crunch/crunchgen/crunched_main.c":
        "input to mkskel.sh, not in SRCS. NOT_NAMED",

    # Two more the build names nowhere. route6d's Makefile has no SRCS
    # at all, so bsd.prog.mk's default is route6d.c and the misc/
    # directory is not descended into; ntpdc's SRCS is `ntpdc.c
    # ntpdc_ops.c ntpdc-opts.c version.c' and nl.c is not among them.
    "usr.sbin/route6d/misc/cksum.c":
        "not in SRCS; misc/ is not built. NOT_NAMED",
    "usr.sbin/ntp/ntpdc/nl.c":
        "not in SRCS. NOT_NAMED",

    # ipfilter's application proxies. ip_proxy.c is the translation
    # unit and it #includes the nine of them by name, three of those
    # inside `#if defined(_KERNEL)'. The tenth, ip_dns_pxy.c, is
    # referenced by nothing anywhere in the tree - ipfilter ships it and
    # FreeBSD has never wired it up.
    "sys/netpfil/ipfilter/netinet/ip_ftp_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_tftp_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_rcmd_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_pptp_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_irc_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_raudio_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_netbios_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_ipsec_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_rpcb_pxy.c":
        "INCLUDED_BY:sys/netpfil/ipfilter/netinet/ip_proxy.c",
    "sys/netpfil/ipfilter/netinet/ip_dns_pxy.c":
        "a tenth proxy nothing includes and nothing builds. NOT_NAMED",

    # msun's Bell Labs gamma: b_tgamma.c and its long double twin each
    # #include the exp and log they need, so four of the six files in
    # those two directories are not translation units.
    "lib/msun/bsdsrc/b_exp.c":
        "INCLUDED_BY:lib/msun/bsdsrc/b_tgamma.c",
    "lib/msun/bsdsrc/b_log.c":
        "INCLUDED_BY:lib/msun/bsdsrc/b_tgamma.c",
    "lib/msun/ld80/b_expl.c":
        "INCLUDED_BY:lib/msun/ld80/b_tgammal.c",
    "lib/msun/ld80/b_logl.c":
        "INCLUDED_BY:lib/msun/ld80/b_tgammal.c",

    # not a translation unit: #included by another file
    "sys/kern/kern_ctf.c":          "#included by kern_linker.c",
    "sys/kern/subr_syscall.c":      "#included by each arch's trap.c",
    "sys/kern/systrace_args.c":     "generated, #included by the dtrace glue",
    "sys/kern/subr_busdma_bounce.c": "#included by each arch's busdma",

    # Sweep 10's first --check-errors over sys/fs and sys/cddl. Nine of
    # these are the DTrace layout: dtrace.c is one translation unit that
    # #includes the rest of the directory, `#include <dtrace_anon.c>' at
    # :18479 and its neighbours. INCLUDED_BY names the file that does it,
    # and test_expected_errors.py checks that the file exists and really
    # does include this one.
    "sys/cddl/dev/dtrace/dtrace_anon.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cddl/dev/dtrace/dtrace_debug.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cddl/dev/dtrace/dtrace_hacks.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cddl/dev/dtrace/dtrace_ioctl.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cddl/dev/dtrace/dtrace_load.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cddl/dev/dtrace/dtrace_modevent.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cddl/dev/dtrace/dtrace_sysctl.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cddl/dev/dtrace/dtrace_unload.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cddl/dev/dtrace/dtrace_vtime.c":
        "INCLUDED_BY:sys/cddl/contrib/opensolaris/uts/common/dtrace/dtrace.c",
    "sys/cam/ctl/ctl_ser_table.c": "INCLUDED_BY:sys/cam/ctl/ctl.c",

    # Sweep 14's reconciliation. Each of these came out of
    # --check-errors as "does not compile and is not in EXPECTED", and
    # each was sorted the way sweep_report.py sorts them: does the BUILD
    # name it. Those it does not name are here; those it does are below,
    # with what the analyser is not yet giving them.
    #
    # Not translation units at all - another source #includes them.
    "lib/libc/regex/engine.c":
        "INCLUDED_BY:lib/libc/regex/regexec.c",
    "lib/libc/tests/stdbit/stdbit-test-framework.c":
        "INCLUDED_BY:lib/libc/tests/stdbit/stdc_bit_ceil_test.c",
    "lib/libc/tests/stdbit/stdbit-test-kernel.c":
        "INCLUDED_BY:lib/libc/tests/stdbit/stdbit-test-framework.c",
    "sys/dev/filemon/filemon_wrapper.c":
        "INCLUDED_BY:sys/dev/filemon/filemon.c",
    "sys/dev/xen/netback/netback_unit_tests.c":
        "INCLUDED_BY:sys/dev/xen/netback/netback.c",
    "sys/dev/aic7xxx/aic_osm_lib.c":
        "INCLUDED_BY:sys/dev/aic7xxx/aic79xx_osm.c",
    "sys/arm64/vmm/vmm_hyp.c":
        "INCLUDED_BY:sys/arm64/vmm/vmm_nvhe.c",
    "sys/i386/i386/pmap.c":
        "INCLUDED_BY:sys/i386/i386/pmap_pae.c",
    "sys/i386/i386/minidump_machdep_base.c":
        "INCLUDED_BY:sys/i386/i386/minidump_machdep_pae.c",
    # systrace's argument tables. Five architectures' worth, and the one
    # translation unit that compiles them is DTrace's provider, which
    # #includes whichever the kernel has.
    "sys/amd64/linux/linux_systrace_args.c":
        "INCLUDED_BY:sys/cddl/dev/systrace/systrace.c",
    "sys/amd64/linux32/linux32_systrace_args.c":
        "INCLUDED_BY:sys/cddl/dev/systrace/systrace.c",
    "sys/arm64/linux/linux_systrace_args.c":
        "INCLUDED_BY:sys/cddl/dev/systrace/systrace.c",
    "sys/i386/linux/linux_systrace_args.c":
        "INCLUDED_BY:sys/cddl/dev/systrace/systrace.c",
    "sys/compat/freebsd32/freebsd32_systrace_args.c":
        "INCLUDED_BY:sys/cddl/dev/systrace/systrace.c",

    # Named by nothing: no files* line, no module SRCS, no #include.
    "sys/dev/axgbe/xgbe-ptp.c":
        "not built: named by no files* or SRCS",
    "sys/dev/drm2/ttm/ttm_page_alloc_dma.c":
        "not built: wants <linux/dma-mapping.h>; drm2's TTM keeps the "
        "Linux allocator's file and names it nowhere",
    "sys/dev/pms/freebsd/driver/ini/src/osapi.c":
        "not built: named by no files* or SRCS",
    "sys/ofed/drivers/infiniband/core/ib_umem_odp.c":
        "not built: named by no files* or SRCS",
    "sys/ofed/drivers/infiniband/core/ib_umem_rbtree.c":
        "not built: named by no files* or SRCS",
    "sys/ofed/drivers/infiniband/ulp/ipoib/ipoib_ethtool.c":
        "not built: named by no files* or SRCS",
    "sys/ofed/drivers/infiniband/ulp/ipoib/ipoib_fs.c":
        "not built: named by no files* or SRCS",
    "sys/ofed/drivers/infiniband/ulp/ipoib/ipoib_vlan.c":
        "not built: named by no files* or SRCS",
    "sys/ofed/drivers/infiniband/ulp/sdp/sdp_proc.c":
        "not built: wants <linux/proc_fs.h>, and named by nothing",
    "sys/ofed/drivers/infiniband/ulp/sdp/sdp_zcopy.c":
        "not built: wants <asm/ioctls.h>, and named by nothing",
    "sys/ofed/drivers/infiniband/util/madeye.c":
        "not built: wants <rdma/ib_mad.h>, and named by nothing",
    # ...and the OFED SDP transport itself, which IS named by
    # sys/conf/files but whose own sdp.h opens with <rdma/ib_verbs.h> -
    # an OFED header this tree does not ship.
    "sys/ofed/drivers/infiniband/ulp/sdp/sdp_bcopy.c":
        "wants <rdma/ib_verbs.h>, which this tree does not have",
    "sys/ofed/drivers/infiniband/ulp/sdp/sdp_cma.c":
        "wants <rdma/ib_verbs.h>, which this tree does not have",
    "sys/ofed/drivers/infiniband/ulp/sdp/sdp_main.c":
        "wants <rdma/ib_verbs.h>, which this tree does not have",
    "sys/ofed/drivers/infiniband/ulp/sdp/sdp_rx.c":
        "wants <rdma/ib_verbs.h>, which this tree does not have",
    "sys/ofed/drivers/infiniband/ulp/sdp/sdp_tx.c":
        "wants <rdma/ib_verbs.h>, which this tree does not have",

    # Named only by a fragment no configuration pulls in - the same shape
    # as sys/dev/qcom_ess_edma, established by reading which configs
    # declare which tokens. sys/arm/ti/files.ti is named by no arm
    # config in this tree.
    "sys/arm/ti/ti_gpio.c":
        "not built: sys/arm/ti/files.ti, which no arm config names",
    "sys/arm/ti/clk/ti_gate_clock.c":
        "not built: sys/arm/ti/files.ti, which no arm config names",
    "sys/arm/ti/clk/ti_mux_clock.c":
        "not built: sys/arm/ti/files.ti, which no arm config names",
    "sys/arm/broadcom/bcm2835/bcm2835_fb.c":
        "not built: `optional sc', and no arm config declares `device sc'",
    "sys/dev/gpio/gpiomdio.c":
        "not built: `optional gpiomdio mii_bitbang', and no config on any "
        "architecture declares gpiomdio",
    "sys/dev/xdma/xdma_fdt_test.c":
        "not built: `optional xdma xdma_test fdt', and no config declares "
        "xdma_test",
    "sys/dev/ath/ath_hal/ar5212/ar2316.c":
        "not built: `optional ath_rf2316', declared by no config",
    "sys/dev/ath/ath_hal/ar5212/ar2317.c":
        "not built: `optional ath_rf2317', declared by no config",
    "sys/dev/ath/ath_rate/amrr/amrr.c":
        "not built: `optional ath_rate_amrr', declared by no config",
    "sys/dev/ath/ath_rate/onoe/onoe.c":
        "not built: `optional ath_rate_onoe', declared by no config",

    # Built by an explicit rule in a module Makefile whose target is in
    # neither SRCS nor OBJS, so ask_module() does not see it.
    "sys/i386/linux/linux_vdso_gtod.c":
        "built by sys/modules/linux/Makefile:105's own rule, which "
        "ask_module() does not read",
    "sys/dev/dwwdt/dwwdt.c":
        "sys/modules/dwwdt builds it; the FDT clock API it calls is "
        "declared only where the analyser is not looking",

    # A file's own CFLAGS.<file>. Eight entries were here and are gone:
    # ask_cflags() asks bmake for CFLAGS.<file> now, and asks it as
    # clang, so libc's dlfcn.c and tls.c get ${RTLD_HDRS}, the four
    # blocks tests get -fblocks and are NAMED at all, i387/fenv.c is
    # read as i386, and detect_tz_changes_test gets contrib/tzcode.
    # These two are what is left, and neither is about a flag.
    "libexec/atf/atf-pytest-wrapper/atf_pytest_wrapper.cpp":
        "#include <format>, which needs a C++20 standard library",
    "sys/crypto/sha2/sha256c_arm64.c":
        "built by lib/libmd/Makefile through a .PATH into sys/, with "
        "CFLAGS.sha256c_arm64.c+= -march=armv8-a+crypto",
    "sys/crypto/sha2/sha512c_arm64.c":
        "built by lib/libmd/Makefile through a .PATH into sys/, with "
        "CFLAGS.sha512c_arm64.c+= -march=armv8.2-a+sha3",

    # (The six entries that were here - the five aesni sources and
    # crypto/armv8's wrapper, all of them "the flags are on the
    # compile-with of a .o target and the reader only looks at .c
    # entries" - are gone: includes.files_compile_with() reads that
    # entry now, and files_arch_index() reads its `dependency' so
    # armv8_crypto_wrap.c is aarch64 rather than amd64. All six compile.)

    # A header the BUILD generates by assembling and linking, which this
    # analyser does not do. Each is a genassym-shaped recipe: build a
    # small object, read its symbols, write a .h.
    "sys/amd64/acpica/acpi_wakeup.c":  "wants acpi_wakecode.h, generated",
    "sys/i386/acpica/acpi_wakeup.c":   "wants acpi_wakecode.h, generated",
    "sys/amd64/amd64/elf_machdep.c":   "wants vdso_offsets.h, generated",
    "sys/amd64/ia32/ia32_signal.c":    "wants vdso_ia32_offsets.h, generated",
    "sys/amd64/ia32/ia32_syscall.c":   "wants vdso_ia32_offsets.h, generated",
    "sys/compat/ia32/ia32_sysvec.c":   "wants vdso_ia32_offsets.h, generated",

    # An interface header generated from a .m the ARCHITECTURE'S OWN
    # files* names, in another architecture's directory. incs for the
    # *_if.h are generated by directory, and these two live under
    # sys/arm/ while sys/conf/files.arm64 is what names them.
    "sys/arm64/arm64/gic_v3.c":
        "wants gic_if.h, from sys/arm/arm/gic_if.m",
    "sys/arm64/nvidia/tegra210/tegra210_coretemp.c":
        "wants tegra_soctherm_if.h, from sys/arm/nvidia/tegra_soctherm_if.m",

    # sys/conf/Makefile.<arch>:28 computes LINUX_DTS_VERSION with awk
    # over $S/dts/freebsd-compatible.dts and passes it as a -D; the
    # reader takes -D from that file but not one whose value is a make
    # variable it has not expanded.
    "sys/arm/arm/machdep.c":    "wants -DLINUX_DTS_VERSION, computed by awk",
    "sys/arm64/arm64/machdep.c": "wants -DLINUX_DTS_VERSION, computed by awk",

    # amd64's bhyve. Six sources were here - vmm.c, vmm_ioport.c,
    # vmm_lapic.c, amd/svm.c, amd/vmcb.c and intel/vmx.c - all on
    # "'vatpic.h' file not found". sys/modules/vmm/Makefile puts three
    # of bhyve's four -I inside `.elif ${MACHINE_CPUARCH} == "amd64"',
    # and the module reader refused every else; it decides them now.
    #
    # This one is NOT an include path. amdv.c is named by
    # no files* line and by no module - sys/modules/vmm builds
    # amdvi_hw.c and not this - and with the -I above it still fails:
    #
    #   io/iommu.h:62      extern const struct iommu_ops iommu_ops_amd;
    #   amd/amdvi_hw.c:1367      const struct iommu_ops iommu_ops_amd = {
    #   amd/amdv.c:118                 struct iommu_ops iommu_ops_amd = {
    #
    # A second definition of the same object, without the const the
    # header and the live definition both carry. It is the "not
    # implemented" stub amdvi_hw.c replaced, left behind: were it built,
    # it would be a duplicate symbol.
    "sys/amd64/vmm/amd/amdv.c":
        "not built: the superseded AMD IOMMU stub, named by nothing, and "
        "its iommu_ops_amd disagrees with iommu.h about const",

    # -DKLD_MODULE. sys/conf/kmod.mk:121 passes it and sys/sys/sysctl.h
    # :140 relaxes SYSCTL's type CTASSERT under it, so a driver only ever
    # built as a module may overspecify a sysctl's type. These three do.
    "sys/dev/amd_ecc_inject/ecc_inject.c": "needs -DKLD_MODULE",
    "sys/dev/iicbus/adc/pcf8591.c":        "needs -DKLD_MODULE",
    "sys/dev/iicbus/sensor/htu21.c":       "needs -DKLD_MODULE",

    # The rest, each read once and left with what it said.
    "sys/amd64/amd64/mp_machdep.c":
        "acpi_pxm_get_cpu_locality() is declared inside `#ifdef DEV_ACPI' "
        "in a header this file does not get the macro for",
    "sys/arm/arm/gic_acpi.c":
        "acfreebsd.h:185 wants machine/acpica_machdep.h, which sys/arm "
        "does not have - ACPI on arm is arm64's",
    "sys/arm/arm/pmu_acpi.c":
        "acfreebsd.h:185 wants machine/acpica_machdep.h, which sys/arm "
        "does not have - ACPI on arm is arm64's",
    "sys/arm/broadcom/bcm2835/raspberrypi_virtgpio.c":
        "calls pmap_mapdev_attr() under a name sys/arm does not declare",
    "sys/i386/i386/mp_machdep.c":
        "#error \"The apic device is required for SMP\" - DEV_APIC, which "
        "i386's DEFAULTS declares and the reader does not reach",
    "sys/dev/ispfw/ispfw.c":
        "the firmware images are .h files the build generates from "
        "vendor blobs",
    "sys/dev/mlx5/mlx5_fpga_tools/mlx5fpga_tools_char.c":
        "the Innova FPGA half of mlx5 again - see the mlx5_fpga/ prefix "
        "above; this one sits outside it",
    "sys/dev/vt/hw/fb/vt_early_fb.c":
        "the whole body is inside `#ifdef FDT', and the reader supplies "
        "no -DFDT for a file `optional vt fdt' on an architecture whose "
        "configs do not all set it",

    # Userland programs that live under sys/. Each opens with <stdio.h>,
    # <inttypes.h> or <assert.h> and is compiled by hand, not by the
    # kernel build - the same class as libexec/bootpd's try*.c probes.
    "sys/crypto/rijndael/test00.c":
        "a hand-run test program; wants <stdio.h> under -nostdinc",
    "sys/crypto/skein/skein_debug.c":
        "upstream's debug printer; wants <stdio.h> under -nostdinc",
    "sys/dev/random/unit_test.c":
        "the yarrow/fortuna unit test; wants <inttypes.h> under -nostdinc",
    "sys/dev/videomode/test.c":
        "a hand-run mode-table test; wants <stdio.h> under -nostdinc",

    # ...and the flag order the Makefile walk gets wrong.
    "lib/msun/arm/fenv-softfp.c":
        "lib/msun/Makefile:18 adds -I${.CURDIR}/x86 inside an `.if' that "
        "armv7 does not take; the walk cannot evaluate the condition, so "
        "x86's <fenv.h> lands ahead of arm's and __fetestexcept_int() is "
        "undeclared",

    # Sweep 10's sys/contrib shard. Everything here is a source no
    # sys/conf/files* entry and no module Makefile names - checked with
    # includes.kernel_flag_index(), which resolves SRCS through .PATH -
    # in a directory where OTHER sources are built, so a prefix would
    # absorb the built ones too.
    "sys/contrib/alpine-hal/al_hal_pcie.c":
        "one of nine in the directory that nothing names; the other eight "
        "are in sys/conf/files under `optional al_iofic' and compile",
    "sys/contrib/libb2/blake2b.c":
        "the reference implementation. sys/conf/files builds "
        "blake2b-ref.c and the SSE variants, not this",
    "sys/contrib/libb2/blake2s.c": "the same, for BLAKE2s",
    "sys/contrib/openzfs/module/icp/illumos-crypto.c":
        "illumos' crypto framework entry point, which wants "
        "<linux/module.h>; the FreeBSD build takes none of icp/",
    "sys/contrib/openzfs/module/icp/include/generic_impl.c":
        "a template #included by the icp algorithm sources, not a "
        "translation unit - it opens on IMPL_OPS_T, which its includer "
        "defines first",
    "sys/contrib/openzfs/module/zcommon/simd_stat.c":
        "the Linux /proc/spl/kstat SIMD reporter; the FreeBSD build does "
        "not name it",
    "sys/contrib/openzfs/module/zstd/zstd-in.c":
        "an amalgamation that #includes common/debug.c and the rest of "
        "zstd's sources, for builds that want one translation unit",
    "sys/contrib/xz-embedded/linux/lib/decompress_unxz.c":
        "Linux's decompressor glue; the whole linux/lib directory is "
        "unnamed by this tree's build",
    "sys/contrib/xz-embedded/linux/lib/xz/xz_dec_syms.c":
        "EXPORT_SYMBOL definitions for a Linux module",
    "sys/contrib/xz-embedded/linux/lib/xz/xz_dec_test.c":
        "a Linux kernel module that exercises the decoder",
    "sys/contrib/zlib/gzclose.c":
        "zlib's gzip FILE * layer, which needs <stdio.h>; sys/conf/files "
        "builds the ten sources the kernel uses and none of the four gz*",
    "sys/contrib/zlib/gzlib.c": "the same",
    "sys/contrib/zlib/gzread.c": "the same",
    "sys/contrib/zlib/gzwrite.c": "the same",

    # And four that the build DOES name. All four are the openzfs SPL
    # include order - its <sys/*.h> shadow FreeBSD's - which
    # sys/conf/kmod.mk's OPENZFS_CFLAGS arranges deliberately and which
    # this sweep reproduces well enough to compile 22 of 24 files in the
    # same directory.
    "sys/contrib/openzfs/module/os/freebsd/spl/spl_vm.c":
        "VM_OBJECT_WUNLOCK undeclared: <vm/vm_object.h> resolved to the "
        "SPL's rather than FreeBSD's",
    "sys/contrib/openzfs/module/os/freebsd/zfs/zfs_ctldir.c":
        "DT_DIR undeclared, from the same <sys/dirent.h> shadowing",
    "sys/contrib/openzfs/module/os/freebsd/zfs/zfs_vnops_os.c":
        "DT_UNKNOWN undeclared, the same",
    "sys/contrib/openzfs/module/zstd/lib/common/xxhash.c":
        "its default allocator calls malloc(size); the kernel's malloc "
        "takes three arguments, and the zfs module supplies "
        "XXH_STATIC_LINKING_ONLY and its own allocator that this sweep "
        "does not reproduce",
    # subr_devmap.c, subr_sfbuf.c and subr_intr.c used to be here, all
    # three for the same reason - "arch-private", "needs machine/intr.h,
    # which amd64 has not". They compile now: analyze.py retries a file
    # that fails under the default against the architecture the build
    # system says can build it, and for these three that is enough. An
    # exemption that turns out to have been a missing flag is exactly
    # what --check-errors' staleness half is for; it named all three.

    # The C start-up's per-architecture IRELATIVE handler. Each is
    # `#include "reloc.c"' inside libc_start1.c, after that file has
    # declared __rela_iplt_start and picked Elf_Rela or Elf_Rel from
    # CRT_IRELOC_RELA/REL. Alone they see no <sys/elf.h> and no
    # <sys/types.h>, which is why the errors are `unknown type name
    # u_int' and an Elf_Addr that parsed as a function declaration.
    # libc_start1.c itself compiles now, and did not before this list
    # was written: the first C every process runs after the run-time
    # linker, never analysed once.
    "lib/libc/csu/amd64/reloc.c":     "#included by libc_start1.c",
    "lib/libc/csu/aarch64/reloc.c":   "#included by libc_start1.c",
    "lib/libc/csu/i386/reloc.c":      "#included by libc_start1.c",
    "lib/libc/csu/powerpc64/reloc.c": "#included by libc_start1.c",
    "lib/libc/csu/riscv/reloc.c":     "#included by libc_start1.c",

    # upstream SoftFloat's own benchmark harness, with a K&R main().
    # softfloat/Makefile.inc's SRCS does not name it and no libc links
    # it; softfloat.txt describes it as the timing program.
    "lib/libc/softfloat/timesoftfloat.c": "upstream's benchmark, not in SRCS",

    # rpcgen output. include/rpcsvc/Makefile runs rpcgen over yp.x,
    # nis.x and key_prot.x during buildworld, and the .h files do not
    # exist in a source tree -- so includes.rpc_headers() runs the same
    # recipe over the same .x, and lib/libc/rpc/getpublickey.c,
    # key_call.c and key_prot_xdr.c compile. Their entries are gone.
    #
    # Worth naming why they lasted this long: the recipe needed rpcgen,
    # and until rpcgen_tool() built usr.bin/rpcgen it was the HOST's.
    # This machine has one and the CI runner does not, so --check-errors
    # called these three stale here and valid there -- the same tree,
    # two verdicts, depending on a package. Building the tool out of the
    # tree removes the question.

    # option-gated: no kernel config in this tree sets these

    # wrong architecture for an amd64 sweep
    "sys/kern/subr_atomic64.c":     "32-bit archs only",

    # 32-bit PowerPC, which no architecture this sweep runs is. The
    # cpu index recovered twelve of the fourteen named powerpc ERRORs
    # by asking what the configs that build a file agree on; this is
    # what was left. `optional aim powerpc' is satisfied by exactly one
    # config, sys/powerpc/conf/GENERIC, whose `machine powerpc powerpc'
    # makes MACHINE_ARCH 32-bit - and moea_pte_change() calls mtsrin(),
    # which sys/powerpc/include/cpufunc.h:80 defines inside
    # `#ifndef __powerpc64__'. -DAIM does not help and neither does
    # -DPOWERPC; the file needs a 32-bit target.
    "sys/powerpc/aim/mmu_oea.c":
        "32-bit AIM, built only by powerpc/GENERIC (MACHINE_ARCH powerpc)",

    # ...and three more that are not translation units at all.
    "sys/powerpc/booke/pmap_32.c":
        "INCLUDED_BY:sys/powerpc/booke/pmap.c",
    "sys/powerpc/booke/pmap_64.c":
        "INCLUDED_BY:sys/powerpc/booke/pmap.c",
    "sys/powerpc/powerpc/elf_common.c":
        "INCLUDED_BY:sys/powerpc/powerpc/elf32_machdep.c",

    # net80211
    "sys/net80211/ieee80211_alq.c": "needs option IEEE80211_ALQ",

    # security
    # audit_dtrace.c used to be here - "needs the opensolaris compat
    # headers, i.e. option KDTRACE_HOOKS". It compiles now: a module's
    # own include flags go ahead of -I$S, as sys/conf/kmod.mk:128 says
    # they do, so opensolaris's headers win where the module asks for
    # them. Another exemption that was a missing flag.
    "sys/hardenedbsd/hbsd_pax_SKEL.c":
        "a template, in no sys/conf/files line - see the note below",

    # NOT an honest entry. Listed so the gate is truthful about what it
    # sees, not so the problem is filed away: this one is a DEFECT.
    #
    # sys/conf/files:5299 builds it under `optional mac_grantbylabel',
    # and it does not compile, because its mac_policy_ops initialiser
    # names .mpo_proc_check_resource and struct mac_policy_ops has no
    # such member - the KPI has _debug, _sched, _signal and _wait. The
    # only two references to the name in the whole tree are this file's
    # own function and this initialiser, and upstream HardenedBSD is
    # identical, so `options mac_grantbylabel' has never built in either
    # tree. docs/security/UB_FINDINGS.md has the analysis; it needs a
    # decision that is not a mechanical fix.
    "sys/security/mac_grantbylabel/mac_grantbylabel.c":
        "BROKEN: registers a MAC entry point that does not exist",

    # Option-gated, found the first time --check-errors was run over the
    # kern shard rather than over lib and sys/dev alone.

    # DTrace's own SDT provider, and it is the one file that cannot
    # survive a decision made deliberately elsewhere. opt_shim() drops
    # KDTRACE_HOOKS on purpose - sys/sys/sdt.h:218 writes every probe as
    # `asm goto(...)', which clang's analyser gives up on, and it cost 85
    # errors of 105 translation units in sys/netinet alone. A probe is a
    # nop sled the kernel patches at run time, so dropping it does not
    # change what the surrounding code computes. It does change what
    # sdt.c computes, because SDT is its whole subject: `struct
    # sdt_tracepoint' is only defined under the option.
    "sys/cddl/dev/sdt/sdt.c":
        "option-gated: opt_shim() drops KDTRACE_HOOKS by design, and "
        "struct sdt_tracepoint is declared only under it",

    # Vendored beside the opensolaris compat layer and named by nothing:
    # not sys/conf/files*, not a module's SRCS, not the dtrace or zfs
    # module (both of which take a .PATH on this directory and build
    # their own files from it). opensolaris_atomic.c, the file beside
    # them that IS built, is in conf/files.powerpc and
    # modules/opensolaris/Makefile.
    "sys/cddl/compat/opensolaris/kern/opensolaris_uio.c":
        "not built: named by no files* or SRCS; uio_t has no definition "
        "left in this tree",
    "sys/cddl/compat/opensolaris/kern/opensolaris_cmn_err.c":
        "not built: named by no files* or SRCS",
    "sys/cddl/compat/opensolaris/kern/opensolaris_vm.c":
        "not built: named by no files* or SRCS",

    # libexec/rtld-elf. The other nine translation units in this
    # directory compiled for the first time when the rtld's own include
    # flags were supplied - see includes.py - and riscv/reloc.c reported
    # a finding immediately. These six are what is left.
    "libexec/rtld-elf/tests/parse_integer_test.c":
        "#includes parse_integer_func.c, which the test Makefile stages",

    # The DPAA ethernet. sys/powerpc/conf/dpaa/config.dpaa is a
    # `makeoptions DPAA_COMPILE_CMD=...' twenty-one -I long, and
    # files.dpaa builds every NCSW source through it; reading that made
    # 55 of these 66 translation units analysable for the first time.
    # These eleven are the remainder, and they are a fourth honest
    # reason:
    #
    #   not built   vendored with the rest of NXP's NetCommSw drop and
    #               named by no files*, no module Makefile and no other
    #               source in the tree. FreeBSD's DPAA port never took
    #               the mEMAC MAC, the MACSEC block, the storage-profile
    #               helper or the frame replicator.
    #
    # If any of them is ever wired into files.dpaa this list is what
    # says so: --check-errors fails on an entry that starts compiling.
    "sys/contrib/ncsw/Peripherals/FM/MAC/fman_crc32.c":
        "not built: no files* or Makefile names it",
    "sys/contrib/ncsw/Peripherals/FM/MAC/fman_memac.c":
        "not built: the mEMAC MAC is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/MAC/fman_memac_mii_acc.c":
        "not built: the mEMAC MAC is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/MAC/memac.c":
        "not built: the mEMAC MAC is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/MAC/memac_mii_acc.c":
        "not built: the mEMAC MAC is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/MACSEC/fm_macsec.c":
        "not built: the MACSEC block is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/MACSEC/fm_macsec_guest.c":
        "not built: the MACSEC block is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/MACSEC/fm_macsec_master.c":
        "not built: the MACSEC block is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/MACSEC/fm_macsec_secy.c":
        "not built: the MACSEC block is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/Pcd/fm_replic.c":
        "not built: the frame replicator is not in files.dpaa",
    "sys/contrib/ncsw/Peripherals/FM/SP/fman_sp.c":
        "not built: the storage-profile helper is not in files.dpaa",

    # libsodium, same fourth reason. sys/conf/files:5142-5200 names the
    # 77 sources the kernel takes - the stream ciphers, the one-time
    # auth, ed25519, AEGIS - and every one of them compiles. These 28
    # are the rest of upstream's library: password hashing, generic
    # hash, secretbox, libsodium's own randomness and its runtime init.
    # They want <assert.h>, <errno.h> and <stdlib.h> because they are
    # not kernel code and were never asked to be. (Its 72 test/default
    # programs are userland outright and are in includes.NOT_KERNEL,
    # with OpenZFS's tests/ and ACPICA's compiler/.)
    "sys/contrib/libsodium/src/libsodium/crypto_aead/aes256gcm/aesni/aead_aes256gcm_aesni.c":
        "not built: sys/conf/files takes AEGIS and chacha20poly1305, not aesni AES-GCM",
    "sys/contrib/libsodium/src/libsodium/crypto_generichash/blake2b/ref/blake2b-ref.c":
        "not built: the kernel has no generichash consumer",
    "sys/contrib/libsodium/src/libsodium/crypto_generichash/blake2b/ref/generichash_blake2b.c":
        "not built: the kernel has no generichash consumer",
    "sys/contrib/libsodium/src/libsodium/crypto_kdf/blake2b/kdf_blake2b.c":
        "not built: the kernel has no libsodium KDF consumer",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-core.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-encoding.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-avx2.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-avx512f.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-ref.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-ssse3.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/argon2.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/pwhash_argon2i.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/argon2/pwhash_argon2id.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/crypto_pwhash.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/scryptsalsa208sha256/nosse/pwhash_scryptsalsa208sha256_nosse.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/scryptsalsa208sha256/pwhash_scryptsalsa208sha256.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/scryptsalsa208sha256/scrypt_platform.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_pwhash/scryptsalsa208sha256/sse/pwhash_scryptsalsa208sha256_sse.c":
        "not built: argon2 and scrypt are userland password hashing",
    "sys/contrib/libsodium/src/libsodium/crypto_secretbox/crypto_secretbox_easy.c":
        "not built: no in-kernel secretbox consumer",
    "sys/contrib/libsodium/src/libsodium/crypto_secretbox/xchacha20poly1305/secretbox_xchacha20poly1305.c":
        "not built: no in-kernel secretbox consumer",
    "sys/contrib/libsodium/src/libsodium/randombytes/nativeclient/randombytes_nativeclient.c":
        "not built: the kernel supplies its own randomness, sys/crypto/libsodium",
    "sys/contrib/libsodium/src/libsodium/randombytes/randombytes.c":
        "not built: the kernel supplies its own randomness, sys/crypto/libsodium",
    "sys/contrib/libsodium/src/libsodium/randombytes/salsa20/randombytes_salsa20_random.c":
        "not built: the kernel supplies its own randomness, sys/crypto/libsodium",
    "sys/contrib/libsodium/src/libsodium/randombytes/sysrandom/randombytes_sysrandom.c":
        "not built: the kernel supplies its own randomness, sys/crypto/libsodium",
    "sys/contrib/libsodium/src/libsodium/sodium/codecs.c":
        "not built: libsodium's own runtime init, allocator and version",
    "sys/contrib/libsodium/src/libsodium/sodium/core.c":
        "not built: libsodium's own runtime init, allocator and version",
    "sys/contrib/libsodium/src/libsodium/sodium/utils.c":
        "not built: libsodium's own runtime init, allocator and version",
    "sys/contrib/libsodium/src/libsodium/sodium/version.c":
        "not built: libsodium's own runtime init, allocator and version",

    # zstd, same reason again, and the one that had been hiding real
    # coverage: sys/conf/files:644-664 writes `compile-with ${ZSTD_C}'
    # WITHOUT quotes, which the files* reader had required. Supplying
    # ZSTD_C's three -I took the in-kernel zstd - zstdio(9) and the ZFS
    # compressor - from 7 OK / 51 ERROR to 27 / 31, and every one of the
    # 27 built sources now compiles. These fifteen are what upstream
    # ships beside them and FreeBSD does not take.
    "sys/contrib/zstd/lib/common/pool.c":
        "not built: the POSIX-thread worker pool, for zstdmt",
    "sys/contrib/zstd/lib/compress/zstdmt_compress.c":
        "not built: multi-threaded compression, userland only",
    "sys/contrib/zstd/lib/deprecated/zbuff_common.c":
        "not built: the deprecated ZBUFF API",
    "sys/contrib/zstd/lib/deprecated/zbuff_compress.c":
        "not built: the deprecated ZBUFF API",
    "sys/contrib/zstd/lib/dictBuilder/cover.c":
        "not built: dictionary training is a userland tool",
    "sys/contrib/zstd/lib/dictBuilder/divsufsort.c":
        "not built: dictionary training is a userland tool",
    "sys/contrib/zstd/lib/dictBuilder/fastcover.c":
        "not built: dictionary training is a userland tool",
    "sys/contrib/zstd/lib/dictBuilder/zdict.c":
        "not built: dictionary training is a userland tool",
    "sys/contrib/zstd/lib/legacy/zstd_v01.c":
        "not built: the v0.1 format decoder, ZSTD_LEGACY_SUPPORT",
    "sys/contrib/zstd/lib/legacy/zstd_v02.c":
        "not built: the v0.2 format decoder, ZSTD_LEGACY_SUPPORT",
    "sys/contrib/zstd/lib/legacy/zstd_v03.c":
        "not built: the v0.3 format decoder, ZSTD_LEGACY_SUPPORT",
    "sys/contrib/zstd/lib/legacy/zstd_v04.c":
        "not built: the v0.4 format decoder, ZSTD_LEGACY_SUPPORT",
    "sys/contrib/zstd/lib/legacy/zstd_v05.c":
        "not built: the v0.5 format decoder, ZSTD_LEGACY_SUPPORT",
    "sys/contrib/zstd/lib/legacy/zstd_v06.c":
        "not built: the v0.6 format decoder, ZSTD_LEGACY_SUPPORT",
    "sys/contrib/zstd/lib/legacy/zstd_v07.c":
        "not built: the v0.7 format decoder, ZSTD_LEGACY_SUPPORT",

    # The vendored Linux wifi drivers. Their module Makefiles say which
    # bus attachments and options a FreeBSD kernel takes -
    # `RTW88_SDIO= 0', `RTW88_USB= 0', `IWLWIFI_DEBUGFS= 0', three
    # `#SRCS+=' lines - and these 37 are what those say no to, plus
    # eight Linux kunit tests that are in no SRCS at all. Reading the
    # rest of those Makefiles (their -D, and the .if blocks that are
    # decidable from a variable set five lines above them) took this
    # scope from 76 ERROR of 239 to 38; -std=gnu17 in place of c17 took
    # it to 38 from 73.
    #
    # There is one honest gap left and it is NOT in this list:
    # sys/contrib/dev/iwlwifi/fw/acpi.c IS built, on any kernel with
    # DEV_ACPI, and needs the -DCONFIG_ACPI that sits inside
    # `.if ${KERN_OPTS:MDEV_ACPI}'. KERN_OPTS is the kernel's own option
    # set and the module index is not per-architecture yet, so that one
    # condition stays undecidable. With the flag it compiles clean; the
    # 20 errors it reports without it are all one missing define.
    "sys/contrib/dev/iwlwifi/fw/debugfs.c":
        "not built: sys/modules/iwlwifi/Makefile sets IWLWIFI_DEBUGFS=0",
    "sys/contrib/dev/iwlwifi/mld/debugfs.c":
        "not built: sys/modules/iwlwifi/Makefile sets IWLWIFI_DEBUGFS=0",
    "sys/contrib/dev/iwlwifi/mvm/debugfs.c":
        "not built: sys/modules/iwlwifi/Makefile sets IWLWIFI_DEBUGFS=0",
    "sys/contrib/dev/iwlwifi/mvm/debugfs-vif.c":
        "not built: sys/modules/iwlwifi/Makefile sets IWLWIFI_DEBUGFS=0",
    "sys/contrib/dev/iwlwifi/fw/uefi.c":
        "not built: commented out of the module's SRCS",
    "sys/contrib/dev/iwlwifi/mld/led.c":
        "not built: commented out of the module's SRCS",
    "sys/contrib/dev/iwlwifi/mvm/led.c":
        "not built: commented out of the module's SRCS",
    "sys/contrib/dev/iwlwifi/mld/tests/agg.c":
        "not built: a Linux kunit test, in no SRCS",
    "sys/contrib/dev/iwlwifi/mld/tests/hcmd.c":
        "not built: a Linux kunit test, in no SRCS",
    "sys/contrib/dev/iwlwifi/mld/tests/link.c":
        "not built: a Linux kunit test, in no SRCS",
    "sys/contrib/dev/iwlwifi/mld/tests/link-selection.c":
        "not built: a Linux kunit test, in no SRCS",
    "sys/contrib/dev/iwlwifi/mld/tests/rx.c":
        "not built: a Linux kunit test, in no SRCS",
    "sys/contrib/dev/iwlwifi/mld/tests/utils.c":
        "not built: a Linux kunit test, in no SRCS",
    "sys/contrib/dev/iwlwifi/mvm/tests/hcmd.c":
        "not built: a Linux kunit test, in no SRCS",
    "sys/contrib/dev/iwlwifi/tests/devinfo.c":
        "not built: a Linux kunit test, in no SRCS",
    "sys/contrib/dev/rtw88/rtw8723cs.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_SDIO=0",
    "sys/contrib/dev/rtw88/rtw8723ds.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_SDIO=0",
    "sys/contrib/dev/rtw88/rtw8821cs.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_SDIO=0",
    "sys/contrib/dev/rtw88/rtw8822bs.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_SDIO=0",
    "sys/contrib/dev/rtw88/rtw8822cs.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_SDIO=0",
    "sys/contrib/dev/rtw88/sdio.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_SDIO=0",
    "sys/contrib/dev/rtw88/rtw8723du.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/rtw8812au.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/rtw8814au.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/rtw8821au.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/rtw8821cu.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/rtw8822bu.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/rtw8822cu.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/rtw88xxa.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/usb.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_USB=0",
    "sys/contrib/dev/rtw88/led.c":
        "not built: sys/modules/rtw88/Makefile sets RTW88_LEDS=0",
    "sys/contrib/dev/rtw89/rtw8851bu.c":
        "not built: sys/modules/rtw89/Makefile sets RTW89_USB=0",
    "sys/contrib/dev/rtw89/rtw8852au.c":
        "not built: sys/modules/rtw89/Makefile sets RTW89_USB=0",
    "sys/contrib/dev/rtw89/rtw8852bu.c":
        "not built: sys/modules/rtw89/Makefile sets RTW89_USB=0",
    "sys/contrib/dev/rtw89/rtw8852cu.c":
        "not built: sys/modules/rtw89/Makefile sets RTW89_USB=0",
    "sys/contrib/dev/rtw89/usb.c":
        "not built: sys/modules/rtw89/Makefile sets RTW89_USB=0",
    "sys/contrib/dev/rtw89/wow.c":
        "not built: sys/modules/rtw89/Makefile sets RTW89_CONFIG_PM=0",

    # Three under sys/contrib/dev that no NOT_BUILT prefix covers,
    # because they sit inside trees that ARE built.
    "sys/contrib/dev/ath/ath_hal/ar9300/ar9300_sim.c":
        "not built: the ar9300 HAL's simulator, in no files* or SRCS",
}


# A whole vendored driver that the module build never enters. Naming
# three hundred files one at a time would be a list that goes stale on
# the next import and says the same sentence three hundred times; naming
# the tree, with the line of the build system that excludes it, says it
# once and stays true.
#
# This is NOT a way to make an ERROR quiet. analyze.py --check-errors
# prints how many translation units each prefix absorbed and fails on a
# prefix that absorbed none, so a driver that starts being built - or one
# whose files all start compiling - shows up as loudly as an unexpected
# ERROR does. The rule for adding one is narrow: the tree must be
# vendored under sys/contrib, and sys/modules/Makefile must not descend
# into its module, which is a fact you can grep for.
NOT_BUILT = {
    # ipfilter's disconnected programs. sbin/ipf/Makefile:3-4 is
    #
    #   SUBDIR=       libipf .WAIT
    #   SUBDIR+=      ipf ipfstat ipmon ipnat ippool
    #
    # plus ipfs under MK_IPFILTER_IPFS, and then :8-9
    #
    #   # XXX Temporarily disconnected.
    #   # SUBDIR+=    ipftest ipresend ipsend
    #
    # so three directories are commented out of the build and ipfsync,
    # ipscan and iplang were never in it. Together with common/lexer.c
    # above, that is every ERROR sbin/ipf has.
    "sbin/ipf/ipftest/":
        "commented out of SUBDIR at sbin/ipf/Makefile:9. NOT_SUBDIR",
    "sbin/ipf/ipsend/":
        "the other half of that commented-out line. It also wants "
        "<sys/stream.h>, <sys/stropts.h> and <net/nit.h>, which are SunOS "
        "headers FreeBSD has never had. NOT_SUBDIR",
    "sbin/ipf/ipfsync/":
        "never in SUBDIR at all - sbin/ipf/Makefile:3-4 lists libipf, ipf, "
        "ipfstat, ipmon, ipnat and ippool, plus ipfs under "
        "MK_IPFILTER_IPFS. NOT_SUBDIR",
    "sys/contrib/dev/mediatek/":
        "MediaTek mt76: sys/modules/mt76 exists and sys/modules/Makefile "
        "does not descend into it. The tree also carries a mt76 newer "
        "than its linuxkpi - mt76.h:2035 calls page_pool_alloc_frag(), "
        "which sys/compat/linuxkpi does not define, and every one of the "
        "66 translation units that includes mt76.h fails on it. The "
        "whole page_pool shim is TODO stubs returning NULL.",
    "sys/contrib/dev/athk/":
        "Qualcomm ath10k/ath11k/ath12k: sys/modules/ath10k, ath11k, "
        "ath12k and athk_common all exist and sys/modules/Makefile "
        "descends into none of them. They want <linux/of_reserved_mem.h> "
        "and <linux/clk.h>, which this linuxkpi does not have.",
    "sys/contrib/dev/broadcom/":
        "Broadcom brcm80211: sys/modules/brcm80211 exists and "
        "sys/modules/Makefile does not descend into it. brcmsmac's own "
        "headers - defs.h, brcmu_utils.h, brcm_hw_ids.h - are named by "
        "no SRCS either.",
    # The ACPI debugger (option ACPI_DEBUGGER, which no config sets) and
    # the AML disassembler (built from usr.sbin/acpi as iasl) used to be
    # here. Both compile now - the option retry supplies the -D their
    # `optional' clause names - so the prefixes absorbed nothing and
    # --check-errors called them stale. The FACT is unchanged: neither is
    # built into a kernel. What changed is that a file this analyser can
    # read no longer needs an entry to explain an ERROR it does not have.

    # The sys/dev half, from sweep 9's first --check-errors over sys/dev.
    #
    # These fourteen are one fact, checked one way: no sys/conf/files*
    # entry and no sys/modules Makefile names ANY source in the directory,
    # so config(8) cannot reach them and no module builds them. Each
    # reason below says the same thing in the form the directory needs it,
    # and test_expected_errors.py recomputes the fact rather than trusting
    # the sentence - the marker it looks for is NOT_NAMED at the end.
    "sys/dev/etherswitch/ar40xx/":
        "the Qualcomm IPQ40xx switch. sys/arm/conf/std.qca:81 has `device "
        "ar40xx_switch' and no files* entry gives that device a source, "
        "so config(8) accepts the line and compiles nothing. NOT_NAMED",
    "sys/dev/qcom_dwc3/": "Qualcomm DWC3 glue. NOT_NAMED",
    "sys/dev/qcom_ess_edma/": "Qualcomm ESS EDMA ethernet. NOT_NAMED",
    "sys/dev/qcom_gcc/":
        "Qualcomm global clock controller. sys/conf/files.arm64 builds "
        "arm64/qualcomm/qcom_gcc.c, which is a different file. NOT_NAMED",
    "sys/dev/qcom_qup/": "Qualcomm QUP SPI/I2C. NOT_NAMED",
    "sys/dev/qcom_rnd/": "Qualcomm RNG. NOT_NAMED",
    "sys/dev/aic7xxx/aicasm/":
        "aicasm is a HOST program - usr.bin's build runs it to generate "
        "aic7xxx_seq.h - so it includes <ctype.h> and <db.h> and is not "
        "kernel code at all. NOT_NAMED",
    "sys/dev/ath/ath_hal/ar5312/":
        "the AR5312 SoC HAL, which no files* list and no module names. "
        "ar5312_attach.c reaches for ah_gpioSetIntr, a member the current "
        "struct ath_hal_private does not have. NOT_NAMED",
    "sys/dev/cfe/": "Broadcom CFE firmware interface. NOT_NAMED",
    "sys/dev/etherswitch/mtkswitch/": "MediaTek switch. NOT_NAMED",
    "sys/dev/hdmi/": "the DWC HDMI FDT glue. NOT_NAMED",
    "sys/dev/pms/RefTisa/tisa/sassata/sas/tgt/":
        "the PMC-Sierra driver's SAS TARGET mode. The initiator half is "
        "built; every source in this directory wants <osenv.h>, which the "
        "tree does not have. NOT_NAMED",
    "sys/dev/pms/freebsd/driver/common/":
        "the same driver's OS-abstraction sources, which agtiapi.c "
        "#includes rather than links against. NOT_NAMED",
    "sys/dev/xen/pcifront/":
        "the Xen PCI frontend, which wants <machine/xen-os.h> - a header "
        "removed when FreeBSD's Xen support was rewritten. NOT_NAMED",

    "sys/cddl/boot/zfs/":
        "the BOOT LOADER's ZFS reader, not the kernel's. "
        "stand/libsa/zfs/zfsimpl.c #includes zfssubr.c, which #includes "
        "the rest of the directory, and stand/libsa/zfs/Makefile.inc is "
        "what names it. Nothing under sys/ compiles any of these as a "
        "translation unit of its own - sys/cddl/boot/zfs/README says so "
        "in as many words. NOT_NAMED",

    "sys/contrib/xz-embedded/userspace/":
        "xz-embedded's own test programs - boottest, buftest, bytetest, "
        "xzminidec - which include <stdio.h> and are built by its own "
        "Makefile, not by this tree. NOT_NAMED",
    "sys/contrib/zlib/test/":
        "zlib's example, infcover and minigzip, the same way. NOT_NAMED",

    # Userland's scratch. Until userland_names.py these could not be
    # distinguished from a coverage gap, because the only authority the
    # inventory had was sys/conf/files* -- which has no opinion about
    # lib/libc and would have called every source in it unbuilt.
    "lib/libc/db/test/":
        "the db package's hand-run drivers - btree.tests/main.c and the "
        "eight programs under hash.tests. There is no Makefile anywhere "
        "under lib/libc/db/test, and lib/libc/db/Makefile.inc names none "
        "of them; they are run by a person with a compiler, which is why "
        "they call random() with no <stdlib.h> and bcopy() with no "
        "<strings.h>. NOT_NAMED",
    "lib/libc/quad/TESTS/":
        "divrem.c and mul.c, which exercise the quad arithmetic support "
        "by hand. lib/libc/quad/Makefile.inc lists the support routines "
        "and not these. NOT_NAMED",
    "lib/libc/regex/grot/":
        "Henry Spencer's own regex test harness, kept with the vendored "
        "engine. lib/libc/regex/Makefile.inc names regcomp.c, regerror.c, "
        "regexec.c and regfree.c, and nothing in grot/. NOT_NAMED",

    "sys/contrib/vchiq/":
        "the Raspberry Pi VCHIQ driver, which no kernel configuration in "
        "this tree can reach - in either direction. sys/conf/files.arm64 "
        "lists all 20 of its sources under `optional vchiq "
        "soc_brcm_bcm2837 fdt' and NO arm64 config declares `device "
        "vchiq', not even NOTES; sys/arm/conf/GENERIC:228 DOES declare it "
        "and sys/conf/files.arm names no vchiq source at all, nor does the "
        "machine-independent files, nor is there a sys/modules/vchiq. And "
        "vchiq_kmod.c:50 includes <machine/fdt.h>, which exists for amd64, "
        "arm, i386 and x86 and not for arm64 - so the architecture whose "
        "files list names it could not compile it even if a config asked. "
        "VCHIQ_UNREACHABLE",

    "sys/dev/mlx5/mlx5_fpga/":
        "the Innova FPGA half of mlx5. Its sources ARE named, but only "
        "inside `.if defined(CONFIG_BUILD_FPGA)' in "
        "sys/modules/mlx5/Makefile, and nothing in the tree defines that "
        "variable - the only three mentions are the three modules testing "
        "it. So the block is dead, includes.py correctly drops the "
        "CFLAGS of a definitely-false block, and these compile without "
        "the -DCONFIG_MLX5_FPGA that mlx5_fpga/core.h puts its whole body "
        "behind. Five of the seven compile cleanly when it is supplied by "
        "hand; mlx5fpga_ipsec.c and mlx5fpga_conn.c do not, and that is "
        "the code having rotted under a dead option rather than anything "
        "about this sweep. DEAD_OPTION:CONFIG_BUILD_FPGA",

    # Whole directories of userland test programs under sys/. Each has
    # its own Makefile and is built by hand; sys/conf/files* names none
    # of them.
    "sys/netpfil/ipfw/test/":
        "dummynet's scheduler test harness, built by its own Makefile",
    "sys/teken/demo/":
        "the teken terminal emulator's ncurses demo",
    "sys/teken/stress/":
        "the teken terminal emulator's fuzz driver",
    "sys/tests/runtest/":
        "the in-kernel test runner's userland front end",
    "sys/tools/syscalls/examples/":
        "worked examples for the syscall generator, compiled by hand",
}



def not_built(rel: str) -> str | None:
    """The NOT_BUILT prefix covering this file, if any."""
    for pre, why in NOT_BUILT.items():
        if rel.startswith(pre):
            return pre
    return None


def classify(errors: set[str]) -> tuple[list[str], list[str]]:
    """(unexpected, stale) for the ERROR set of one run.

    `stale' is only meaningful for files the run actually looked at, so
    the caller passes the scope's whole file set as `errors' being the
    ERROR subset of it - see analyze.py.
    """
    unexpected = sorted(e for e in errors
                        if e not in EXPECTED and not not_built(e))
    return unexpected, []
