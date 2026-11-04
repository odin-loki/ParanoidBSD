#!/bin/sh
#
# build.sh -- build and run the PBSD batch b0320 differential test.
#
# Compiles oracle.c as C11, port.cppm and harness.cpp as C++23 (with whichever
# named-module flags the local toolchain wants), links all three, and execs the
# harness so its exit code is this script's exit code.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODNAME=pbsd.lib.libc.posix1e.b0320
OUT=b0320_harness

rm -rf gcm.cache "$OUT" oracle.o port.o harness.o port.pcm

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=port.pcm" -c harness.cpp \
	    -o harness.o
else
	$CXX $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o "$OUT" harness.o port.o oracle.o \
    -Wl,--wrap=posix_memalign

exec ./"$OUT"
