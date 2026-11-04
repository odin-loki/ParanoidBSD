#!/bin/sh
# Build and run the b0050 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/stdbit/b0050/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.lib.libc.stdbit.b0050

rm -rf build gcm.cache
mkdir -p build

$CC -std=c11 -O2 -c oracle.c -o build/oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 --precompile -x c++-module port.cppm \
	    -o build/port.pcm
	$CXX -std=c++23 -O2 -c build/port.pcm -o build/port.o
	$CXX -std=c++23 -O2 -fmodule-file="$MODULE=build/port.pcm" \
	    -c harness.cpp -o build/harness.o
else
	$CXX -std=c++23 -fmodules-ts -O2 -x c++ -c port.cppm -o build/port.o
	$CXX -std=c++23 -fmodules-ts -O2 -c harness.cpp -o build/harness.o
fi

$CXX -std=c++23 -O2 -o build/harness \
    build/harness.o build/port.o build/oracle.o

exec ./build/harness
