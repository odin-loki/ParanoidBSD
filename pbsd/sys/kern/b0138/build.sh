#!/bin/sh
# Build and run the PBSD b0138 differential test.
# Usage: sh build.sh   (from pbsd/sys/kern/b0138/)
#
# Compiles oracle.c as C11, port.cppm + harness.cpp as C++23 modules, links
# the three objects and execs the harness, so the harness exit status is the
# exit status of this script.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.sys.kern.b0138
RUN=./b0138_run

rm -rf gcm.cache oracle.o port.o harness.o "$MODULE.pcm" "$RUN"

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface unit, then feed the BMI to importers.
	CXXFLAGS="-std=c++23 -O2 -Wno-zero-length-array -Wno-deprecated-volatile"
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o "$MODULE.pcm"
	$CXX $CXXFLAGS -c "$MODULE.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE=$MODULE.pcm" -c harness.cpp \
	    -o harness.o
	$CXX $CXXFLAGS oracle.o port.o harness.o -o "$RUN"
else
	# GCC: -fmodules-ts, BMI goes to ./gcm.cache; the interface unit must
	# be compiled before its importers.
	CXXFLAGS="-std=c++23 -O2 -fmodules-ts -Wno-volatile"
	$CXX $CXXFLAGS -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -c harness.cpp -o harness.o
	$CXX $CXXFLAGS oracle.o port.o harness.o -o "$RUN"
fi

exec "$RUN"
