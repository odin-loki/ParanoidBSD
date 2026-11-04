#!/bin/sh
# Build and run the PBSD b0205 differential test.
# Usage: sh build.sh   (from pbsd/sys/fs/fifofs/b0205/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.sys.fs.fifofs.b0205
BIN=./b0205_difftest

rm -rf gcm.cache oracle.o port.o harness.o "$MODULE.pcm" "$BIN"

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then compile it and the
	# importer against the resulting BMI.
	$CXX -std=c++23 -O1 -x c++-module --precompile port.cppm \
	    -o "$MODULE.pcm"
	$CXX -std=c++23 -O1 -c "$MODULE.pcm" -o port.o
	$CXX -std=c++23 -O1 -fmodule-file="$MODULE=$MODULE.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 oracle.o port.o harness.o -o "$BIN"
else
	# gcc: -fmodules-ts, BMI goes to ./gcm.cache and is found by name.
	$CXX -std=c++23 -fmodules-ts -O1 -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts -O1 -c harness.cpp -o harness.o
	$CXX -std=c++23 -fmodules-ts oracle.o port.o harness.o -o "$BIN"
fi

exec "$BIN"
