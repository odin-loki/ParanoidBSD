#!/bin/sh
#
# PBSD batch b0122 -- build the oracle, the C++23 module port and the
# differential harness, then run the harness.  The harness exit status is the
# exit status of this script.
#
# Usage: sh build.sh   (from pbsd/lib/libc/uuid/b0122/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.lib.libc.uuid.b0122

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

# Reference implementation.
$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -i clang >/dev/null 2>&1; then
	# Clang: precompile the interface, then compile the object from it.
	$CXX -std=c++23 -O2 --precompile -x c++-module port.cppm -o port.pcm
	$CXX -std=c++23 -O2 -c port.pcm -o port.o
	$CXX -std=c++23 -O2 -fmodule-file="$MODULE=port.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 -O2 -o harness harness.o port.o oracle.o
else
	# GCC: -fmodules-ts, CMI is written to ./gcm.cache automatically.
	$CXX -std=c++23 -fmodules-ts -O2 -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts -O2 -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts -O2 -o harness harness.o port.o oracle.o
fi

exec ./harness
