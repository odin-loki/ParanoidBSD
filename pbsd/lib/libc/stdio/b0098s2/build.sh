#!/bin/sh
# Build and run the b0098s2 differential harness.
# Usage: sh build.sh   (from pbsd/lib/libc/stdio/b0098s2/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.lib.libc.stdio.b0098s2

rm -rf gcm.cache oracle.o port.o port.pcm harness.o harness

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 --precompile -x c++-module port.cppm -o port.pcm
	$CXX -std=c++23 -O2 -c port.pcm -o port.o
	$CXX -std=c++23 -O2 -fmodule-file="$MODNAME=port.pcm" -c harness.cpp -o harness.o
else
	$CXX -std=c++23 -O2 -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 -O2 -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX -std=c++23 -O2 harness.o port.o oracle.o -o harness

exec ./harness
