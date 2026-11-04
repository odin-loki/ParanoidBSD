#!/bin/sh
#
# PBSD batch b0197 -- build and run the timeout(1) differential harness.
#
# Usage: sh build.sh   (from pbsd/bin/timeout/b0197/)
#
# The harness exit status becomes this script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}

MODULE=pbsd.bin.timeout.b0197

case "$($CXX --version 2>&1)" in
*clang*) FLAVOUR=clang ;;
*)       FLAVOUR=gcc ;;
esac

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC $CFLAGS -c oracle.c -o oracle.o

if [ "$FLAVOUR" = clang ]; then
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=$MODULE=port.pcm -c harness.cpp \
	    -o harness.o
	$CXX $CXXFLAGS oracle.o port.o harness.o -o harness
else
	MODFLAGS="-fmodules-ts"
	$CXX $CXXFLAGS $MODFLAGS -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o harness.o
	$CXX $CXXFLAGS $MODFLAGS oracle.o port.o harness.o -o harness
fi

exec ./harness
