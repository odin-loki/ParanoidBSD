#!/bin/sh
# Build and run the differential test for PBSD batch b0161.
# Usage: sh build.sh   (from pbsd/sys/fs/procfs/b0161/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.sys.fs.procfs.b0161

rm -rf gcm.cache b0161_run oracle.o port.o harness.o "$MODULE_NAME.pcm"

$CC -std=c11 -O2 -D_POSIX_C_SOURCE=200809L -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -D_POSIX_C_SOURCE=200809L -x c++-module --precompile port.cppm \
	    -o "$MODULE_NAME.pcm"
	$CXX -std=c++23 -D_POSIX_C_SOURCE=200809L -c "$MODULE_NAME.pcm" -o port.o
	$CXX -std=c++23 -D_POSIX_C_SOURCE=200809L -fmodule-file="$MODULE_NAME=$MODULE_NAME.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 oracle.o port.o harness.o -o b0161_run
else
	$CXX -std=c++23 -D_POSIX_C_SOURCE=200809L -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 -D_POSIX_C_SOURCE=200809L -fmodules-ts -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts oracle.o port.o harness.o -o b0161_run
fi

exec ./b0161_run
