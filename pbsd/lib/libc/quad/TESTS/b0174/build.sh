#!/bin/sh
#
# Build and run the b0174 differential test.
#
# Usage: sh build.sh          (from pbsd/lib/libc/quad/TESTS/b0174/)
#
# The harness's exit status is this script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}

MODNAME=pbsd.lib.libc.quad.tests.b0174
ROOT=$(cd ../../../../../.. && pwd)
QUADDIR=$ROOT/hbsd/src/lib/libc/quad

rm -rf gcm.cache oracle.o port.o harness.o port.pcm harness \
    muldi3.o qdivrem.o quad_linux.h

cat >quad_linux.h <<'EOF'
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
typedef long long quad_t;
typedef unsigned long long u_quad_t;
#ifndef u_long
typedef unsigned long u_long;
#endif
#ifndef __predict_false
#define __predict_false(exp) __builtin_expect((exp) != 0, 0)
#endif
#ifndef _QUAD_HIGHWORD
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _QUAD_HIGHWORD 0
#define _QUAD_LOWWORD 1
#else
#define _QUAD_HIGHWORD 1
#define _QUAD_LOWWORD 0
#endif
#endif
EOF

QUAD_CFLAGS="$CFLAGS -I$QUADDIR -include quad_linux.h"

if $CXX --version 2>&1 | grep -qi clang; then
	MODE=clang
else
	MODE=gcc
fi

echo "=== compiling quad helpers ($CC $QUAD_CFLAGS)"
$CC $QUAD_CFLAGS -c "$QUADDIR/muldi3.c" -o muldi3.o
$CC $QUAD_CFLAGS -c "$QUADDIR/qdivrem.c" -o qdivrem.o

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
	$CXX $CXXFLAGS -o harness harness.o port.o oracle.o muldi3.o qdivrem.o
	;;
gcc)
	echo "=== compiling port.cppm ($CXX $CXXFLAGS -fmodules-ts)"
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	echo "=== compiling harness.cpp"
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
	echo "=== linking"
	$CXX $CXXFLAGS -fmodules-ts -o harness harness.o port.o oracle.o \
	    muldi3.o qdivrem.o
	;;
esac

echo "=== running"
exec ./harness
