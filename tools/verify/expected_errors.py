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
    # nis.x and key_prot.x during buildworld; the .h files do not exist
    # in a source tree and there is nothing to point an -I at.
    "lib/libc/yp/xdryp.c":            "wants rpcsvc/yp.h, generated by rpcgen",
    "lib/libc/yp/yplib.c":            "wants rpcsvc/yp.h, generated by rpcgen",
    "lib/libc/rpc/getpublickey.c":    "wants rpc/key_prot.h, from rpcgen",
    "lib/libc/rpc/key_call.c":        "wants rpc/key_prot.h, from rpcgen",
    "lib/libc/rpc/auth_des.c":        "wants rpcsvc/nis.h, from rpcgen",
    "lib/libc/rpc/auth_time.c":       "wants rpcsvc/nis.h, from rpcgen",
    "lib/libc/rpc/key_prot_xdr.c":    "wants rpc/key_prot.h, from rpcgen",
    "lib/libc/rpc/crypt_client.c":    "wants rpcsvc/crypt.h, from rpcgen",

    # option-gated: no kernel config in this tree sets these
    "sys/kern/subr_asan.c":         "needs option KASAN",
    "sys/kern/subr_csan.c":         "needs option KCSAN",
    "sys/kern/subr_msan.c":         "needs option KMSAN",
    "sys/kern/kern_tslog.c":        "needs option TSLOG",
    "sys/kern/kern_poll.c":         "needs option DEVICE_POLLING",
    "sys/kern/tty_compat.c":        "needs option COMPAT_43TTY",

    # wrong architecture for an amd64 sweep
    "sys/kern/subr_atomic64.c":     "32-bit archs only",
    "sys/powerpc/ofw/ofw_machdep.c": "wants powerpc's <fdt.h>, absent on amd64",

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
    "sys/netinet/tcp_stats.c":
        "option-gated: `optional stats inet | stats inet6', and no "
        "kernel config in this tree sets STATS",
    "sys/vm/memguard.c":
        "option-gated: `optional DEBUG_MEMGUARD'",

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
    "libexec/rtld-elf/aarch64/reloc.c":   "another architecture's relocations",
    "libexec/rtld-elf/arm/reloc.c":       "another architecture's relocations",
    "libexec/rtld-elf/powerpc/reloc.c":   "another architecture's relocations",
    "libexec/rtld-elf/powerpc64/reloc.c": "another architecture's relocations",
    "libexec/rtld-elf/tests/parse_integer_test.c":
        "#includes parse_integer_func.c, which the test Makefile stages",
    "libexec/rtld-elf/tests/target/target.c":
        "#includes pythagoras.h from a sibling test library",

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
    "sys/contrib/dev/acpica/components/resources/rsdump.c":
        "option-gated: sys/conf/files:545 is `optional acpi acpi_debug'",
    "sys/contrib/dev/acpica/os_specific/service_layers/osgendbg.c":
        "option-gated: sys/conf/files:601 is `optional acpi acpi_debug'",
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
    "sys/contrib/dev/acpica/components/debugger/":
        "the ACPI debugger, option ACPI_DEBUGGER, which no config sets",
    "sys/contrib/dev/acpica/components/disassembler/":
        "the AML disassembler, built from usr.sbin/acpi as iasl",

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
