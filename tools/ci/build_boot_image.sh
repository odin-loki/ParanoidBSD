#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Build a bootable PBSD image from hbsd/src.
#
# Run this on FreeBSD. The tree is HardenedBSD-derived and release/ is
# FreeBSD's own release machinery, which assumes a FreeBSD host: makefs,
# mkimg, mtree and a native bmake. Cross-building from Linux via
# tools/build/make.py does work for world and kernel, but the image targets
# are the part that wants the host.
#
# Usage:
#   sh tools/ci/build_boot_image.sh [kernel|world|vm|memstick|iso]
#
# Stages, cheapest first, because each is a real gate on the one after:
#   kernel    buildkernel only        - proves the kernel config compiles
#   world     buildworld buildkernel  - the long one, hours
#   memstick  ... + memstick.img      - bootable USB image
#   iso       ... + disc1.iso         - bootable installer ISO
#   vm        ... + vm.ufs.raw        - a disk image that boots to a login
#
# memstick and iso are the installer: they boot to bsdinstall's menu, which
# is enough to prove the kernel and userland start and is not enough to ask
# the system anything. vm.ufs.raw has a root login on the serial console,
# which is what tools/ci/boot_test.py --run needs - and several open
# questions in this repository (what hardening.pax.mprotect.status really
# defaults to, what is setuid in the built image rather than in the
# makefiles) can only be answered from inside a booted system.
set -eu

STAGE="${1:-kernel}"
SRC="$(cd "$(dirname "$0")/../.." && pwd)/hbsd/src"
OBJ="${OBJ:-/usr/obj/pbsd}"
TARGET="${TARGET:-amd64}"
# Each architecture names its 64-bit variant differently, and getting it wrong
# produces a confusing failure a long way in. Derive it unless told otherwise.
case "$TARGET" in
amd64)   TARGET_ARCH="${TARGET_ARCH:-amd64}" ;;
arm64)   TARGET_ARCH="${TARGET_ARCH:-aarch64}" ;;
arm)     TARGET_ARCH="${TARGET_ARCH:-armv7}" ;;
i386)    TARGET_ARCH="${TARGET_ARCH:-i386}" ;;
powerpc) TARGET_ARCH="${TARGET_ARCH:-powerpc64}" ;;
riscv)   TARGET_ARCH="${TARGET_ARCH:-riscv64}" ;;
*)       TARGET_ARCH="${TARGET_ARCH:-$TARGET}" ;;
esac
# HARDENEDBSD exists for amd64, arm64, i386 and riscv; powerpc calls it
# HARDENEDBSD64 and arm only has board-specific ones. GENERIC exists for all
# six, so it is the default that lets every architecture be built at all.
KERNCONF="${KERNCONF:-GENERIC}"
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

# hbsd/src.conf.pbsd is PBSD's build options - notably
# WITHOUT_MACHDEP_OPTIMIZATIONS, which selects the machine-independent C in
# libc, msun and libmd instead of the hand-written assembly. It existed for
# some time without being passed to anything, so it configured nothing. Set
# SRCCONF= to build with FreeBSD's defaults instead.
REPOROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SRCCONF="${SRCCONF-$REPOROOT/hbsd/src.conf.pbsd}"

# TOOLCHAIN=external builds with the packaged clang instead of building one.
# MK_TOOLCHAIN=no cascades to CLANG, LLD, LLDB and LLVM_BINUTILS, so the world
# stops shipping a compiler and contrib/llvm-project stops being compiled -
# 14,337 files that the running system does not need.
#
# The cost is self-hosting: the installed system cannot rebuild itself without
# installing a compiler first. That is a real property and it is given up
# deliberately - building a compiler in order to build a system that does not
# run one is most of buildworld's time for a benefit PBSD was not using.
# TOOLCHAIN=internal builds it in-tree if a release ever needs to be.
TOOLCHAIN="${TOOLCHAIN:-external}"
# TOOLCHAIN=internal needs sources that are not tracked. Say so here, with the
# command, rather than letting the build discover it several minutes in with a
# missing-file error that does not explain itself.
if [ "$TOOLCHAIN" = "internal" ] && \
   [ ! -d "$(cd "$(dirname "$0")/../.." && pwd)/hbsd/src/contrib/llvm-project/llvm/lib" ]; then
    echo "FAIL TOOLCHAIN=internal, but the LLVM compiler sources are not here." >&2
    echo "     PBSD does not track them - see docs/TOOLCHAIN.md. To fetch:" >&2
    echo "       git clone --depth 1 --branch llvmorg-21.1.8 --filter=blob:none \\" >&2
    echo "           --sparse https://github.com/llvm/llvm-project /tmp/llvm" >&2
    echo "       (cd /tmp/llvm && git sparse-checkout set llvm clang lldb)" >&2
    echo "       cp -a /tmp/llvm/llvm /tmp/llvm/lldb hbsd/src/contrib/llvm-project/" >&2
    echo "       cp -a /tmp/llvm/clang/. hbsd/src/contrib/llvm-project/clang/" >&2
    exit 1
