#!/bin/sh
#
# Build and run the b0104 differential test.  Run as `sh build.sh' from
# pbsd/lib/libc/stdio/b0104/.  The harness's exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

MODNAME=pbsd.lib.libc.stdio.b0104

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC -std=c11 -D_GNU_SOURCE $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -D_GNU_SOURCE $CXXFLAGS -x c++-module port.cppm --precompile \
	    -o port.pcm
	$CXX -std=c++23 -D_GNU_SOURCE $CXXFLAGS -c port.pcm -o port.o
	$CXX -std=c++23 -D_GNU_SOURCE $CXXFLAGS -fmodule-file=$MODNAME=port.pcm \
	    -c harness.cpp -o harness.o
else
	$CXX -std=c++23 -D_GNU_SOURCE -fmodules-ts $CXXFLAGS -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -D_GNU_SOURCE -fmodules-ts $CXXFLAGS -c harness.cpp -o harness.o
fi

$CXX -std=c++23 -D_GNU_SOURCE $CXXFLAGS -o harness harness.o port.o oracle.o

exec ./harness
