#!/bin/sh
# Build and run the b0124s1 differential test.
#   sh build.sh
# Exit code is the harness exit code.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS_C="-std=c11 -O2"
CFLAGS_CXX="-std=c++23 -O2"

rm -f oracle.o port.o harness.o port.pcm harness
rm -rf gcm.cache

# The C oracle.
$CC $CFLAGS_C -c oracle.c -o oracle.o

# Which C++ front end are we dealing with?  The module flags differ.
if $CXX --version 2>&1 | grep -qi clang; then
	MODFLAGS=""
	$CXX $CFLAGS_CXX -x c++-module port.cppm --precompile -o port.pcm
	$CXX $CFLAGS_CXX -c port.pcm -o port.o
	$CXX $CFLAGS_CXX \
	    -fmodule-file=pbsd.sbin.ipf.libipf.b0124s1=port.pcm \
	    -c harness.cpp -o harness.o
else
	MODFLAGS="-fmodules-ts"
	mkdir -p gcm.cache
	$CXX $CFLAGS_CXX $MODFLAGS -x c++ -c port.cppm -o port.o
	$CXX $CFLAGS_CXX $MODFLAGS -c harness.cpp -o harness.o
fi

$CXX $CFLAGS_CXX $MODFLAGS -o harness harness.o port.o oracle.o

exec ./harness
