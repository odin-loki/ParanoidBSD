#!/bin/sh
set -e
cd "$(dirname "$0")"
CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}
MODNAME=pbsd.bin.sh.b0228
rm -rf gcm.cache build
mkdir -p build
$CC -std=c11 $CFLAGS -c oracle.c -o build/oracle.o
if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 $CXXFLAGS -x c++-module port.cppm --precompile -o build/port.pcm
	$CXX -std=c++23 $CXXFLAGS -c build/port.pcm -o build/port.o
	$CXX -std=c++23 $CXXFLAGS -fmodule-file=$MODNAME=build/port.pcm -c harness.cpp -o build/harness.o
else
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -x c++ -c port.cppm -o build/port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c harness.cpp -o build/harness.o
fi
$CXX -std=c++23 $CXXFLAGS -o build/b0228_test build/harness.o build/port.o build/oracle.o
exec ./build/b0228_test