fi
CROSS_TOOLCHAIN=""
EXTRA_SRCCONF=""
if [ "$TOOLCHAIN" = "external" ]; then
    CROSS_TOOLCHAIN="$REPOROOT/hbsd/toolchains/llvm${LLVM_VERSION:-21}.mk"
    EXTRA_SRCCONF="$REPOROOT/hbsd/src.conf.pbsd-external-toolchain"
    [ -f "$CROSS_TOOLCHAIN" ] || { echo "FAIL no $CROSS_TOOLCHAIN" >&2; exit 1; }
    [ -f "$EXTRA_SRCCONF" ] || { echo "FAIL no $EXTRA_SRCCONF" >&2; exit 1; }
    # Two src.conf files, and make takes one. Concatenate into the objdir.
    #
    # SRCCONF may be deliberately EMPTY - that is what src_conf=none means,
    # and line 59 uses ${SRCCONF-...} with a bare `-` precisely so that an
    # explicitly-empty value survives rather than being replaced by the
    # default. `cat "" "$EXTRA_SRCCONF"` then fails with
    #
    #   cat: : No such file or directory
    #
    # and the build stops before it starts. Run 42 - the first bisect this
    # repository ever actually needed - died there in two and a half
    # minutes, and the combination has been broken since the external
    # toolchain was added, because nobody had run src_conf=none with it.
    #
    # The external toolchain's own src.conf is NOT optional even under
    # src_conf=none: it is what turns MK_TOOLCHAIN off, and without it the
    # build tries to compile a compiler that is not there. So it is always
    # in the list; only the PBSD options are dropped.
    mkdir -p "$OBJ"
    if [ -n "$SRCCONF" ]; then
        cat "$SRCCONF" "$EXTRA_SRCCONF" > "$OBJ/src.conf.combined"
    else
        echo "== src_conf is empty: FreeBSD defaults plus the external" \
             "toolchain options only"
        cat "$EXTRA_SRCCONF" > "$OBJ/src.conf.combined"
    fi
    SRCCONF="$OBJ/src.conf.combined"

    # SafeStack and CFI are not just compiler flags: -fsanitize=safe-stack
    # makes clang link libclang_rt.safestack, and MK_CLANG=no means the
    # in-tree lib/libclang_rt is not built, so the archive has to come from
    # the package.
    #
    # Whether clang will find it is not the same question as whether the
    # package ships it, and the first version of this probe asked the wrong
    # one. The llvm21 port builds compiler-rt for its own default triple:
    #
    #   /usr/local/llvm21/lib/clang/21/lib/x86_64-portbld-freebsd15.0/
    #
    # and Makefile.inc1:146 builds the world with
    #
    #   -target x86_64-unknown-freebsd15.1
    #
    # so clang looks in .../x86_64-unknown-freebsd15.1/ and finds nothing.
    # A probe that only asked "does a libclang_rt.safestack exist anywhere
    # under the resource directory" answered yes, left the option on, and
    # the build failed at bin/cat exactly as before - run 11.
    #
    # So the probe asks with the target the build will use. Same derivation
    # as Makefile.inc1: TARGET_ARCH with amd64 spelled x86_64, the ABI
    # (gnueabihf for arm, unknown otherwise), and OS_REVISION out of
    # sys/conf/newvers.sh.
    OS_REVISION="$(awk -F'"' '/^REVISION=/ { print $2 }' \
        "$SRC/sys/conf/newvers.sh")"
    case "$TARGET" in
    arm) TRIPLE_ABI="gnueabihf" ;;
    *)   TRIPLE_ABI="unknown" ;;
    esac
    case "$TARGET_ARCH" in
    amd64) TRIPLE_ARCH="x86_64" ;;
    *)     TRIPLE_ARCH="$TARGET_ARCH" ;;
    esac
    TRIPLE="${TRIPLE_ARCH}-${TRIPLE_ABI}-freebsd${OS_REVISION}"

    XCC="${LOCALBASE:-/usr/local}/bin/clang${LLVM_VERSION:-21}"
    RTDIR="$("$XCC" -target "$TRIPLE" -print-runtime-dir 2>/dev/null || true)"
    RTBASE="$("$XCC" -print-resource-dir 2>/dev/null || true)"
    echo "== external toolchain runtime"
    echo "   clang=$XCC"
    echo "   build target=$TRIPLE"
    echo "   runtime-dir=${RTDIR:-<clang could not say>}"
    if [ -n "$RTBASE" ] && [ -d "$RTBASE/lib" ]; then
        echo "   what the package actually has:"
        ls -1 "$RTBASE/lib" | sed "s|^|     |"
    fi

    # If the archives are under one other triple directory, alias it. The
    # port's triple and the tree's differ in vendor and in the FreeBSD minor
    # version, and a compiler-rt archive is the same either way; the
    # alternative is losing a mitigation to a directory name. Said out loud,
    # because it is a change to the machine rather than to the tree, and it
    # is only ever done inside a throwaway CI VM.
    if [ -n "$RTDIR" ] && [ ! -d "$RTDIR" ] && [ -n "$RTBASE" ]; then
        alt="$(ls -1d "$RTBASE"/lib/*-freebsd* 2>/dev/null | head -1 || true)"
        if [ -n "$alt" ] && [ -d "$alt" ]; then
            echo "   the package built its runtime for $(basename "$alt")"
            echo "   and this build asks for $(basename "$RTDIR"); aliasing"
            mkdir -p "$(dirname "$RTDIR")" 2>/dev/null || true
            ln -sfn "$alt" "$RTDIR" 2>/dev/null \
                && echo "   $RTDIR -> $alt" \
                || echo "   could not create the alias; options will go off"
        fi
    fi

    for rt in safestack cfi; do
        opt="$(echo "$rt" | tr 'a-z' 'A-Z')"
        if [ -n "$RTDIR" ] && [ -e "$RTDIR/libclang_rt.$rt.a" ]; then
            echo "   $opt on: $RTDIR/libclang_rt.$rt.a"
            continue
        fi
        echo "   $opt off: no libclang_rt.$rt.a for $TRIPLE"
        echo "WITHOUT_$opt=YES" >> "$SRCCONF"
    done
