#!/bin/sh
# Build and run the batch b0034 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/libc/quad/b0034/)
#
# The harness' exit status becomes this script's exit status.

set -eu

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

rm -rf gcm.cache oracle.o port.o harness.o port.pcm b0034_harness

# The oracle is plain C11 and must never be built as C++.
$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface, then turn the BMI into an object.
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS \
	    -fmodule-file=pbsd.lib.libc.quad.b0034=port.pcm \
	    -c harness.cpp -o harness.o
else
	# gcc: -fmodules-ts (gcc 11-14) or -fmodules (gcc 15+); the module
	# interface has to be compiled before its importer so that the BMI
	# lands in gcm.cache first.  .cppm is not a suffix gcc knows, hence
	# the explicit -x c++.
	MODFLAG=-fmodules-ts
	if ! $CXX $MODFLAG -E -x c++ - </dev/null >/dev/null 2>&1; then
		MODFLAG=-fmodules
	fi
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o b0034_harness harness.o port.o oracle.o

exec ./b0034_harness
