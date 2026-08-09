#!/bin/sh
# Build and run the PBSD b0020s2 differential harness.
# Usage: sh build.sh   (from pbsd/lib/libc/sys/b0020s2/)

set -e

cd "$(dirname "$0")"

CC="${CC:-cc}"
CXX="${CXX:-c++}"
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODULE_NAME="pbsd.lib.libc.sys.b0020s2"

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

# Oracle: plain C11.
$CC $CFLAGS -c oracle.c -o oracle.o

# Module interface unit + harness.  Module flags differ per toolchain.
if $CXX --version 2>&1 | head -n 1 | grep -qi 'clang'; then
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE_NAME"=port.pcm \
	    -c harness.cpp -o harness.o
else
	# GCC: -fmodules-ts, .cppm needs an explicit language selection.
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS harness.o port.o oracle.o -o harness

exec ./harness
