#!/bin/sh
# Build and run the b0225 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/compat-43/b0225/)
set -eu

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.lib.libc.compat_43.b0225

rm -rf gcm.cache port.pcm oracle.o port.o harness.o harness

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	"$CXX" -std=c++23 -O2 -fpermissive --precompile -x c++-module port.cppm -o port.pcm
	"$CXX" -std=c++23 -O2 -fpermissive -c port.pcm -o port.o
	"$CXX" -std=c++23 -O2 -fpermissive -fmodule-file="$MODNAME=port.pcm" \
	    -c harness.cpp -o harness.o
else
	"$CXX" -std=c++23 -fmodules-ts -O2 -fpermissive -x c++ -c port.cppm -o port.o
	"$CXX" -std=c++23 -fmodules-ts -O2 -fpermissive -c harness.cpp -o harness.o
fi

"$CXX" -std=c++23 -O2 -fpermissive -o harness port.o harness.o oracle.o

exec ./harness
