#!/bin/sh
#
# PBSD batch b0183 - build and run the differential harness.
#
# Usage: sh build.sh   (from pbsd/bin/cpuset/b0183/)
#
set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}

MODULE=pbsd.bin.cpuset.b0183

rm -rf gcm.cache oracle.o port.o harness.o "$MODULE".pcm harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then feed the BMI in.
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o "$MODULE".pcm
	$CXX $CXXFLAGS -c "$MODULE".pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE"="$MODULE".pcm \
	    -c harness.cpp -o harness.o
	$CXX $CXXFLAGS harness.o port.o oracle.o -o harness
else
	# gcc: -fmodules-ts, the CMI lands in ./gcm.cache automatically.
	$CXX $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
	$CXX $CXXFLAGS -fmodules-ts harness.o port.o oracle.o -o harness
fi

exec ./harness
