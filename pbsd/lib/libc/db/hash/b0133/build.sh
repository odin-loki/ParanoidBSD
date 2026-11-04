#!/bin/sh
#
# Build and run the b0133 differential test.
#
#   sh build.sh
#
# Compiles oracle.c as C11, port.cppm and harness.cpp as C++23 modules, links
# the three together and execs the harness, so this script exits with the
# harness' exit status: 0 only if every single case matched.

set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

MODNAME=pbsd.lib.libc.db.hash.b0133

src=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

build=${TMPDIR:-/tmp}/pbsd-b0133-build.$$
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
