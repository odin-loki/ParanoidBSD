#!/bin/sh
# Build and run the PBSD b0146s3 differential test.
# Usage: sh build.sh   (from pbsd/sys/kern/b0146s3/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.sys.kern.b0146s3
RUN=b0146s3_run

rm -rf gcm.cache "$MODULE.pcm" oracle.o port.o harness.o "$RUN"

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 -x c++-module --precompile port.cppm \
	    -o "$MODULE.pcm"
	$CXX -std=c++23 -O2 -c "$MODULE.pcm" -o port.o
	$CXX -std=c++23 -O2 -fmodule-file="$MODULE=$MODULE.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 -O2 oracle.o port.o harness.o -o "$RUN"
else
	$CXX -std=c++23 -O2 -fmodules-ts -Wno-clobbered -x c++ \
	    -c port.cppm -o port.o
	$CXX -std=c++23 -O2 -fmodules-ts -Wno-clobbered \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 -O2 -fmodules-ts oracle.o port.o harness.o -o "$RUN"
fi

exec ./"$RUN"
