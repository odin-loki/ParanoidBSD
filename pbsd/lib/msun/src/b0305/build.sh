#!/bin/sh
# Build and run the differential test for PBSD batch b0305.
# Usage: sh build.sh   (from pbsd/lib/msun/src/b0305/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.lib.msun.src.b0305

# lrint manipulates the floating-point environment via rint(); both sides
# need -frounding-math and -fno-builtin-rint so GCC does not replace
# (long)rint(x) with a round-to-nearest builtin under -O2.
FENVFLAGS='-frounding-math -fno-builtin-rint'

rm -rf gcm.cache b0305_run oracle.o port.o harness.o "$MODULE_NAME.pcm"

$CC -std=c11 -D_GNU_SOURCE -O2 $FENVFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 $FENVFLAGS -x c++-module --precompile port.cppm \
	    -o "$MODULE_NAME.pcm"
	$CXX -std=c++23 $FENVFLAGS -c "$MODULE_NAME.pcm" -o port.o
	$CXX -std=c++23 $FENVFLAGS -fmodule-file="$MODULE_NAME=$MODULE_NAME.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 oracle.o port.o harness.o -o b0305_run -lm
else
	$CXX -std=c++23 $FENVFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 $FENVFLAGS -fmodules-ts -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts oracle.o port.o harness.o \
	    -o b0305_run -lm
fi

exec ./b0305_run
