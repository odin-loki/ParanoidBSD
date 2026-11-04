#!/bin/sh
set -e
cd "$(dirname "$0")"
CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}
MODNAME=pbsd.bin.sh.b0230
BUILD=build
rm -rf "$BUILD" gcm.cache
mkdir -p "$BUILD"
$CC $CFLAGS -c oracle.c -o "$BUILD/oracle.o"
if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module port.cppm --precompile -o "$BUILD/port.pcm"
	$CXX $CXXFLAGS -c "$BUILD/port.pcm" -o "$BUILD/port.o"
	$CXX $CXXFLAGS -fmodule-file=$MODNAME="$BUILD/port.pcm" -c harness.cpp -o "$BUILD/harness.o"
else
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o "$BUILD/port.o"
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o "$BUILD/harness.o"
fi
$CXX $CXXFLAGS -o "$BUILD/b0230_test" "$BUILD/harness.o" "$BUILD/port.o" "$BUILD/oracle.o"
exec "$BUILD/b0230_test"