fi

echo "== PBSD image build"
echo "   src=$SRC"
echo "   stage=$STAGE kernconf=$KERNCONF target=$TARGET/$TARGET_ARCH jobs=$JOBS"
echo "   srcconf=${SRCCONF:-<none, FreeBSD defaults>}"
echo "   toolchain=$TOOLCHAIN${CROSS_TOOLCHAIN:+ ($CROSS_TOOLCHAIN)}"
uname -a

if [ -n "$SRCCONF" ] && [ ! -f "$SRCCONF" ]; then
    echo "FAIL SRCCONF=$SRCCONF does not exist." >&2
    echo "     Set SRCCONF= (empty) to build with FreeBSD defaults." >&2
    exit 1
fi
if [ ! -f "$SRC/Makefile.inc1" ]; then
    echo "FAIL $SRC does not look like a FreeBSD source tree." >&2
    exit 1
fi
if [ ! -f "$SRC/sys/$TARGET/conf/$KERNCONF" ]; then
    echo "FAIL kernel config $KERNCONF not found for $TARGET." >&2
    echo "     available: $(ls "$SRC/sys/$TARGET/conf" | tr '\n' ' ')" >&2
    exit 1
fi

# The tree shipped with every file mode 100644 and the build runs its own
# scripts directly, so this is a real precondition, not a formality.
if [ ! -x "$SRC/contrib/bmake/configure" ]; then
    echo "FAIL contrib/bmake/configure is not executable." >&2
    echo "     Run: python3 tools/check_exec_bits.py" >&2
    exit 1
fi

mkdir -p "$OBJ"
export MAKEOBJDIRPREFIX="$OBJ"
cd "$SRC"

# KEEP_GOING=1 adds make -k, so a build reports every independent failure
# instead of the first one.
#
# This is worth a flag of its own because of how the architecture matrix has
# actually gone. Six runs found six bugs, one per run, each an hour apart:
# arm's PAX ASLR hid arm's sched_shim, which hid whatever is behind that, and
# i386's identical ASLR bug never appeared at all because a module failed
# first under -j. Under -k a survey run reports the whole list in one pass.
#
# It is off by default because -k also keeps going after a failure that makes
# everything downstream meaningless, and that noise is the wrong thing for a
# release build. Survey with it, gate without it.
KEEP_GOING="${KEEP_GOING:-0}"
[ "$KEEP_GOING" = "1" ] && KFLAG="-k" || KFLAG=""

run_make() {
    echo "--- make $*"
    if [ -n "$SRCCONF" ]; then
        make -j"$JOBS" $KFLAG TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
             KERNCONF="$KERNCONF" SRCCONF="$SRCCONF" \
             ${CROSS_TOOLCHAIN:+CROSS_TOOLCHAIN="$CROSS_TOOLCHAIN"} "$@"
    else
        make -j"$JOBS" $KFLAG TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
             KERNCONF="$KERNCONF" "$@"
    fi
}

