#!/bin/sh
# Build and run the b0020s4 differential test.
#
# Compiles the C oracle with the C compiler, the C++23 module interface and the
# harness with the C++ compiler, links all three and execs the harness so that
# the harness exit status is this script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS_STD="-std=c11 -O2"
CXXFLAGS_STD="-std=c++23 -O2"
MODNAME=pbsd.lib.libc.sys.b0020s4

rm -rf build gcm.cache
mkdir -p build

"$CC" $CFLAGS_STD -c oracle.c -o build/oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then compile the BMI to an
	# object and point the harness at the BMI.
	"$CXX" $CXXFLAGS_STD -x c++-module --precompile port.cppm \
	    -o build/port.pcm
	"$CXX" $CXXFLAGS_STD -c build/port.pcm -o build/port.o
	"$CXX" $CXXFLAGS_STD -fmodule-file="$MODNAME"=build/port.pcm \
	    -c harness.cpp -o build/harness.o
else
	# gcc: -fmodules-ts, module interface first so the CMI lands in the
	# module cache before the harness imports it.
	MODFLAGS="-fmodules-ts"
	"$CXX" $CXXFLAGS_STD $MODFLAGS -x c++ -c port.cppm -o build/port.o
	"$CXX" $CXXFLAGS_STD $MODFLAGS -c harness.cpp -o build/harness.o
fi

"$CXX" $CXXFLAGS_STD -o build/harness build/harness.o build/port.o \
    build/oracle.o

exec ./build/harness
