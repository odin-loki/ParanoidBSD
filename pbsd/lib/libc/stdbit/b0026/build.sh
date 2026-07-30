#!/bin/sh
# Build and run the b0026 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/stdbit/b0026/)

set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

cd "$(dirname "$0")"

rm -rf build gcm.cache
mkdir -p build

$CC $CFLAGS -c oracle.c -o build/oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then reuse the BMI.
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o build/port.pcm
	$CXX $CXXFLAGS -c build/port.pcm -o build/port.o
	$CXX $CXXFLAGS \
	    -fmodule-file=pbsd.lib.libc.stdbit.b0026=build/port.pcm \
	    -c harness.cpp -o build/harness.o
else
	# GCC: -fmodules-ts, BMI goes through gcm.cache automatically.
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o build/port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o build/harness.o
fi

$CXX $CXXFLAGS build/harness.o build/port.o build/oracle.o -o build/harness

exec ./build/harness
