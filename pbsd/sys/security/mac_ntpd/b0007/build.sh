#!/bin/sh
# Build and run the PBSD mac_ntpd b0007 differential test.
set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

rm -f oracle.o port.o harness.o port.pcm b0007_test
rm -rf gcm.cache

$CC $CFLAGS -c oracle.c -o oracle.o

MODNAME=pbsd.sys.security.mac.ntpd.b0007

if $CXX --version 2>&1 | grep -i clang >/dev/null 2>&1; then
	$CXX $CXXFLAGS -x c++-module port.cppm --precompile -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODNAME"=port.pcm -c harness.cpp -o harness.o
else
	$CXX $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS harness.o port.o oracle.o -o b0007_test

exec ./b0007_test
