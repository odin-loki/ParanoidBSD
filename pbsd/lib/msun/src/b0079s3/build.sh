#!/bin/sh
# Build and run the differential test for PBSD batch b0079s3.
# Usage: sh build.sh   (from pbsd/lib/msun/src/b0079s3/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.lib.msun.src.b0079s3

rm -rf gcm.cache b0079s3_run oracle.o port.o harness.o "$MODULE_NAME.pcm"

# The oracle is plain C11; _GNU_SOURCE exposes lgammaf_r.
$CC -std=c11 -D_GNU_SOURCE -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface, then reference the BMI explicitly.
	$CXX -std=c++23 -x c++-module --precompile port.cppm \
	    -o "$MODULE_NAME.pcm"
	$CXX -std=c++23 -c "$MODULE_NAME.pcm" -o port.o
	$CXX -std=c++23 -fmodule-file="$MODULE_NAME=$MODULE_NAME.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 oracle.o port.o harness.o -o b0079s3_run -lm
else
	# gcc: -fmodules-ts, module interfaces need an explicit -x c++.
	$CXX -std=c++23 -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts oracle.o port.o harness.o \
	    -o b0079s3_run -lm
fi

exec ./b0079s3_run
