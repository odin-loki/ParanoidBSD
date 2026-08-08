#!/bin/sh
#
# Build and run the b0163 differential test.
#
#	sh build.sh
#
# Exits with the harness' exit status: 0 if every case matched, 1 otherwise.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}
MODULE=pbsd.bin.cat.b0163

rm -rf gcm.cache port.pcm oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface, then feed the BMI to the importer.
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=$MODULE=port.pcm -c harness.cpp \
	    -o harness.o
else
	# gcc: -fmodules-ts, and .cppm needs an explicit language selection.
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o harness oracle.o port.o harness.o

exec ./harness
