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
KERNCONF="${KERNCONF:-HARDENEDBSD}"
TARGET="${TARGET:-amd64}"
TARGET_ARCH="${TARGET_ARCH:-amd64}"
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

# hbsd/src.conf.pbsd is PBSD's build options - notably
# WITHOUT_MACHDEP_OPTIMIZATIONS, which selects the machine-independent C in
# libc, msun and libmd instead of the hand-written assembly. It existed for
# some time without being passed to anything, so it configured nothing. Set
# SRCCONF= to build with FreeBSD's defaults instead.
SRCCONF="${SRCCONF-$(cd "$(dirname "$0")/../.." && pwd)/hbsd/src.conf.pbsd}"

echo "== PBSD image build"
echo "   src=$SRC"
echo "   stage=$STAGE kernconf=$KERNCONF target=$TARGET/$TARGET_ARCH jobs=$JOBS"
echo "   srcconf=${SRCCONF:-<none, FreeBSD defaults>}"
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
             KERNCONF="$KERNCONF" SRCCONF="$SRCCONF" "$@"
    else
        make -j"$JOBS" TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
             KERNCONF="$KERNCONF" "$@"
    fi
}

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
