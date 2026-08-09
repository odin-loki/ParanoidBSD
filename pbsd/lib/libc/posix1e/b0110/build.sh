#!/bin/sh
#
# build.sh -- build and run the PBSD b0110 differential harness.
#
# Compiles the untouched-behaviour oracle as C11, the port and the harness as
# C++23 modules, links the three together and execs the harness so that its
# exit status becomes the exit status of this script.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

rm -rf gcm.cache port.pcm oracle.o port.o harness.o harness

$CC -std=c11 -O2 -c oracle.c -o oracle.o

# GCC spells module support -fmodules-ts; clang wants an explicit precompile
# of the module interface unit.  Try GCC's form first and fall back.
if $CXX -std=c++23 -fmodules-ts -O2 -x c++ -c port.cppm -o port.o \
    2>modflags.log; then
	MODFLAGS=-fmodules-ts
	$CXX -std=c++23 $MODFLAGS -O2 -c harness.cpp -o harness.o
	$CXX -std=c++23 $MODFLAGS -o harness port.o harness.o oracle.o
elif $CXX -std=c++23 -O2 -x c++-module --precompile port.cppm -o port.pcm \
    2>>modflags.log; then
	$CXX -std=c++23 -O2 -c port.pcm -o port.o
	$CXX -std=c++23 -O2 \
	    -fmodule-file=pbsd.lib.libc.posix1e.b0110=port.pcm \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 -o harness port.o harness.o oracle.o
else
	echo "build.sh: no working C++23 module flags for $CXX" >&2
	cat modflags.log >&2
	exit 1
fi

rm -f modflags.log

exec ./harness
