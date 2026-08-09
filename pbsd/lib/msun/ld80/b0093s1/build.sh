#!/bin/sh
# Build and run the PBSD batch b0093s1 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/msun/ld80/b0093s1/)
#
# Compiles the unmodified C oracle with cc -std=c11 -O2, the C++23 module
# interface and the harness with c++ -std=c++23, links the three together
# and execs the harness, so the harness exit code is this script's exit
# code.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

MODULE=pbsd.lib.msun.ld80.b0093s1

rm -rf gcm.cache pcm.cache
rm -f oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface, then compile the .pcm to an object.
	mkdir -p pcm.cache
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o "pcm.cache/$MODULE.pcm"
	$CXX $CXXFLAGS -c "pcm.cache/$MODULE.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE=pcm.cache/$MODULE.pcm" \
	    -c harness.cpp -o harness.o
else
	# gcc: -fmodules-ts, the CMI lands in ./gcm.cache and is picked up
	# by the import in harness.cpp.
	MODFLAG=-fmodules-ts
	if ! $CXX $CXXFLAGS $MODFLAG -x c++ -fsyntax-only /dev/null \
	    >/dev/null 2>&1; then
		MODFLAG=-fmodules
	fi
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
	CXXFLAGS="$CXXFLAGS $MODFLAG"
fi

$CXX $CXXFLAGS -o harness harness.o port.o oracle.o -lm

exec ./harness
