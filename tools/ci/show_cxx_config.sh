#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
#
# What C++ standard and standard library does the build actually use?
#
# The port's stated target is ISO C++23. Three places disagree about that and
# only make can settle which wins:
#
#   share/mk/bsd.sys.mk:24     CXXSTD?= gnu++17     the tree-wide default
#   sys/conf/kern.mk:364       CXXSTD?= c++23       PBSD's kernel C++ block
#   lib/libc++/Makefile:115    CXXSTD?= c++23       libc++ building itself
#
# All three are ?=, so the first one evaluated wins and the answer depends on
# include order, not on which line looks most specific. The oracle verifies
# ports at -std=c++23; if the build compiles them at gnu++17 then it is
# verifying something the build does not do - the same shape of gap as
# comparing without -fno-math-errno, and wider.
#
# gnu++17 is also not ISO anything. The GNU dialect is what the tree defaults
# to, and "ISO C++23" is the project's word.
set -eu

SRC="$(cd "$(dirname "$0")/../.." && pwd)/hbsd/src"
TARGET="${1:-amd64}"
SRCCONF="${2:-}"

case "$TARGET" in
amd64) TARGET_ARCH=amd64 ;;
arm64) TARGET_ARCH=aarch64 ;;
*)     TARGET_ARCH="$TARGET" ;;
esac

# `make -f Makefile.inc1 -V` errors for cross targets and `| tail -1` then
# reports "make: stopped in ..." as if it were a value. Query in a directory
# that includes the right makefiles, and treat a non-zero exit as unknown
# rather than as an answer.
ask() {
    out="$( ( cd "$1" && make -f "$2" -V "$3" \
        TARGET="$TARGET" TARGET_ARCH="$TARGET_ARCH" \
        MACHINE="$TARGET" MACHINE_ARCH="$TARGET_ARCH" \
        ${SRCCONF:+SRCCONF="$SRCCONF"} 2>/dev/null ) )" || out=""
    case "$out" in
    *"make: stopped"*|*"error"*|"") echo "?" ;;
    *) printf '%s\n' "$out" | tail -1 ;;
    esac
}

echo "== C++ configuration, $TARGET/$TARGET_ARCH"
echo
printf "  world CXXSTD          %s\n" "$(ask "$SRC" Makefile.inc1 CXXSTD)"
printf "  world CSTD            %s\n" "$(ask "$SRC" Makefile.inc1 CSTD)"

if [ -d "$SRC/lib/libc++" ]; then
    printf "  libc++ CXXSTD         %s\n" "$(ask "$SRC/lib/libc++" Makefile CXXSTD)"
    printf "  libc++ built          yes (lib/Makefile SUBDIR, not gated on MK_TOOLCHAIN)\n"
else
    printf "  libc++                ABSENT\n"
fi
for l in libcxxrt libc++experimental libcompiler_rt; do
    [ -d "$SRC/lib/$l" ] && printf "  %-21s present\n" "$l"
done

echo
echo "  The oracle compares ports at -std=c++23. A world CXXSTD of gnu++17"
echo "  means the build does not, and the verification does not describe it."
