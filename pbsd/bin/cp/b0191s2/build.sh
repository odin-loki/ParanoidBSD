#!/bin/sh
# Build and run the PBSD b0191s2 differential harness.
# Usage: sh build.sh   (from pbsd/bin/cp/b0191s2/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

# Work out which module spelling this C++ toolchain wants.
MODFLAGS=""
if $CXX -std=c++23 -fmodules-ts -x c++ -fsyntax-only /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules-ts"
elif $CXX -std=c++23 -fmodules -x c++ -fsyntax-only /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules"
fi

rm -rf gcm.cache
rm -f oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

# The module interface unit must be compiled (and its BMI emitted) first.
$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o port.o
$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o harness.o

$CXX $CXXFLAGS $MODFLAGS oracle.o port.o harness.o -o harness

exec ./harness
