#!/bin/sh
#
# build.sh -- build and run the PBSD batch b0047 differential test.
#
# Compiles oracle.c as C11, port.cppm and harness.cpp as C++23 (with whichever
# named-module flags the local toolchain wants), links the three together and
# execs the harness, so the harness exit status is this script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
# float64_rem hands add64 an `sbits32 *' where the prototype says `bits32 *';
# that is in the original body, so the diagnostic is silenced rather than the
# code changed.
CFLAGS="-std=c11 -O2 -Wno-incompatible-pointer-types"
CXXFLAGS="-std=c++23 -O2"
MODNAME=pbsd.lib.libc.softfloat.bits32.b0047
OUT=b0047_harness

rm -rf gcm.cache "$OUT" oracle.o port.o harness.o port.pcm

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODNAME=port.pcm" -c harness.cpp \
	    -o harness.o
else
	$CXX $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o "$OUT" harness.o port.o oracle.o

exec ./"$OUT"
