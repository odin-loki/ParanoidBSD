#!/bin/sh
# Build and run the PBSD batch b0088s2 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/msun/ld128/b0088s2/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

MODULE=pbsd.lib.msun.ld128.b0088s2

CFLAGS="-std=c11 -O2 -ffp-contract=off"
CXXFLAGS="-std=c++23 -O2 -ffp-contract=off"

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
	    -o "pcm.cache/$MODULE.pcm"
	$CXX $CXXFLAGS -c "pcm.cache/$MODULE.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE=pcm.cache/$MODULE.pcm" \
	    -c harness.cpp -o harness.o
else
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS $MODFLAG -o harness harness.o port.o oracle.o -lm

exec ./harness
