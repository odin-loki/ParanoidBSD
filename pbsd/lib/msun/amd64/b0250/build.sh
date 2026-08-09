#!/bin/sh
#
# Build and run the PBSD b0250 differential test.
#
# Usage:  sh build.sh          (from pbsd/lib/msun/amd64/b0250/)

set -e

srcdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$srcdir"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.lib.msun.amd64.b0250

CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23}

builddir=$srcdir/build
rm -rf "$builddir"
mkdir -p "$builddir"
cd "$builddir"

$CC $CFLAGS -c ../oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS --precompile -x c++-module ../port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=$MODNAME=port.pcm -c ../harness.cpp \
	    -o harness.o
else
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c ../port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c ../harness.cpp -o harness.o
fi

$CXX $CXXFLAGS oracle.o port.o harness.o -o harness

exec ./harness
