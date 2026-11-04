#!/bin/sh
# PBSD batch b0054s1 -- build and run the differential harness.
#
# Usage: sh build.sh   (from pbsd/usr.bin/cksum/b0054s1/)
#
# Compiles the C oracle, the C++23 module port and the harness, links the
# three together and execs the harness, so the harness exit status becomes
# this script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

OBJDIR=./build
rm -rf "$OBJDIR" gcm.cache
mkdir -p "$OBJDIR"

"$CC" $CFLAGS -c oracle.c -o "$OBJDIR/oracle.o"

if "$CXX" --version 2>&1 | grep -qi clang; then
	# Clang: precompile the module interface, then reference the BMI.
	"$CXX" $CXXFLAGS --precompile -x c++-module port.cppm -o "$OBJDIR/port.pcm"
	"$CXX" $CXXFLAGS -c "$OBJDIR/port.pcm" -o "$OBJDIR/port.o"
	"$CXX" $CXXFLAGS -fmodule-file=pbsd.usr.bin.cksum.b0054s1="$OBJDIR/port.pcm" \
		-c harness.cpp -o "$OBJDIR/harness.o"
else
	# GCC: -fmodules-ts, module interface first so the CMI exists.
	"$CXX" $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o "$OBJDIR/port.o"
	"$CXX" $CXXFLAGS -fmodules-ts -c harness.cpp -o "$OBJDIR/harness.o"
fi

"$CXX" $CXXFLAGS -o "$OBJDIR/harness" "$OBJDIR/harness.o" "$OBJDIR/port.o" \
	"$OBJDIR/oracle.o"

exec "$OBJDIR/harness"
