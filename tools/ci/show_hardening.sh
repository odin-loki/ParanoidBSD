#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Print the hardening options the build will actually use, and fail if one
# that should be on is off.
#
# Reading src.opts.mk is not good enough. The options cascade: WITHOUT_TOOLCHAIN
# sets MK_CLANG=no, and src.opts.mk:551 then sets MK_SAFESTACK:=no with a plain
# := that no src.conf can override. Switching to the external toolchain turned
# SafeStack off on amd64, where it defaults to on, and nothing said so.
#
# `make -V` asks the build system itself, which is the only answer that counts.
#
# Usage: sh tools/ci/show_hardening.sh [TARGET] [SRCCONF] [CROSS_TOOLCHAIN]
set -eu

SRC="$(cd "$(dirname "$0")/../.." && pwd)/hbsd/src"
TARGET="${1:-amd64}"
SRCCONF="${2:-}"
CROSS_TOOLCHAIN="${3:-}"

case "$TARGET" in
amd64) TARGET_ARCH=amd64 ;;
arm64) TARGET_ARCH=aarch64 ;;
*)     TARGET_ARCH="$TARGET" ;;
esac

# Options that carry hardening. REQUIRED ones fail the check when off.
REQUIRED="PIE RELRO BIND_NOW SSP"
EXPECTED="SAFESTACK CFI BRANCH_PROTECTION RETPOLINE"
INFO="REPRODUCIBLE_BUILD LTOLIB ASAN UBSAN"

# -f Makefile.inc1, not the top-level Makefile. The top-level one is a wrapper
# and does not include src.opts.mk, so MK_PIE and every other src.opts option
# came back empty while MK_SSP - which comes from bsd.opts.mk via sys.mk -
# answered fine. That looked like "three required options are off" and was
# really "this script asked the wrong makefile".
ask() {
    ( cd "$SRC" && make -f Makefile.inc1 -V "MK_$1" \
        TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
        ${SRCCONF:+SRCCONF="$SRCCONF"} \
        ${CROSS_TOOLCHAIN:+CROSS_TOOLCHAIN="$CROSS_TOOLCHAIN"} \
        2>/dev/null ) | tail -1
}

echo "== hardening options, $TARGET/$TARGET_ARCH"
echo "   srcconf=${SRCCONF:-<none>}"
echo "   toolchain=${CROSS_TOOLCHAIN:-<in-tree>}"
echo

fail=0
for o in $REQUIRED; do
    v="$(ask "$o")"
    printf "  %-18s %-4s  (required)\n" "$o" "${v:-?}"
    if [ -z "$v" ]; then
        echo "       ^ empty: make could not answer, which is a bug in this"
        echo "         script or a broken tree, not a disabled option."
        fail=$((fail + 1))
    elif [ "$v" != "yes" ]; then
        fail=$((fail + 1))
    fi
done
# Expected on where the architecture supports it. RETPOLINE is x86-only and
# BRANCH_PROTECTION is aarch64-only by hardware, so OFF is the right answer
# for those elsewhere - it is printed, not judged.
for o in $EXPECTED; do
    v="$(ask "$o")"
    note=""
    [ "$v" = "no" ] && note="  <- off"
    printf "  %-18s %-4s  (expected where the architecture supports it)%s\n" \
        "$o" "${v:-?}" "$note"
done
for o in $INFO; do
    v="$(ask "$o")"
    printf "  %-18s %-4s\n" "$o" "${v:-?}"
done

echo
if [ "$fail" -gt 0 ]; then
    echo "FAIL $fail required hardening option(s) are off."
    exit 1
fi
echo "OK  required hardening options are on."
