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
#   sh tools/ci/build_boot_image.sh [kernel|world|memstick|iso]
#
# Stages, cheapest first, because each is a real gate on the one after:
#   kernel    buildkernel only        - proves the kernel config compiles
#   world     buildworld buildkernel  - the long one, hours
#   memstick  ... + memstick.img      - bootable USB image
#   iso       ... + disc1.iso         - bootable installer ISO
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
    # the package. Whether it does is a property of how the port was built,
    # not of this tree, and run 9 found out the expensive way - a link error
    # in bin/cat two thirds of the way through buildworld, after 16 minutes.
    #
    # So ask first, print what is there either way, and turn the option off
    # rather than failing. show_hardening.sh reports SAFESTACK and CFI, so an
    # option turned off here is visible in the run rather than silent.
    XCC="${LOCALBASE:-/usr/local}/bin/clang${LLVM_VERSION:-21}"
    RTBASE="$("$XCC" -print-resource-dir 2>/dev/null || true)"
    echo "== external toolchain runtime"
    echo "   clang=$XCC"
    echo "   resource-dir=${RTBASE:-<clang could not say>}"
    if [ -n "$RTBASE" ] && [ -d "$RTBASE/lib" ]; then
        find "$RTBASE/lib" -name 'libclang_rt.*' | sed "s|^$RTBASE/lib/|     |"
    else
        echo "     no runtime directory at all"
    fi
    for rt in safestack cfi; do
        opt="$(echo "$rt" | tr 'a-z' 'A-Z')"
        if [ -n "$RTBASE" ] && \
           [ -n "$(find "$RTBASE/lib" -name "libclang_rt.$rt*" -print 2>/dev/null | head -1)" ]
        then
            continue
        fi
        echo "   $opt off: this toolchain ships no libclang_rt.$rt"
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

run_make() {
    echo "--- make $*"
    if [ -n "$SRCCONF" ]; then
        make -j"$JOBS" TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
             KERNCONF="$KERNCONF" SRCCONF="$SRCCONF" \
             ${CROSS_TOOLCHAIN:+CROSS_TOOLCHAIN="$CROSS_TOOLCHAIN"} "$@"
    else
        make -j"$JOBS" TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
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

case "$STAGE" in
kernel)
    run_make buildkernel
    ;;
world)
    run_make buildworld
    run_make buildkernel
    ;;
memstick|iso)
    run_make buildworld
    run_make buildkernel
    # release/ installs world and kernel into a staging tree, then makefs
    # and mkimg turn that into something bootable.
    cd "$SRC/release"
    case "$STAGE" in
    memstick) run_make memstick.img; OUT="memstick.img" ;;
    iso)      run_make disc1.iso;    OUT="disc1.iso" ;;
    esac
    echo "== image: $SRC/release/$OUT"
    ls -lh "$SRC/release/$OUT"
    ;;
*)
    echo "FAIL unknown stage '$STAGE' (kernel|world|memstick|iso)" >&2
    exit 1
    ;;
esac

echo "== done: $STAGE"
