#!/bin/sh
# Build and run the b0019 differential test.
#   sh build.sh
# Exit code is the harness exit code: 0 iff every case matched.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.lib.libc.uuid.b0019

rm -rf gcm.cache oracle.o port.o harness.o "$MODNAME".pcm harness

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	# Clang: precompile the interface unit, then feed the BMI to the TU
	# that imports it.
	"$CXX" -std=c++23 -O2 -x c++-module --precompile port.cppm \
	    -o "$MODNAME".pcm
	"$CXX" -std=c++23 -O2 -c "$MODNAME".pcm -o port.o
	"$CXX" -std=c++23 -O2 -fmodule-file="$MODNAME"="$MODNAME".pcm \
	    -c harness.cpp -o harness.o
else
	# GCC: -fmodules-ts, CMI is written to ./gcm.cache by the first
	# compilation and picked up by the second.
	"$CXX" -std=c++23 -O2 -fmodules-ts -x c++ -c port.cppm -o port.o
	"$CXX" -std=c++23 -O2 -fmodules-ts -c harness.cpp -o harness.o
fi

"$CXX" -std=c++23 -O2 -o harness harness.o port.o oracle.o

exec ./harness
