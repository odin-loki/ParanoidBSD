#!/bin/sh
# Build and run the differential test for PBSD batch b0267.
# Usage: sh build.sh   (from pbsd/lib/msun/src/b0267/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.lib.msun.src.b0267

ROOT=$(cd ../../../../.. && pwd)
MSUN=$ROOT/hbsd/src/lib/msun/src

PREREQ=$(mktemp)
trap 'rm -f "$PREREQ"' EXIT

cat > "$PREREQ" << 'EOF'
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>
#include <endian.h>

#define _MATH_H_
#define _MATH_PRIVATE_H_

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

#define IEEE_WORD_ORDER __BYTE_ORDER

typedef union {
  double value;
  struct {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint32_t msw;
    uint32_t lsw;
#else
    uint32_t lsw;
    uint32_t msw;
#endif
  } parts;
} ieee_double_shape_type;

#define GET_HIGH_WORD(i,d) do { ieee_double_shape_type gh_u; gh_u.value = (d); (i) = (int32_t)gh_u.parts.msw; } while(0)
#define GET_LOW_WORD(i,d) do { ieee_double_shape_type gl_u; gl_u.value = (d); (i) = (int32_t)gl_u.parts.lsw; } while(0)
#define INSERT_WORDS(d,ix0,ix1) do { ieee_double_shape_type iw_u; iw_u.parts.msw = (uint32_t)(ix0); iw_u.parts.lsw = (uint32_t)(ix1); (d) = iw_u.value; } while(0)

typedef float __float_t;
typedef double __double_t;

#ifndef STRICT_ASSIGN
#define STRICT_ASSIGN(type, lval, rval) ((lval) = (rval))
#endif

static inline int irint(double x) { return (int)x; }
static inline double rnint(double x) { return rint(x); }

#define INLINE_REM_PIO2
EOF

rm -rf gcm.cache b0267_run oracle.o port.o harness.o k_rem_pio2.o e_rem_pio2.o \
    "$MODULE_NAME.pcm"

CFLAGS_MSUN="-std=c11 -O2 -I$MSUN -include $PREREQ"
CFLAGS_ORACLE="-std=c11 -O2 -include $PREREQ"
CXXFLAGS="-std=c++23 -O2 -include $PREREQ"

$CC $CFLAGS_ORACLE -c oracle.c -o oracle.o
$CC $CFLAGS_MSUN -c "$MSUN/k_rem_pio2.c" -o k_rem_pio2.o
$CC $CFLAGS_MSUN -Dzero=e_rem_zero -Dtwo24=e_rem_two24 -c "$MSUN/e_rem_pio2.c" \
    -o e_rem_pio2.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o "$MODULE_NAME.pcm"
	$CXX $CXXFLAGS -c "$MODULE_NAME.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE_NAME=$MODULE_NAME.pcm" \
	    -c harness.cpp -o harness.o
	$CXX $CXXFLAGS oracle.o k_rem_pio2.o e_rem_pio2.o port.o harness.o \
	    -o b0267_run -lm
else
	MODFLAG=""
	for f in -fmodules-ts -fmodules ""; do
		if $CXX $CXXFLAGS $f -x c++ -fsyntax-only /dev/null >/dev/null 2>&1; then
			MODFLAG=$f
			break
		fi
	done
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
	$CXX $CXXFLAGS $MODFLAG oracle.o k_rem_pio2.o e_rem_pio2.o port.o harness.o \
	    -o b0267_run -lm
fi

exec ./b0267_run
