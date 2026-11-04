#!/bin/sh
# Build and run the batch b0126 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/libc/stdlib/b0126/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

OUT=./build
rm -rf "$OUT"
mkdir -p "$OUT" "$OUT/gcm.cache"

# Pick the module flags this toolchain understands.
MODFLAGS=""
if $CXX -std=c++23 -fmodules-ts -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules-ts"
elif $CXX -std=c++23 -fmodules -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules"
fi

$CC $CFLAGS -c oracle.c -o "$OUT/oracle.o"

# GCC caches the compiled module interface under gcm.cache in the current
# directory, so keep both C++ compilations in the same place.
$CXX $CXXFLAGS $MODFLAGS -c -x c++ port.cppm -o "$OUT/port.o"
$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o "$OUT/harness.o"

$CXX $CXXFLAGS $MODFLAGS "$OUT/port.o" "$OUT/harness.o" "$OUT/oracle.o" \
    -o "$OUT/harness"

exec "$OUT/harness"
