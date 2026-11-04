#!/bin/sh
#
# build.sh -- build and run the PBSD b0117s3 differential test.
#
# Usage: sh build.sh            (from pbsd/lib/libc/secure/b0117s3/)
#
# Compiles the C oracle, the C++23 module port and the harness, links them
# together and execs the resulting binary so that its exit status becomes the
# exit status of this script.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:-"-std=c11 -O2"}
CXXFLAGS=${CXXFLAGS:-"-std=c++23 -O2 -D_GNU_SOURCE"}

BUILD=build
MODNAME=pbsd.lib.libc.secure.b0117s3

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
    "$BUILD/oracle.o" -o "$BUILD/b0117s3_test"

exec "$BUILD/b0117s3_test"
