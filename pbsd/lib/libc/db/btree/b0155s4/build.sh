#!/bin/sh
# Build and run the b0155s4 differential test.
#   sh build.sh
# Exit status is the harness exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.lib.libc.db.btree.b0155s4

rm -rf gcm.cache port.pcm port.o harness.o oracle.o harness

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	"$CXX" -std=c++23 -x c++-module port.cppm --precompile -o port.pcm
	"$CXX" -std=c++23 -c port.pcm -o port.o
	"$CXX" -std=c++23 -fmodule-file="$MODULE=port.pcm" \
	    -c harness.cpp -o harness.o
else
	# GCC: -fmodules-ts, interface unit first (populates gcm.cache).
	"$CXX" -std=c++23 -fmodules-ts -x c++ -c port.cppm -o port.o
	"$CXX" -std=c++23 -fmodules-ts -c harness.cpp -o harness.o
fi

"$CXX" -std=c++23 port.o harness.o oracle.o -o harness

exec ./harness
