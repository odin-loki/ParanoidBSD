#!/bin/sh
#
# Build and run the PBSD b0195 differential test.
#
# Usage: sh build.sh   (from pbsd/bin/chmod/b0195/)
#
# The harness's exit status becomes this script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

MODNAME=pbsd.bin.chmod.b0195

rm -rf gcm.cache
rm -f oracle.o port.o harness.o "$MODNAME.pcm" harness

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 $CXXFLAGS -x c++-module port.cppm --precompile \
	    -o "$MODNAME.pcm"
	$CXX -std=c++23 $CXXFLAGS -c "$MODNAME.pcm" -o port.o
	$CXX -std=c++23 $CXXFLAGS -fmodule-file="$MODNAME=$MODNAME.pcm" \
	    -c harness.cpp -o harness.o
else
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c harness.cpp -o harness.o
fi

$CXX -std=c++23 $CXXFLAGS -o harness harness.o port.o oracle.o

exec ./harness
