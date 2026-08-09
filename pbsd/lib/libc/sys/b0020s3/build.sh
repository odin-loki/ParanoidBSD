#!/bin/sh
# Build and run the b0020s3 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/sys/b0020s3/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

# GCC wants -fmodules-ts for C++ modules and does not recognise the .cppm
# suffix, hence -x c++.  Clang wants -fmodules and the interface unit
# precompiled to a .pcm first.  Probe for whichever this toolchain accepts.
if $CXX -std=c++23 -fmodules-ts -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS=-fmodules-ts
	MODE=gcc
elif $CXX -std=c++23 -fmodules -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS=-fmodules
	MODE=clang
else
	echo "build.sh: $CXX supports neither -fmodules-ts nor -fmodules" >&2
	exit 2
fi

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC -std=c11 -O2 -c oracle.c -o oracle.o

if [ "$MODE" = gcc ]; then
	$CXX -std=c++23 $MODFLAGS -O2 -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 $MODFLAGS -O2 -c harness.cpp -o harness.o
else
	$CXX -std=c++23 $MODFLAGS -O2 -x c++-module port.cppm \
	    --precompile -o port.pcm
	$CXX -std=c++23 $MODFLAGS -O2 -c port.pcm -o port.o
	$CXX -std=c++23 $MODFLAGS -O2 -fmodule-file=port.pcm \
	    -c harness.cpp -o harness.o
fi

$CXX -std=c++23 $MODFLAGS -O2 oracle.o port.o harness.o -o harness

exec ./harness
