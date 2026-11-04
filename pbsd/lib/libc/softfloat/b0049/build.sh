#!/bin/sh
# PBSD migration batch b0049 -- build and run the differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/softfloat/b0049/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

rm -rf gcm.cache port.pcm oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface unit, then feed the BMI back in.
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=pbsd.lib.libc.softfloat.b0049=port.pcm \
	    -c harness.cpp -o harness.o
else
	# gcc: -fmodules-ts, the CMI lands in ./gcm.cache
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o harness oracle.o port.o harness.o

exec ./harness
