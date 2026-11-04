#!/bin/sh
#
# Build and run the b0036 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/libc/stdbit/b0036/)

set -e

cd "$(dirname "$0")"

CC="${CC:-cc}"
CXX="${CXX:-c++}"
CFLAGS="${CFLAGS:--O2}"
CXXFLAGS="${CXXFLAGS:--O2}"

MODULE_NAME=pbsd.lib.libc.stdbit.b0036

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then feed the BMI in.
	$CXX -std=c++23 $CXXFLAGS --precompile -x c++-module port.cppm \
	    -o port.pcm
	$CXX -std=c++23 $CXXFLAGS -c port.pcm -o port.o
	$CXX -std=c++23 $CXXFLAGS -fmodule-file="$MODULE_NAME"=port.pcm \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 $CXXFLAGS oracle.o port.o harness.o -o harness
else
	# gcc: -fmodules-ts, BMI goes through gcm.cache.
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS oracle.o port.o harness.o \
	    -o harness
fi

exec ./harness
