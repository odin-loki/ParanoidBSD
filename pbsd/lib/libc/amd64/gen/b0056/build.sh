#!/bin/sh
#
# build.sh -- build and run the PBSD batch b0056 differential test.
#
# Run as:  sh build.sh
#
# Compiles oracle.c as C11, port.cppm and harness.cpp as C++23 modules,
# links the three together and execs the harness, so the exit status of
# this script is the exit code of the harness.

set -e

srcdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
hbsd_sys=$(CDPATH= cd -- "$srcdir/../../../../../../hbsd/src/sys" && pwd)

: "${CC:=cc}"
: "${CXX:=c++}"

builddir="$srcdir/.build"
mkdir -p "$builddir/x86_inc/x86"
ln -sf "$hbsd_sys/x86/include/x86_ieeefp.h" "$builddir/x86_inc/x86/x86_ieeefp.h"
cd "$builddir"

CFLAGS="-std=c11 -O2 -I$hbsd_sys/amd64/include -I$builddir/x86_inc"
CXXFLAGS="-std=c++23 -O2 -I$hbsd_sys/amd64/include -I$builddir/x86_inc"

modname=pbsd.lib.libc.amd64.gen.b0056

if "$CXX" --version 2>&1 | grep -i -q clang; then
	compiler=clang
else
	compiler=gcc
fi

echo "building batch b0056 with $CC / $CXX ($compiler modules)"

$CC $CFLAGS -c "$srcdir/oracle.c" -o oracle.o

if [ "$compiler" = clang ]; then
	$CXX $CXXFLAGS -x c++-module --precompile "$srcdir/port.cppm" \
	    -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$modname=port.pcm" \
	    -c "$srcdir/harness.cpp" -o harness.o
	$CXX $CXXFLAGS -o harness harness.o port.o oracle.o
else
	rm -rf gcm.cache
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c "$srcdir/port.cppm" -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c "$srcdir/harness.cpp" -o harness.o
	$CXX $CXXFLAGS -fmodules-ts -o harness harness.o port.o oracle.o
fi

exec ./harness
