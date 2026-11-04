#!/bin/sh
# Build and run the PBSD b0010s1 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/softfloat/b0010s1/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.lib.libc.softfloat.b0010s1

rm -rf oracle.o port.o harness.o harness gcm.cache "$MODULE.pcm"

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

case "$("$CXX" --version 2>&1)" in
*clang*)
	# Clang: precompile the module interface, then feed it to the importer.
	"$CXX" -std=c++23 -O2 -x c++-module port.cppm --precompile \
	    -o "$MODULE.pcm"
	"$CXX" -std=c++23 -O2 -c "$MODULE.pcm" -o port.o
	"$CXX" -std=c++23 -O2 -fmodule-file="$MODULE=$MODULE.pcm" \
	    -c harness.cpp -o harness.o
	;;
*)
	# GCC: -fmodules-ts, module interface first so gcm.cache is populated.
	# .cppm is not a recognised suffix, hence -x c++.
	"$CXX" -std=c++23 -O2 -fmodules-ts -c -x c++ port.cppm -o port.o
	"$CXX" -std=c++23 -O2 -fmodules-ts -c harness.cpp -o harness.o
	;;
esac

"$CXX" -std=c++23 -O2 -o harness harness.o port.o oracle.o

exec ./harness
