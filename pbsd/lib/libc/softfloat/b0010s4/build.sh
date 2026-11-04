#!/bin/sh
# PBSD batch b0010s4: build the oracle, the port and the differential harness,
# link them together and run the harness so its exit status is ours.
#
# Usage: sh build.sh   (from pbsd/lib/libc/softfloat/b0010s4/)

set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.lib.libc.softfloat.b0010s4

cd "$(dirname "$0")"

rm -rf gcm.cache harness harness.o oracle.o port.o port.pcm

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi 'clang'; then
	# Clang: precompile the module interface, then compile the BMI to an
	# object and point the importer at it.
	MODFLAGS=""
	$CXX -std=c++23 -O2 -x c++-module --precompile port.cppm -o port.pcm
	$CXX -std=c++23 -O2 -c port.pcm -o port.o
	$CXX -std=c++23 -O2 -fmodule-file="$MODULE_NAME=port.pcm" \
	    -c harness.cpp -o harness.o
else
	# GCC: -fmodules-ts, with the BMI deposited in ./gcm.cache by the
	# module interface compilation and picked up from there by the import.
	MODFLAGS="-fmodules-ts"
	$CXX -std=c++23 -O2 $MODFLAGS -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -O2 $MODFLAGS -c harness.cpp -o harness.o
fi

$CXX -std=c++23 -O2 $MODFLAGS port.o harness.o oracle.o -o harness

exec ./harness
