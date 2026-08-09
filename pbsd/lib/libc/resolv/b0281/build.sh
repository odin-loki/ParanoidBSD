#!/bin/sh
#
# Build and run the b0281 differential test.  Run as `sh build.sh' from
# pbsd/lib/libc/resolv/b0281/.  The harness's exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

MODNAME=pbsd.lib.libc.resolv.b0281

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 -x c++-module port.cppm --precompile -o port.pcm
	$CXX -std=c++23 -O2 -c port.pcm -o port.o
	$CXX -std=c++23 -O2 -fmodule-file="$MODNAME"=port.pcm -c harness.cpp \
	    -o harness.o
else
	$CXX -std=c++23 -O2 -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -O2 -fmodules-ts -c harness.cpp -o harness.o
fi

WRAP_FLAGS="-Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=free \
    -Wl,--wrap=clock_gettime -Wl,--wrap=stat"

$CXX -std=c++23 -O2 $WRAP_FLAGS -o harness harness.o port.o oracle.o

exec ./harness
