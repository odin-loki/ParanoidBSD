#!/bin/sh
#
# build.sh -- build and run the PBSD batch b0222 differential test.
#
# Run as:  sh build.sh
#
# Compiles oracle.c as C11, port.cppm and harness.cpp as C++23 modules,
# links the three together and execs the harness, so the exit status of
# this script is the exit status of the harness.

set -e

srcdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

: "${CC:=cc}"
: "${CXX:=c++}"

builddir="$srcdir/.build"
mkdir -p "$builddir"
cd "$builddir"

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
LDFLAGS="-Wl,--wrap=malloc -Wl,--wrap=exit -Wl,--wrap=abort"

modname=pbsd.lib.libc.aarch64.gen.b0222

if "$CXX" --version 2>&1 | grep -i -q clang; then
	compiler=clang
else
	compiler=gcc
fi

echo "building batch b0222 with $CC / $CXX ($compiler modules)"

$CC $CFLAGS -c "$srcdir/oracle.c" -o oracle.o

if [ "$compiler" = clang ]; then
	$CXX $CXXFLAGS -x c++-module --precompile "$srcdir/port.cppm" \
	    -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$modname=port.pcm" \
	    -c "$srcdir/harness.cpp" -o harness.o
	$CXX $CXXFLAGS $LDFLAGS -o harness harness.o port.o oracle.o
else
	rm -rf gcm.cache
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c "$srcdir/port.cppm" -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c "$srcdir/harness.cpp" -o harness.o
	$CXX $CXXFLAGS -fmodules-ts $LDFLAGS -o harness harness.o port.o oracle.o
fi

exec ./harness
