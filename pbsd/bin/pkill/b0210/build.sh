#!/bin/sh
#
# PBSD batch b0210 -- bin/pkill.
#
# Builds the C oracle, the C++23 module port and the differential harness,
# links them together and execs the harness so its exit status becomes the
# exit status of this script.

set -e

cd "$(dirname "$0")"

: "${CC:=cc}"
: "${CXX:=c++}"

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

BUILD=./_build
rm -rf "$BUILD"
mkdir -p "$BUILD"

MODNAME=pbsd.bin.pkill.b0210

$CC $CFLAGS -c oracle.c -o "$BUILD/oracle.o"

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface unit, then compile it to an object.
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o "$BUILD/$MODNAME.pcm"
	$CXX $CXXFLAGS -c "$BUILD/$MODNAME.pcm" -o "$BUILD/port.o"
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=$BUILD/$MODNAME.pcm" \
	    -c harness.cpp -o "$BUILD/harness.o"
else
	# gcc: -fmodules-ts, CMI goes to the module mapper cache directory.
	MODFLAGS="-fmodules-ts -fmodule-mapper=|@g++-mapper-server"
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o "$BUILD/port.o"
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o "$BUILD/harness.o"
fi

$CXX $CXXFLAGS -o "$BUILD/b0210_test" "$BUILD/harness.o" "$BUILD/port.o" \
    "$BUILD/oracle.o"

exec "$BUILD/b0210_test"
