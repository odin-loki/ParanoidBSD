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
REQUIRED="PIE RELRO BIND_NOW"
EXPECTED="SAFESTACK RETPOLINE CFI"
INFO="ASAN UBSAN LTOLIB SSP"

ask() {
    ( cd "$SRC" && make -V "MK_$1" \
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
    printf "  %-12s %-4s  (required)\n" "$o" "${v:-?}"
    [ "$v" = "yes" ] || fail=$((fail + 1))
done
for o in $EXPECTED; do
    v="$(ask "$o")"
    note=""
    [ "$v" = "no" ] && note="  <- OFF"
    printf "  %-12s %-4s  (expected on where the architecture supports it)%s\n" \
        "$o" "${v:-?}" "$note"
done
for o in $INFO; do
    v="$(ask "$o")"
    printf "  %-12s %-4s\n" "$o" "${v:-?}"
done

echo
if [ "$fail" -gt 0 ]; then
    echo "FAIL $fail required hardening option(s) are off."
    exit 1
fi
echo "OK  required hardening options are on."
