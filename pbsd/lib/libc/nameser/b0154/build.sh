#!/bin/sh
# Build and run the b0154 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/nameser/b0154/)
set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODULE=pbsd.lib.libc.nameser.b0154
OBJDIR=./.b0154-obj
HBSDINC="../../../../../hbsd/src/include"

if $CXX --version 2>&1 | grep -qi clang; then
	MODFLAGS="-fmodules"
	PRECOMPILE=clang
else
	MODFLAGS="-fmodules-ts"
	PRECOMPILE=gcc
fi

rm -rf "$OBJDIR" gcm.cache
mkdir -p "$OBJDIR"

$CC $CFLAGS -I"$HBSDINC" -c oracle.c -o "$OBJDIR/oracle.o"

if [ "$PRECOMPILE" = clang ]; then
	$CXX $CXXFLAGS $MODFLAGS -I"$HBSDINC" --precompile -x c++-module port.cppm \
	    -o "$OBJDIR/port.pcm"
	$CXX $CXXFLAGS $MODFLAGS -c "$OBJDIR/port.pcm" -o "$OBJDIR/port.o"
	$CXX $CXXFLAGS $MODFLAGS -I"$HBSDINC" \
	    -fmodule-file="$MODULE=$OBJDIR/port.pcm" \
	    -c harness.cpp -o "$OBJDIR/harness.o"
else
	$CXX $CXXFLAGS $MODFLAGS -I"$HBSDINC" -c -x c++ port.cppm -o "$OBJDIR/port.o"
	$CXX $CXXFLAGS $MODFLAGS -I"$HBSDINC" -c harness.cpp -o "$OBJDIR/harness.o"
fi

$CXX $CXXFLAGS $MODFLAGS "$OBJDIR/harness.o" "$OBJDIR/port.o" \
    "$OBJDIR/oracle.o" -lresolv -o "$OBJDIR/b0154_test"

exec "$OBJDIR/b0154_test"