# A kernel for an architecture other than the host needs cross-tools first.
# buildworld builds them itself; buildkernel alone does not.
if [ "$TARGET_ARCH" != "$(uname -p 2>/dev/null || echo amd64)" ]; then
    case "$STAGE" in
    kernel) run_make kernel-toolchain ;;
    esac
fi

# The directories PBSD has ported to C++, and only those.
#
# The first version of the check below scanned every .cpp under lib/ and
# failed the run on
#
#   lib/clang/liblldb/LLDBWrapLua.cpp
#
# which is vendor source for lldb, is not a port, and is deliberately not
# built at all: TOOLCHAIN=external sets MK_TOOLCHAIN=no, which cascades to
# MK_CLANG and MK_LLDB. "A .cpp in the tree" and "a file PBSD ported" are
# not the same set, and the tree carries a lot of C++ that was always C++.
#
# So the scopes are named. This list grows as scopes are ported, and it is
# the same boundary the IR oracle ratchets on.
#
# lib/libc joined it with gen/isatty.cpp, the library's first port. Run 32
# built that world green with lib/msun as the only scope, which asserted
# nothing whatever about isatty.o - and a leaf function silently dropped
# from SRCS does not fail a link, it just is not in libc. Symbol.map names
# isatty, and a version script that names a missing symbol is a warning,
# not an error. Exactly the hole this check exists to close.
PORTED_SCOPES="${PORTED_SCOPES:-lib/msun lib/libc}"

# Every ported .cpp must have been compiled.
#
# A green buildworld is not by itself evidence that a port was built. The
# port is a rename plus one edited line in a hand-written SRCS list, and the
# failure mode of getting that line wrong is not a build error - it is a file
# quietly dropping out of SRCS. libm would then be built without it and the
# build would stay green until something tried to call the missing symbol.
#
# So assert it: for each .cpp under a ported scope, the object has to exist
# in the objdir. This is the check that turns "world is green on a tree that
# contains k_cos.cpp" into "k_cos.cpp was compiled".
check_ports_built() {
    _objroot="$OBJ$SRC/$TARGET.$TARGET_ARCH"
    _missing=0
    _found=0
    _dirs=""
    for _scope in $PORTED_SCOPES; do
        [ -d "$SRC/$_scope" ] && _dirs="$_dirs $SRC/$_scope"
    done
    if [ -z "$_dirs" ]; then
        echo "== no ported scopes present ($PORTED_SCOPES)"
        return 0
    fi
    for _cpp in $(find $_dirs -name '*.cpp' 2>/dev/null); do
        _rel="${_cpp#$SRC/}"
        _obj="$(basename "$_cpp" .cpp).o"
        # The object lands in the objdir for the library, not beside the
        # source, and lib32 builds a second copy - one hit is enough.
        if find "$_objroot" -name "$_obj" -print -quit 2>/dev/null | \
                grep -q .; then
            echo "   built: $_rel -> $_obj"
            _found=$((_found + 1))
        else
            echo "FAIL $_rel is in the tree and $_obj is not in the objdir." >&2
            echo "     The rename landed and the SRCS entry did not, so the" >&2
            echo "     library was built without it." >&2
            _missing=$((_missing + 1))
        fi
    done
    if [ "$_missing" -gt 0 ]; then
        return 1
    fi
    echo "== $_found ported .cpp file(s) in $PORTED_SCOPES, all compiled"
}

case "$STAGE" in
kernel)
    run_make buildkernel
    ;;
world)
    run_make buildworld
    check_ports_built
    run_make buildkernel
    ;;
