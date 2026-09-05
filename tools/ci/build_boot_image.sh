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
    mkdir -p "$OBJ"
    cat "$SRCCONF" "$EXTRA_SRCCONF" > "$OBJ/src.conf.combined"
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

# Every ported .cpp must have been compiled.
#
# A green buildworld is not by itself evidence that a port was built. The
# port is a rename plus one edited line in a hand-written SRCS list, and the
# failure mode of getting that line wrong is not a build error - it is a file
# quietly dropping out of SRCS. libm would then be built without it and the
# build would stay green until something tried to call the missing symbol.
#
# So assert it: for each .cpp under a ported directory, the object has to
# exist in the objdir. This is the check that turns "world is green on a tree
# that contains k_cos.cpp" into "k_cos.cpp was compiled".
check_ports_built() {
    _objroot="$OBJ$SRC/$TARGET.$TARGET_ARCH"
    _missing=0
    _found=0
    for _cpp in $(find "$SRC/lib" -name '*.cpp' 2>/dev/null); do
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
    echo "== $_found ported .cpp file(s), all compiled"
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
