#!/bin/sh
#
# Build and run the b0043 differential test.
#
# Usage: sh build.sh          (from pbsd/lib/libcalendar/b0043/)
#
# The harness's exit status is this script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}

MODNAME=pbsd.lib.libcalendar.b0043

rm -rf gcm.cache oracle.o port.o harness.o port.pcm harness

# Which module dialect does this toolchain speak?
if $CXX --version 2>&1 | grep -qi clang; then
	MODE=clang
else
	MODE=gcc
fi

echo "=== compiling oracle.c ($CC $CFLAGS)"
$CC $CFLAGS -c oracle.c -o oracle.o

case $MODE in
clang)
	echo "=== compiling port.cppm ($CXX $CXXFLAGS, clang modules)"
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	echo "=== compiling harness.cpp"
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=port.pcm" \
	    -c harness.cpp -o harness.o
	echo "=== linking"
	$CXX $CXXFLAGS -o harness harness.o port.o oracle.o
	;;
gcc)
	echo "=== compiling port.cppm ($CXX $CXXFLAGS -fmodules-ts)"
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	echo "=== compiling harness.cpp"
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
	echo "=== linking"
	$CXX $CXXFLAGS -fmodules-ts -o harness harness.o port.o oracle.o
	;;
esac

echo "=== running"
exec ./harness
