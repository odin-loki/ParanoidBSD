#!/bin/sh
# Build and run the PBSD batch b0093s4 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/msun/ld80/b0093s4/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

MODULE=pbsd.lib.msun.ld80.b0093s4

CFLAGS="-std=c11 -O2 -ffp-contract=off"
CXXFLAGS="-std=c++23 -O2 -ffp-contract=off"

rm -rf gcm.cache pcm.cache
mkdir -p pcm.cache

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o "pcm.cache/$MODULE.pcm"
	$CXX $CXXFLAGS -c "pcm.cache/$MODULE.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE=pcm.cache/$MODULE.pcm" \
	    -c harness.cpp -o harness.o
	$CXX $CXXFLAGS -o harness harness.o port.o oracle.o -lm
else
	MODFLAG=-fmodules-ts
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
	$CXX $CXXFLAGS $MODFLAG -o harness harness.o port.o oracle.o -lm
fi

exec ./harness
