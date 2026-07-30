#!/bin/sh
# Build and run the b0067 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/compat-43/b0067/)
set -eu

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.lib.libc.compat.43.b0067

rm -rf gcm.cache port.pcm oracle.o port.o harness.o harness

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then feed it to the importer
	"$CXX" -std=c++23 -O2 --precompile -x c++-module port.cppm -o port.pcm
	"$CXX" -std=c++23 -O2 -c port.pcm -o port.o
	"$CXX" -std=c++23 -O2 -fmodule-file="$MODNAME=port.pcm" \
	    -c harness.cpp -o harness.o
else
	# gcc: -fmodules-ts, interface unit first so gcm.cache is populated
	"$CXX" -std=c++23 -fmodules-ts -O2 -x c++ -c port.cppm -o port.o
	"$CXX" -std=c++23 -fmodules-ts -O2 -c harness.cpp -o harness.o
fi

"$CXX" -std=c++23 -O2 -o harness port.o harness.o oracle.o

exec ./harness
