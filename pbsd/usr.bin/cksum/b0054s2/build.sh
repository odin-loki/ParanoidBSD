#!/bin/sh
# Build and run the b0054s2 differential test.
# Usage: sh build.sh   (from pbsd/usr.bin/cksum/b0054s2/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

# Pick the module flags this toolchain needs.
if $CXX --version 2>/dev/null | head -n 1 | grep -qi clang; then
	MODFLAGS="-fmodules"
	PRECOMPILE_ONLY=yes
else
	MODFLAGS="-fmodules-ts"
	PRECOMPILE_ONLY=no
fi

rm -rf gcm.cache pcm.cache oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

if [ "$PRECOMPILE_ONLY" = yes ]; then
	mkdir -p pcm.cache
	$CXX $CXXFLAGS $MODFLAGS -x c++-module --precompile port.cppm \
	    -o pcm.cache/pbsd.usr.bin.cksum.b0054s2.pcm
	$CXX $CXXFLAGS $MODFLAGS -c \
	    pcm.cache/pbsd.usr.bin.cksum.b0054s2.pcm -o port.o
	$CXX $CXXFLAGS $MODFLAGS \
	    -fmodule-file=pbsd.usr.bin.cksum.b0054s2=pcm.cache/pbsd.usr.bin.cksum.b0054s2.pcm \
	    -c harness.cpp -o harness.o
else
	$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS $MODFLAGS -o harness harness.o port.o oracle.o

exec ./harness
