#!/bin/sh
# Build and run the b0155s1 differential test.
#
# oracle.c is the C specification, port.cppm the C++23 module port, harness.cpp
# the differential driver and the shared btree/mpool environment both sides run
# against.  The harness exit status becomes this script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2 -fno-strict-aliasing"
CXXFLAGS="-std=c++23 -O2 -fno-strict-aliasing"

# GCC needs -fmodules-ts for named modules and does not know the .cppm suffix.
MODFLAGS=""
if $CXX -std=c++23 -fmodules-ts -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules-ts"
	rm -rf gcm.cache
fi

rm -f oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o
$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o port.o
$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o harness.o
$CXX $CXXFLAGS $MODFLAGS port.o harness.o oracle.o -o harness

exec ./harness
