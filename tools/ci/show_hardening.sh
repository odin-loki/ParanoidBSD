#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# Print the hardening options the build will actually use, and fail if one
# that should be on is off.
#
# Reading src.opts.mk is not good enough. The options cascade: WITHOUT_TOOLCHAIN
# sets MK_CLANG=no, and src.opts.mk:551 then set MK_SAFESTACK:=no with a plain
# := that no src.conf can override. Switching to the external toolchain turned
# SafeStack off on amd64, where it defaults to on, and nothing said so.
#
# Ask the build system. Two earlier attempts at that got it wrong and are worth
# recording, because both looked like they worked:
#
#   make -V MK_PIE            the top-level Makefile does not include
#                             src.opts.mk, so every src.opts option came back
#                             empty while MK_SSP - which arrives via sys.mk -
#                             answered fine. Read as "three options are off".
#
#   make -f Makefile.inc1 -V  works for the host architecture and errors for
#                             every cross target, and `| tail -1` then captured
#                             "make: stopped in ..." as the value. Read as
#                             "four options are off", on five architectures.
#
# Makefile.inc1 has a target for exactly this. showconfig runs src.opts.mk and
# kern.opts.mk with the right MACHINE and MACHINE_ARCH and prints every MK_.
# One invocation, cross targets included, and it is the tree's own answer
# rather than this script's reconstruction of it.
set -eu

SRC="$(cd "$(dirname "$0")/../.." && pwd)/hbsd/src"
TARGET="${1:-amd64}"
SRCCONF="${2:-}"
CROSS_TOOLCHAIN="${3:-}"

case "$TARGET" in
amd64) TARGET_ARCH=amd64 ;;
arm64) TARGET_ARCH=aarch64 ;;
arm)   TARGET_ARCH=armv7 ;;
powerpc) TARGET_ARCH=powerpc64 ;;
riscv) TARGET_ARCH=riscv64 ;;
*)     TARGET_ARCH="$TARGET" ;;
esac

REQUIRED="PIE RELRO BIND_NOW SSP"
EXPECTED="SAFESTACK CFI BRANCH_PROTECTION RETPOLINE"
INFO="REPRODUCIBLE_BUILD LTOLIB ASAN UBSAN"

CONF="$(mktemp)"
trap 'rm -f "$CONF"' EXIT
if ! ( cd "$SRC" && make showconfig \
        TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
        ${SRCCONF:+SRCCONF="$SRCCONF"} \
        ${CROSS_TOOLCHAIN:+CROSS_TOOLCHAIN="$CROSS_TOOLCHAIN"} ) > "$CONF" 2>/dev/null
then
    echo "FAIL 'make showconfig' failed for $TARGET/$TARGET_ARCH." >&2
    echo "     Nothing below would be trustworthy, so this is an error and" >&2
    echo "     not a list of disabled options." >&2
    exit 1
fi

ask() { awk -F= -v k="MK_$1" '$1==k{print $2; found=1} END{if(!found) print ""}' "$CONF" | tail -1; }

echo "== hardening options, $TARGET/$TARGET_ARCH"
echo "   srcconf=${SRCCONF:-<none>}"
echo "   toolchain=${CROSS_TOOLCHAIN:-<in-tree>}"
echo "   options reported by showconfig: $(grep -c '^MK_' "$CONF" || echo 0)"
echo

fail=0
for o in $REQUIRED; do
    v="$(ask "$o")"
    printf "  %-18s %-4s  (required)\n" "$o" "${v:-?}"
    if [ -z "$v" ]; then
        echo "       ^ showconfig did not report this option at all, which is"
        echo "         a bug in this script or a renamed option, not a"
        echo "         disabled mitigation."
        fail=$((fail + 1))
    elif [ "$v" != "yes" ]; then
        fail=$((fail + 1))
    fi
done
# RETPOLINE is x86-only and BRANCH_PROTECTION aarch64-only by hardware, so
# "no" is the right answer for those elsewhere. Printed, not judged.
for o in $EXPECTED; do
    v="$(ask "$o")"
    note=""
    [ "$v" = "no" ] && note="  <- off"
    printf "  %-18s %-4s  (expected where the architecture supports it)%s\n" \
        "$o" "${v:-?}" "$note"
done
for o in $INFO; do
    printf "  %-18s %-4s\n" "$o" "$(ask "$o" | sed 's/^$/?/')"
done

echo
if [ "$fail" -gt 0 ]; then
    echo "FAIL $fail required hardening option(s) are off."
    exit 1
fi
echo "OK  required hardening options are on."
