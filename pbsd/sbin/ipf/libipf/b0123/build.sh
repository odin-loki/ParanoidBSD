#!/bin/sh
#
# build.sh -- build and run the PBSD b0123 differential test.
#
# Usage: sh build.sh            (from pbsd/sbin/ipf/libipf/b0123/)
#
# Compiles the C oracle, the C++23 module port and the harness, links them
# together and execs the resulting binary so that its exit status becomes the
# exit status of this script.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:-"-std=c11 -O2"}
CXXFLAGS=${CXXFLAGS:-"-std=c++23 -O2"}
LDFLAGS=${LDFLAGS:-"-Wl,--wrap=free"}

BUILD=build
MODNAME=pbsd.sbin.ipf.libipf.b0123

rm -rf "$BUILD" gcm.cache
mkdir -p "$BUILD"

# 1. the reference implementation (plain C11)
$CC $CFLAGS -c oracle.c -o "$BUILD/oracle.o"

# 2. + 3. the module interface unit and the harness that imports it.
#    Module flags differ between toolchains, so probe for clang first.
MODFLAGS=""
if $CXX --version 2>&1 | grep -qi 'clang'; then
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm \
	    -o "$BUILD/port.pcm"
	$CXX $CXXFLAGS -c "$BUILD/port.pcm" -o "$BUILD/port.o"
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=$BUILD/port.pcm" \
	    -c harness.cpp -o "$BUILD/harness.o"
else
	MODFLAGS="-fmodules-ts"
	$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o "$BUILD/port.o"
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o "$BUILD/harness.o"
fi

# 4. link
$CXX $CXXFLAGS $MODFLAGS "$BUILD/port.o" "$BUILD/harness.o" \
    "$BUILD/oracle.o" $LDFLAGS -o "$BUILD/b0123_test"

exec "$BUILD/b0123_test"
