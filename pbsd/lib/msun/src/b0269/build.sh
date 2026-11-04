#!/bin/sh
# Build and run the differential test for PBSD batch b0269.
# Usage: sh build.sh   (from pbsd/lib/msun/src/b0269/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.lib.msun.src.b0269

rm -rf gcm.cache b0269_run oracle.o port.o harness.o "$MODULE_NAME.pcm"

$CC -std=c11 -D_GNU_SOURCE -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 -x c++-module --precompile port.cppm \
	    -o "$MODULE_NAME.pcm"
	$CXX -std=c++23 -O2 -c "$MODULE_NAME.pcm" -o port.o
	$CXX -std=c++23 -O2 -fmodule-file="$MODULE_NAME=$MODULE_NAME.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 -O2 oracle.o port.o harness.o -o b0269_run -lm
else
	$CXX -std=c++23 -fmodules-ts -x c++ -O2 -c port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts -O2 -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts oracle.o port.o harness.o \
	    -o b0269_run -lm
fi

exec ./b0269_run
