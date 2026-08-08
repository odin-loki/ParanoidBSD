#!/bin/sh
#
# PBSD batch b0206 -- build the reference, the port and the differential
# harness, then run it.  The harness exit status is this script's exit status.
#
# Usage: sh build.sh   (from pbsd/sys/compat/freebsd32/b0206/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2 -Wno-sign-compare"
CXXFLAGS="-std=c++23 -O2 -Wno-sign-compare"

# Module flags: GCC uses -fmodules-ts and a gcm.cache directory, clang wants
# precompiled module interfaces.  Detect which one we have.
if $CXX --version 2>&1 | grep -qi clang; then
	MODFLAGS="-fmodules"
	CLANG=yes
else
	MODFLAGS="-fmodules-ts"
	CLANG=no
fi

rm -rf gcm.cache oracle.o port.o harness.o port.pcm harness

$CC $CFLAGS -c oracle.c -o oracle.o

if [ "$CLANG" = yes ]; then
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=pbsd.sys.compat.freebsd32.b0206=port.pcm \
	    -c harness.cpp -o harness.o
else
	$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS oracle.o port.o harness.o -o harness

exec ./harness
