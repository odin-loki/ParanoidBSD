#!/bin/sh
# Build and run the b0105s2 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/string/b0105s2/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
OUT=./b0105s2_test

rm -rf gcm.cache port.pcm port.o oracle.o harness.o "$OUT"

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	# Clang: precompile the module interface, then feed it to the TU that
	# imports it.
	"$CXX" -std=c++23 -O2 -Wno-reserved-module-identifier \
	    --precompile -x c++-module port.cppm -o port.pcm
	"$CXX" -std=c++23 -O2 -c port.pcm -o port.o
	"$CXX" -std=c++23 -O2 \
	    -fmodule-file=pbsd.lib.libc.string.b0105s2=port.pcm \
	    -c harness.cpp -o harness.o
else
	# GCC: -fmodules-ts, module interface compiled first so that the CMI
	# lands in gcm.cache before harness.cpp imports it.
	"$CXX" -std=c++23 -fmodules-ts -O2 -x c++ -c port.cppm -o port.o
	"$CXX" -std=c++23 -fmodules-ts -O2 -c harness.cpp -o harness.o
fi

"$CXX" -std=c++23 -O2 -o "$OUT" harness.o port.o oracle.o

exec "$OUT"
