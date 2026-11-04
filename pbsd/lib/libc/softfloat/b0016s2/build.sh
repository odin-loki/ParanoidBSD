#!/bin/sh
#
# Build and run the PBSD b0016s2 differential test.
#
#   sh build.sh
#
# Compiles the C oracle, the C++23 module port and the harness, links them
# together and execs the harness so its exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2 -Wall"
CXXFLAGS="-std=c++23 -O2 -Wall"

MODNAME=pbsd.lib.libc.softfloat.b0016s2

rm -rf gcm.cache oracle.o port.o harness.o port.pcm harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then compile the BMI.
	$CXX $CXXFLAGS -x c++-module port.cppm --precompile -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=port.pcm" -c harness.cpp \
		-o harness.o
	$CXX $CXXFLAGS -o harness harness.o port.o oracle.o
else
	# GCC: -fmodules-ts, the CMI goes through gcm.cache.
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
	$CXX $CXXFLAGS -fmodules-ts -o harness harness.o port.o oracle.o
fi

exec ./harness
