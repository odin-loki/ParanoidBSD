#!/bin/sh
# Build and run the PBSD batch b0029s2 differential test.
# Usage: sh build.sh   (from pbsd/sys/security/mac_veriexec/b0029s2/)

set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
BUILD=.build

rm -rf "$BUILD" gcm.cache
mkdir -p "$BUILD"

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

MODFLAGS=""
if $CXX -std=c++23 -fmodules-ts -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules-ts"
elif $CXX -std=c++23 -fmodules -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules"
fi

$CC $CFLAGS -c oracle.c -o "$BUILD/oracle.o"

$CXX $CXXFLAGS $MODFLAGS -c -x c++ port.cppm -o "$BUILD/port.o"
$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o "$BUILD/harness.o"

$CXX $CXXFLAGS $MODFLAGS -o "$BUILD/harness" \
	"$BUILD/harness.o" "$BUILD/port.o" "$BUILD/oracle.o"

exec "$BUILD/harness"
