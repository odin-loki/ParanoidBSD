#!/bin/sh
#
# build.sh -- build and run the PBSD batch b0333 differential test.
#
# Run as:  sh build.sh
# The exit code is the harness exit code: 0 when every case matched.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
OUT=b0333_harness

rm -rf gcm.cache port.pcm oracle.o port.o harness.o "$OUT"

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 --precompile -x c++-module port.cppm -o port.pcm
	$CXX -std=c++23 -O2 -c port.pcm -o port.o
	$CXX -std=c++23 -O2 \
	    -fmodule-file=pbsd.lib.libc.gen.b0333=port.pcm \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 -O2 oracle.o port.o harness.o -o "$OUT" -lpthread
else
	$CXX -std=c++23 -fmodules-ts -O2 -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts -O2 -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts -O2 oracle.o port.o harness.o \
	    -o "$OUT" -lpthread
fi

exec ./"$OUT"
