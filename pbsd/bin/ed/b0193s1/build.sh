#!/bin/sh
# Build and run the b0193s1 differential test.
# Usage: sh build.sh   (from pbsd/bin/ed/b0193s1/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODNAME=pbsd.bin.ed.b0193s1
OBJDIR=.build

rm -rf "$OBJDIR"
mkdir -p "$OBJDIR"

$CC $CFLAGS -c oracle.c -o "$OBJDIR/oracle.o"

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o "$OBJDIR/$MODNAME.pcm"
	$CXX $CXXFLAGS -c "$OBJDIR/$MODNAME.pcm" -o "$OBJDIR/port.o"
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=$OBJDIR/$MODNAME.pcm" \
	    -c harness.cpp -o "$OBJDIR/harness.o"
else
	( cd "$OBJDIR" && \
	  $CXX $CXXFLAGS -fmodules-ts -x c++ -c ../port.cppm -o port.o && \
	  $CXX $CXXFLAGS -fmodules-ts -c ../harness.cpp -o harness.o )
fi

$CXX $CXXFLAGS -o "$OBJDIR/harness" \
    "$OBJDIR/harness.o" "$OBJDIR/port.o" "$OBJDIR/oracle.o"

exec "$OBJDIR/harness"
