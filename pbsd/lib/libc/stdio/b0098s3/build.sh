#!/bin/sh
# Build and run the b0098s3 differential harness.
# Usage: sh build.sh   (from pbsd/lib/libc/stdio/b0098s3/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2 -D_GNU_SOURCE}
CXXFLAGS=${CXXFLAGS:--O2 -D_GNU_SOURCE}
MODNAME=pbsd.lib.libc.stdio.b0098s3

rm -rf gcm.cache oracle.o port.o port.pcm harness.o harness

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX -std=c++23 $CXXFLAGS -c port.pcm -o port.o
	$CXX -std=c++23 $CXXFLAGS -fmodule-file="$MODNAME=port.pcm" -c harness.cpp -o harness.o
else
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c harness.cpp -o harness.o
fi

$CXX -std=c++23 $CXXFLAGS harness.o port.o oracle.o -o harness

exec ./harness
