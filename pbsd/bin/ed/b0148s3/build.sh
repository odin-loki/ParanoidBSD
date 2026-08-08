#!/bin/sh
# build.sh: compile the oracle, the C++23 module port and the differential
# harness, then run the harness.  Exit status is the harness exit status.
set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODNAME=pbsd.bin.ed.b0148s3

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC $CFLAGS -c oracle.c -o oracle.o

# Module flags differ between the two toolchains that can build this.
if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module port.cppm --precompile -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=$MODNAME=port.pcm -c harness.cpp \
	    -o harness.o
else
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o harness harness.o port.o oracle.o

exec ./harness
