#!/bin/sh
#
# Build and run the b0156s4 differential test.
#
#   sh build.sh
#
set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

MODNAME=pbsd.lib.libc.locale.b0156s4

src=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

build=${TMPDIR:-/tmp}/pbsd-b0156s4-build.$$
rm -rf "$build"
mkdir -p "$build"
cd "$build"

$CC $CFLAGS -c "$src/oracle.c" -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module --precompile "$src/port.cppm" -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=port.pcm" \
	    -c "$src/harness.cpp" -o harness.o
else
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c "$src/port.cppm" -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c "$src/harness.cpp" -o harness.o
fi

$CXX $CXXFLAGS -o harness harness.o port.o oracle.o

exec ./harness
