#!/bin/sh
# Build and run the b0190s2 differential test.
# Usage: sh build.sh   (from pbsd/bin/date/b0190s2/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

rm -f port.o harness.o oracle.o port.pcm harness
rm -rf gcm.cache

if $CXX --version 2>&1 | grep -i -q clang; then
	# Clang: precompile the interface unit, then feed the BMI to the
	# importer and also turn it into an object file.
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=pbsd.bin.date.b0190s2=port.pcm \
	    -c harness.cpp -o harness.o
	MODFLAGS=""
else
	# GCC: -fmodules-ts, BMI goes into ./gcm.cache and is found from
	# there by the importer.
	MODFLAGS="-fmodules-ts"
	$CXX $CXXFLAGS $MODFLAGS -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o harness.o
fi

$CC $CFLAGS -c oracle.c -o oracle.o

$CXX $CXXFLAGS $MODFLAGS port.o harness.o oracle.o -o harness

exec ./harness
