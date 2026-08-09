#!/bin/sh
# Build and run the PBSD batch b0057 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/msun/src/b0057/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

MODFLAG=""
for f in -fmodules-ts -fmodules ""; do
	if $CXX $CXXFLAGS $f -x c++ -fsyntax-only /dev/null >/dev/null 2>&1; then
		MODFLAG=$f
		break
	fi
done

rm -rf gcm.cache pcm.cache
mkdir -p pcm.cache

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o pcm.cache/pbsd.lib.msun.src.b0057.pcm
	$CXX $CXXFLAGS -c pcm.cache/pbsd.lib.msun.src.b0057.pcm -o port.o
	$CXX $CXXFLAGS \
	    -fmodule-file=pbsd.lib.msun.src.b0057=pcm.cache/pbsd.lib.msun.src.b0057.pcm \
	    -c harness.cpp -o harness.o
else
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS $MODFLAG -o harness harness.o port.o oracle.o -lm

exec ./harness
