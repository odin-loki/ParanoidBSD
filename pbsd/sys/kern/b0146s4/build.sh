#!/bin/sh
#
# PBSD batch b0146s4: build the oracle, the C++23 module port and the
# differential harness, then run the harness.  The exit status of this script
# is the exit status of the harness.

set -e

dir=$(dirname "$0")
cd "$dir"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.sys.kern.b0146s4

rm -rf gcm.cache b0146s4.harness *.o *.pcm

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 -x c++-module port.cppm --precompile -o port.pcm
	$CXX -std=c++23 -O2 -c port.pcm -o port.o
	$CXX -std=c++23 -O2 -fmodule-file=$MODNAME=port.pcm -c harness.cpp \
	    -o harness.o
	$CXX -std=c++23 -O2 oracle.o port.o harness.o -o b0146s4.harness
else
	$CXX -std=c++23 -fmodules-ts -O2 -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts -O2 -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts -O2 oracle.o port.o harness.o \
	    -o b0146s4.harness
fi

exec ./b0146s4.harness
