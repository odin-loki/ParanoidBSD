#!/bin/sh
# PBSD batch b0196s1 -- build and run the differential harness.
#
# Usage: sh build.sh   (from pbsd/lib/libc/rpc/b0196s1/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.lib.libc.rpc.b0196s1

# The C++ driver needs a compiler with usable C++23 module support.  Prefer
# whatever "c++" is if it is clang, otherwise fall back to a clang++ on PATH,
# otherwise use GCC's -fmodules-ts.
if ! "$CXX" --version 2>/dev/null | grep -qi clang; then
	if command -v clang++ >/dev/null 2>&1; then
		CXX=clang++
	fi
fi

CFLAGS="-std=c11 -O2 -Wall"
CXXFLAGS="-std=c++23 -O2 -Wall"
LDLIBS="-lpthread"

rm -f oracle.o port.o harness.o port.pcm harness
rm -rf gcm.cache

"$CC" $CFLAGS -c oracle.c -o oracle.o

if "$CXX" --version 2>/dev/null | grep -qi clang; then
	"$CXX" $CXXFLAGS -x c++-module --precompile port.cppm -o port.pcm
	"$CXX" $CXXFLAGS -c port.pcm -o port.o
	"$CXX" $CXXFLAGS -fmodule-file="$MODULE"=port.pcm \
	    -c harness.cpp -o harness.o
else
	"$CXX" $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	"$CXX" $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

"$CXX" $CXXFLAGS -o harness harness.o port.o oracle.o $LDLIBS

exec ./harness
