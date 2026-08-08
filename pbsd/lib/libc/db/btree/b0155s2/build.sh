#!/bin/sh
# Build and run the b0155s2 differential test.
# Usage: sh build.sh     (from pbsd/lib/libc/db/btree/b0155s2/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

OUT=./build
rm -rf "$OUT" gcm.cache
mkdir -p "$OUT"

# Named-module support: GCC spells it -fmodules-ts, clang -fmodules (with an
# explicit precompiled interface).  Pick whichever the toolchain accepts.
MODFLAGS=
if $CXX $CXXFLAGS -fmodules-ts -x c++ -E /dev/null >/dev/null 2>&1; then
	MODFLAGS=-fmodules-ts
elif $CXX $CXXFLAGS -fmodules -x c++ -E /dev/null >/dev/null 2>&1; then
	MODFLAGS=-fmodules
fi

$CC $CFLAGS -c oracle.c -o "$OUT/oracle.o"

# The module interface must be compiled before anything that imports it so the
# module mapper finds the compiled interface in ./gcm.cache.
$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o "$OUT/port.o"
$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o "$OUT/harness.o"

$CXX $CXXFLAGS $MODFLAGS "$OUT/port.o" "$OUT/harness.o" "$OUT/oracle.o" \
    -o "$OUT/harness"

exec "$OUT/harness"
