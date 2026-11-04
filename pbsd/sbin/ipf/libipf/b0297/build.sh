#!/bin/sh
#
# build.sh -- build and run the PBSD b0297 differential test.
#
# Usage: sh build.sh		(from pbsd/sbin/ipf/libipf/b0297/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:-"-std=c11 -O2"}
CXXFLAGS=${CXXFLAGS:-"-std=c++23 -O2"}

BUILD=build
MODNAME=pbsd.sbin.ipf.libipf.b0297
TEST=$BUILD/b0297_test

rm -rf "$BUILD" gcm.cache
mkdir -p "$BUILD"

$CC $CFLAGS -c oracle.c -o "$BUILD/oracle.o"

MODFLAGS=""
if $CXX --version 2>&1 | grep -qi 'clang'; then
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm \
	    -o "$BUILD/port.pcm"
	$CXX $CXXFLAGS -c "$BUILD/port.pcm" -o "$BUILD/port.o"
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=$BUILD/port.pcm" \
	    -c harness.cpp -o "$BUILD/harness.o"
else
	MODFLAGS="-fmodules-ts"
	$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o "$BUILD/port.o"
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o "$BUILD/harness.o"
fi

$CXX $CXXFLAGS $MODFLAGS "$BUILD/port.o" "$BUILD/harness.o" \
    "$BUILD/oracle.o" \
    -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=ioctl \
    -o "$TEST"

exec "$TEST"
