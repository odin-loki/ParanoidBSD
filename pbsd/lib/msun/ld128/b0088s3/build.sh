#!/bin/sh
# Build and run the PBSD batch b0088s3 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/msun/ld128/b0088s3/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

rm -rf gcm.cache pcm.cache oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	mkdir -p pcm.cache
	PCM=pcm.cache/pbsd.lib.msun.ld128.b0088s3.pcm
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm -o "$PCM"
	$CXX $CXXFLAGS -c "$PCM" -o port.o
	$CXX $CXXFLAGS \
	    -fmodule-file=pbsd.lib.msun.ld128.b0088s3="$PCM" \
	    -c harness.cpp -o harness.o
	$CXX $CXXFLAGS -o harness harness.o port.o oracle.o -lm
else
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
	$CXX $CXXFLAGS -fmodules-ts -o harness harness.o port.o oracle.o -lm
fi

exec ./harness