vm|memstick|iso)
    run_make buildworld
    check_ports_built
    run_make buildkernel
    # release/ installs world and kernel into a staging tree, then makefs
    # and mkimg turn that into something bootable.
    cd "$SRC/release"
    case "$STAGE" in
    memstick) run_make memstick.img; OUT="memstick.img" ;;
    iso)      run_make disc1.iso;    OUT="disc1.iso" ;;
    # vm-image is a no-op unless WITH_VMIMAGES is set and non-empty.
    #
    # release/Makefile.vm guards the whole body of the target:
    #
    #   vm-image: ${QEMUTGT}
    #   .if defined(WITH_VMIMAGES) && !empty(WITH_VMIMAGES)
    #           ... mk-vmimage.sh ...
    #   .endif
    #           touch ${.TARGET}
    #
    # so without it the target succeeds in 0.1 seconds, touches a stamp file
    # called `vm-image`, and produces no image. That is exactly what run 14
    # did: `touch vm-image`, then this script's own error path, then a failed
    # job. The build did not break - it was never asked to do anything.
    #
    # VMFORMATS defaults to `vhd vmdk qcow2 raw` and VMFSLIST to `ufs zfs`,
    # and the target loops over the cross product. That is eight images when
    # one is wanted; raw is the only format QEMU needs and ufs is the only
    # filesystem the boot test cares about.
    #
    # The name is ${VMBASE}.${FS}.${FORMAT}, not ${VMBASE}.${FORMAT} - the
    # filesystem is in the middle. `vm.raw` never existed under any setting.
    vm)       run_make vm-image WITH_VMIMAGES=YES \
                       VMFORMATS=raw VMFSLIST=ufs \
                       VMSIZE="${VMSIZE:-6144m}"
              OUT="vm.ufs.raw" ;;
    esac

    # The image is in the OBJECT tree, not the source tree.
    #
    # release/ builds under MAKEOBJDIRPREFIX like everything else, and
    # make-memstick.sh is invoked as
    #
    #   cd disc1-memstick && sh .../make-memstick.sh . ../memstick.img
    #
    # where disc1-memstick is in the objdir, so ../memstick.img is
    # .../amd64.amd64/release/memstick.img. Run 12 built a memstick
    # successfully - makefs and mkimg both finished - and then this script
    # ran `ls "$SRC/release/memstick.img"`, which does not exist, and set -e
    # turned a completed build into a failed job with the boot test skipped.
    # Two months of "it has never booted" and the last step was looking in
    # the wrong directory.
    #
    # Ask make where it built rather than reconstructing the path.
    OBJRELEASE="$(make -V .OBJDIR TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
        2>/dev/null | tail -1)"
    [ -n "$OBJRELEASE" ] && [ -d "$OBJRELEASE" ] || \
        OBJRELEASE="$OBJ$SRC/$TARGET.$TARGET_ARCH/release"

    FOUND=""
    for d in "$OBJRELEASE" "$SRC/release"; do
        if [ -f "$d/$OUT" ]; then FOUND="$d/$OUT"; break; fi
    done
    if [ -z "$FOUND" ]; then
        echo "FAIL the build finished and $OUT is not where it should be." >&2
        echo "     looked in: $OBJRELEASE" >&2
        echo "                $SRC/release" >&2
        echo "     what is in the objdir release directory:" >&2
        ls -l "$OBJRELEASE" 2>&1 | sed 's/^/       /' >&2
        exit 1
    fi

    # Copy it somewhere the workflow does not have to guess about.
    mkdir -p "$REPOROOT/out"
    cp "$FOUND" "$REPOROOT/out/$OUT"

    # Symbol tables for the two binaries a userland hang lands in.
    #
    # Run 43's debugger frame carried rip = 0x3d6201b9fd0, a user address
    # inside pid 1's second executable mapping. ASLR makes that number mean
    # nothing on its own; minus the object's load base it is ELF virtual
    # address 0x5fd0, and that DOES mean something - if the symbols are to
    # hand. They were built an hour earlier and thrown away.
    #
    # `nm -n`, so boot_test.py --symbols can name the function. Two files,
    # a few hundred KB, and the difference between reading the answer and
    # dispatching another fifty-minute run to guess at it.
    # OBJRELEASE came from `make -V .OBJDIR` for release/, so its parent is
    # the object tree top. Derived from a path make itself gave, not
    # reconstructed - which is the mistake this block sits next to.
    _objtop="$(dirname "$OBJRELEASE")"
    for _b in "libexec/rtld-elf/ld-elf.so.1.full" \
              "libexec/rtld-elf/ld-elf.so.1" \
              "sbin/init/init.full" "sbin/init/init"; do
        _p="$_objtop/$_b"
        [ -f "$_p" ] || continue
        _n="$(basename "$_b" .full)"
        [ -f "$REPOROOT/out/$_n.nm" ] && continue
        if nm -n "$_p" > "$REPOROOT/out/$_n.nm" 2>/dev/null &&
           [ -s "$REPOROOT/out/$_n.nm" ]; then
            echo "   symbols: out/$_n.nm ($(wc -l < "$REPOROOT/out/$_n.nm") \
entries, from $_b)"
            # And the disassembly. Run 56's rip resolved to `_rtld+0x10`
            # from a symbol 32 bytes wide, and _rtld() is 534 lines of C -
            # so the nearest-preceding-symbol name was not the function,
            # and there was no way to tell from the artifact. The
            # instruction at the address is not a guess: a load, a store,
            # an indirect call and a ud2 wedge a process in four different
            # ways, and only the disassembly separates them.
            #
            # A few MB of text against another fifty-minute run to find
            # out what one instruction was.
            if objdump -d "$_p" > "$REPOROOT/out/$_n.dis" 2>/dev/null &&
               [ -s "$REPOROOT/out/$_n.dis" ]; then
                echo "   disasm:  out/$_n.dis ($(wc -l \
< "$REPOROOT/out/$_n.dis") lines)"
            else
                rm -f "$REPOROOT/out/$_n.dis"
                echo "   no objdump output for $_b; a userland hang will" \
                     "name a symbol and not an instruction"
            fi
        else
            # A stripped install is the normal case; the .full is the one
            # that carries symbols. Say which was found rather than leaving
            # an empty file that looks like a table with nothing in it.
            rm -f "$REPOROOT/out/$_n.nm"
        fi
    done
    [ -f "$REPOROOT/out/ld-elf.so.1.nm" ] || \
        echo "   no ld-elf.so.1 symbols under $_objtop/libexec/rtld-elf;" \
             "a userland hang will report an ELF vaddr and no name"

    # LOADER_CONF_EXTRA: lines to append to the image's /boot/loader.conf.
    #
    # Setting a variable at the loader's interactive prompt does not reach
    # the kernel. Run 25 typed
    #
    #   set init_path="/rescue/sh:/sbin/init"
    #
    # the loader echoed it back in full - the boot test records the receipt -
    # and the kernel then printed `start_init: trying /sbin/init`. Whatever
    # the reason, a loader-prompt `set` is not a way to configure the kernel,
    # and three runs were spent finding that out.
    #
    # loader.conf demonstrably is: this image mounts root from
    # ufs:/dev/ufs/HardenedBSD_Install, which is vfs.root.mountfrom out of
    # the loader.conf release/ writes. So put the setting where the one that
    # works goes.
    #
    # Done to the built image rather than to release/'s staging tree, so it
    # does not have to fit into that makefile's dependency graph: attach the
    # image with mdconfig, mount the freebsd-ufs partition, append, unmount.
    # The workflow stages it as a file in the workspace; an environment
    # variable still wins if one is set, so this works by hand too.
    if [ -z "${LOADER_CONF_EXTRA:-}" ] && [ -s "$REPOROOT/.loader_conf_extra" ]
    then
        LOADER_CONF_EXTRA="$(cat "$REPOROOT/.loader_conf_extra")"
    fi
    # INIT_SWAP=rescue moves /sbin/init aside so the kernel falls through
    # its COMPILED-IN init_path to /rescue/init.
    #
    # sys/kern/init_main.c:716 has that list as
    #
    #     "/sbin/init:/sbin/oinit:/sbin/init.bak:/rescue/init"
    #
    # and start_init() walks it, taking ENOENT as "try the next one". So
    # renaming one file selects the fourth entry, using the only mechanism
    # in this area that has ever been demonstrated to work - no init_path,
    # no kern_getenv, no loader.conf.
    #
    # It is the control this question has needed since run 20.
    # rescue/rescue/Makefile builds its crunched binary MK_CFI=no MK_PIE=no
    # NO_SHARED=yes (and MK_SAFESTACK=no per program), and `init` is in
    # CRUNCH_PROGS_sbin, so /rescue/init is statically linked with none of
    # PIE, SafeStack, CFI or the rtld. /sbin/init has all four. Run 31 left
    # init running in userland and saying nothing; if the static one talks,
    # the cause is in that set and src_conf=none bisects it, and if the
    # static one is equally silent then it is not those options at all.
    #
    # The moved file is init.pbsd deliberately: oinit and init.bak are the
    # second and third entries of the list above, and either name would put
    # the same dynamic binary straight back into the path.
    if [ -z "${INIT_SWAP:-}" ] && [ -s "$REPOROOT/.init_swap" ]; then
        INIT_SWAP="$(cat "$REPOROOT/.init_swap")"
    fi
    INIT_SWAP="${INIT_SWAP:-none}"
    if [ -n "${LOADER_CONF_EXTRA:-}" ] || [ "$INIT_SWAP" != "none" ]; then
        if [ -n "${LOADER_CONF_EXTRA:-}" ]; then
            echo "== appending to /boot/loader.conf in the image"
            printf '%s\n' "$LOADER_CONF_EXTRA" | tr ';' '\n' | sed 's/^/   /'
        fi
        [ "$INIT_SWAP" = "none" ] || \
            echo "== init swap in the image: $INIT_SWAP"
        MD="$(mdconfig -a -t vnode -f "$REPOROOT/out/$OUT")" || {
            echo "FAIL mdconfig could not attach the image" >&2; exit 1; }
        echo "   attached as /dev/$MD"
        # Pick the UFS filesystem by type, not by number, and handle both
        # schemes - because the memstick is not the one I assumed.
        #
        # The first version looked only for a GPT `freebsd-ufs` partition,
        # which is what the vm image has. The memstick is MBR with a BSD
        # label inside a slice:
        #
        #   =>      1  2302010    md0  MBR  (1.1G)
        #           1   131050  md0s1  efi  (64M)
        #      131051  2170960  md0s2  freebsd  [active]  (1.0G)
        #
        # so there is no freebsd-ufs at the top level at all; the filesystem
        # is md0s2a, one level down. The stub test that passed before this
        # ran used a GPT layout I made up, and the real thing differed - the
        # error path below is what turned that into one clear line instead
        # of a corrupted image.
        PART="$(gpart show -p "$MD" 2>/dev/null | \
            awk '$4 == "freebsd-ufs" { print $3; exit }')"
        if [ -z "$PART" ]; then
            SLICE="$(gpart show -p "$MD" 2>/dev/null | \
                awk '$4 == "freebsd" { print $3; exit }')"
            if [ -n "$SLICE" ]; then
                echo "   MBR: descending into slice $SLICE"
                PART="$(gpart show -p "$SLICE" 2>/dev/null | \
                    awk '$4 == "freebsd-ufs" { print $3; exit }')"
                # A BSD label whose partition table gpart cannot read still
                # has its root filesystem at 'a' by convention.
                [ -n "$PART" ] || PART="${SLICE}a"
            fi
        fi
        if [ -z "$PART" ]; then
            echo "FAIL no UFS filesystem in the image (looked for a GPT" >&2
            echo "     freebsd-ufs, then a freebsd MBR slice):" >&2
            gpart show -p "$MD" >&2 || true
            [ -n "${SLICE:-}" ] && gpart show -p "$SLICE" >&2
            mdconfig -d -u "$MD"; exit 1
        fi
        echo "   UFS filesystem: /dev/$PART"
        MNT="$(mktemp -d)"
        if ! mount "/dev/$PART" "$MNT"; then
            echo "FAIL could not mount /dev/$PART" >&2
            mdconfig -d -u "$MD"; exit 1
        fi
        _bail() {
            echo "FAIL $1" >&2
            umount "$MNT" 2>/dev/null || true
            mdconfig -d -u "$MD"
            exit 1
        }
        if [ "$INIT_SWAP" = "rescue" ]; then
            [ -f "$MNT/sbin/init" ] || _bail "no /sbin/init in the image"
            # The installer media has no /rescue. Run 33 is how I found out.
            #
            # release/Makefile's disc1 target installs the world with
            #
            #     MK_RESCUE=no MK_DICT=no
            #
            # on its installworld line. Every claim run 33 was dispatched on
            # was right about the binary - static, non-PIE, no SafeStack, no
            # CFI, no rtld - and wrong about it being in this image. Fifty-one
            # minutes to find out, at the last step before the boot.
            #
            # test_init_swap.sh could not have caught it. Its stub wrote
            # fakeroot/rescue/init itself and then asserted that same file
            # was still there afterwards: it exercised the swap and
            # manufactured the precondition the swap needs. A stub that
            # supplies what the real thing lacks says nothing about the real
            # thing - the same shape as run 26's invented GPT layout.
            #
            # buildworld does build rescue: src.conf.pbsd sets no
            # WITHOUT_RESCUE, and release/Makefile's base.txz target runs
            # distributeworld into ${.OBJDIR}/dist, which stages the real
            # installed layout - one crunched binary plus a hard link per
            # program name. Only the *.txz are moved out of DISTDIR
            # afterwards, so dist/base/rescue is still there when this runs.
            if [ ! -f "$MNT/rescue/init" ]; then
                RTREE=""
                if [ -n "${RESCUE_DIST:-}" ] && \
                        [ -f "${RESCUE_DIST}/init" ]; then
                    RTREE="$RESCUE_DIST"
                elif [ -n "${OBJRELEASE:-}" ] && \
                        [ -f "${OBJRELEASE}/dist/base/rescue/init" ]; then
                    RTREE="${OBJRELEASE}/dist/base/rescue"
                fi
                [ -n "$RTREE" ] || _bail "no /rescue in the image and no staged rescue tree to install from (looked in ${RESCUE_DIST:-<RESCUE_DIST unset>} and ${OBJRELEASE:-<OBJRELEASE unset>}/dist/base/rescue)"
                echo "   no /rescue in the image - disc1 is built"
                echo "   MK_RESCUE=no - installing it from $RTREE"
                NEED="$(du -sk "$RTREE" | awk '{print $1}')"
                HAVE="$(df -k "$MNT" | awk 'END {print $4}')"
                echo "   /rescue is ${NEED}K; ${HAVE}K free in the image"
                # makefs sizes these images to fit their contents, so there
                # is no promise of room. The distribution tarballs under
                # /usr/freebsd-dist are the installer's payload and this
                # boot test never opens one, so they are what gives way.
                if [ "$HAVE" -lt "$((NEED + 2048))" ]; then
                    echo "   not enough room; removing the installer"
                    echo "   distribution tarballs, which a boot test"
                    echo "   never reads"
                    rm -f "$MNT"/usr/freebsd-dist/*.txz
                    HAVE="$(df -k "$MNT" | awk 'END {print $4}')"
                    echo "   ${HAVE}K free now"
                fi
                mkdir -p "$MNT/rescue"
                # tar and not cp -R, because every name in /rescue is a hard
                # link to one crunched binary of twelve-odd megabytes, and a
                # copy that does not preserve them writes it once per name.
                if ! ( cd "$RTREE" && tar -cf - . ) | \
                        ( cd "$MNT/rescue" && tar -xof - ); then
                    df -k "$MNT" >&2 || true
                    _bail "could not install /rescue into the image"
                fi
                [ -f "$MNT/rescue/init" ] || \
                    _bail "/rescue/init still absent after installing"
                echo "   installed $(ls "$MNT/rescue" | wc -l | tr -d ' ') names into /rescue"
            fi
            # /sbin/init is installed system-immutable. Run 34 got all the
            # way here, installed 150 names into /rescue, and then:
            #
            #   mv: rename .../sbin/init to .../sbin/init.pbsd:
            #       Operation not permitted
            #
            # as root, on a writable filesystem. That is schg: FreeBSD
            # installs init(8) with the system-immutable flag so a running
            # system cannot have it swapped underneath it, which is exactly
            # what this step is doing. release/Makefile has `chflags -R
            # noschg .` in its own beforeclean for the same reason.
            #
            # Cleared only on the one file, and only if chflags is there -
            # a filesystem without flag support fails the mv below with its
            # own message rather than this one.
            if command -v chflags >/dev/null 2>&1; then
                chflags noschg "$MNT/sbin/init" 2>/dev/null || true
            fi
            mv "$MNT/sbin/init" "$MNT/sbin/init.pbsd" || \
                _bail "could not move /sbin/init aside (schg still set?)"
            [ ! -f "$MNT/sbin/init" ] || \
                _bail "/sbin/init is still there after moving it"
            echo "   /sbin/init -> /sbin/init.pbsd; the kernel falls through"
            echo "   to /rescue/init, 4th in the compiled-in list"
            ls -l "$MNT/sbin/init.pbsd" "$MNT/rescue/init" | sed 's/^/     /'
        elif [ "$INIT_SWAP" != "none" ]; then
            _bail "unknown INIT_SWAP '$INIT_SWAP' (none|rescue)"
        fi
        if [ -n "${LOADER_CONF_EXTRA:-}" ] && \
                ! printf '%s\n' "$LOADER_CONF_EXTRA" | tr ';' '\n' \
                >> "$MNT/boot/loader.conf"; then
            df -k "$MNT" >&2 || true
            _bail "could not append to /boot/loader.conf (full?)"
        fi
        # Read it back. makefs sizes these images to fit, so a write that
        # runs out of space is a real possibility, and a setting that
        # silently did not land is the failure this mechanism exists to
        # stop happening.
        for _line in $(printf '%s' "${LOADER_CONF_EXTRA:-}" | tr ';' ' '); do
            if ! grep -qF "$_line" "$MNT/boot/loader.conf"; then
                _bail "'$_line' is not in the file after appending"
            fi
        done
        if [ -n "${LOADER_CONF_EXTRA:-}" ]; then
            echo "   /boot/loader.conf now ends:"
            tail -5 "$MNT/boot/loader.conf" | sed 's/^/     /'
        fi
        umount "$MNT" || { echo "FAIL could not unmount" >&2; \
            mdconfig -d -u "$MD"; exit 1; }
        # Not fatal: set -e is on, and a leftover temp directory is not
        # worth losing a fifty-minute build over.
        rmdir "$MNT" 2>/dev/null || true
        mdconfig -d -u "$MD"
        echo "   detached /dev/$MD"
    fi
    echo "== image: $FOUND"
    echo "== copied to: $REPOROOT/out/$OUT"
    ls -lh "$REPOROOT/out/$OUT"
    ;;
*)
    echo "FAIL unknown stage '$STAGE' (kernel|world|vm|memstick|iso)" >&2
    exit 1
    ;;
esac

echo "== done: $STAGE"
