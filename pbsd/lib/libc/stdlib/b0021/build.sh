#!/bin/sh
#
# Build and run the PBSD b0021 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/stdlib/b0021/)
#
# The harness' exit status is this script's exit status.

set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2 -fno-strict-aliasing"
MODULE_NAME=pbsd.lib.libc.stdlib.b0021

cd "$(dirname "$0")"

rm -rf gcm.cache port.pcm oracle.o port.o harness.o harness

"$CC" $CFLAGS -c oracle.c -o oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface, then compile it and the importer.
	"$CXX" $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	"$CXX" $CXXFLAGS -c port.pcm -o port.o
	"$CXX" $CXXFLAGS -fmodule-file="$MODULE_NAME"=port.pcm \
	    -c harness.cpp -o harness.o
else
	# gcc: -fmodules-ts, module interface first so gcm.cache is populated.
	"$CXX" $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	"$CXX" $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

"$CXX" $CXXFLAGS -o harness port.o harness.o oracle.o

exec ./harness
