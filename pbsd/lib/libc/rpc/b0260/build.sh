#!/bin/sh
# Build and run the b0260 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/rpc/b0260/)
set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODULE=pbsd.lib.libc.rpc.b0260
OBJDIR=./.b0260-obj

if $CXX --version 2>&1 | grep -qi clang; then
	MODFLAGS="-fmodules"
	PRECOMPILE=clang
else
	MODFLAGS="-fmodules-ts"
	PRECOMPILE=gcc
fi

rm -rf "$OBJDIR" gcm.cache
mkdir -p "$OBJDIR"

$CC $CFLAGS -c oracle.c -o "$OBJDIR/oracle.o"

if [ "$PRECOMPILE" = clang ]; then
	$CXX $CXXFLAGS $MODFLAGS --precompile -x c++-module port.cppm \
	    -o "$OBJDIR/port.pcm"
	$CXX $CXXFLAGS $MODFLAGS -c "$OBJDIR/port.pcm" -o "$OBJDIR/port.o"
	$CXX $CXXFLAGS $MODFLAGS -fmodule-file="$MODULE=$OBJDIR/port.pcm" \
	    -c harness.cpp -o "$OBJDIR/harness.o"
else
	$CXX $CXXFLAGS $MODFLAGS -c -x c++ port.cppm -o "$OBJDIR/port.o"
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o "$OBJDIR/harness.o"
fi

$CXX $CXXFLAGS $MODFLAGS "$OBJDIR/harness.o" "$OBJDIR/port.o" \
    "$OBJDIR/oracle.o" -o "$OBJDIR/b0260_test"

exec "$OBJDIR/b0260_test"
