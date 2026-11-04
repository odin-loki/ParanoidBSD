#!/bin/sh
# Build and run the PBSD batch b0093s3 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/msun/ld80/b0093s3/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

MOD="pbsd.lib.msun.ld80.b0093s3"

rm -rf gcm.cache pcm.cache
rm -f oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | head -n 1 | grep -qi clang; then
	# clang: precompile the interface, then reuse the BMI.
	mkdir -p pcm.cache
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o "pcm.cache/$MOD.pcm"
	$CXX $CXXFLAGS -c "pcm.cache/$MOD.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MOD=pcm.cache/$MOD.pcm" \
	    -c harness.cpp -o harness.o
else
	# gcc: -fmodules-ts (<=14) or -fmodules (>=15); the CMI lands in
	# ./gcm.cache and is picked up automatically by the importer.
	MODFLAG=-fmodules-ts
	if ! $CXX $CXXFLAGS $MODFLAG -x c++ -fsyntax-only /dev/null \
	    >/dev/null 2>&1; then
		MODFLAG=-fmodules
	fi
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o harness harness.o port.o oracle.o -lm

exec ./harness
