#!/bin/sh
#
# b0055 -- build and run the differential harness.
#
# Usage:  sh build.sh          (from pbsd/lib/libc/amd64/gen/b0055/)
#
# Compiles oracle.c with cc -std=c11 -O2, port.cppm and harness.cpp with
# c++ -std=c++23 plus this toolchain's module flags, links the three objects
# and execs the harness, so the harness exit status is this script's status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}

MODULE=pbsd.lib.libc.amd64.gen.b0055

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

# oracle.c -- the specification, plain C11.
$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then compile its object and
	# hand the .pcm to the importer explicitly.
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE=port.pcm" -c harness.cpp \
	    -o harness.o
else
	# gcc: -fmodules-ts, module interface first so that the CMI lands in
	# gcm.cache before harness.cpp imports it.
	$CXX $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o harness harness.o port.o oracle.o

exec ./harness
