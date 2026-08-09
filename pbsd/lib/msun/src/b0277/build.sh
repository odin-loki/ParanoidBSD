#!/bin/sh
# Build and run the differential test for PBSD batch b0277.
# Usage: sh build.sh   (from pbsd/lib/msun/src/b0277/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.lib.msun.src.b0277

ROOT=$(cd ../../../../.. && pwd)
MSUN=$ROOT/hbsd/src/lib/msun/src
LD80=$ROOT/hbsd/src/lib/msun/ld80
LIBC_INC=$ROOT/hbsd/src/lib/libc/include
AMD64_INC=$ROOT/hbsd/src/lib/libc/amd64

PREREQ=$(mktemp)
TMPDIR=$(mktemp -d)
trap 'rm -f "$PREREQ"; rm -rf "$TMPDIR"' EXIT

cat > "$PREREQ" << 'EOF'
#include <endian.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

#ifndef _SYS_ENDIAN_H_
#define _BYTE_ORDER __BYTE_ORDER
#define _LITTLE_ENDIAN __LITTLE_ENDIAN
#define _BIG_ENDIAN __BIG_ENDIAN
#endif

#define _MATH_PRIVATE_H_

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

typedef float __float_t;
typedef double __double_t;

#ifndef STRICT_ASSIGN
#define STRICT_ASSIGN(type, lval, rval) ((lval) = (rval))
#endif

static inline int irint(double x) { return (int)x; }
static inline double rnint(double x) { return rint(x); }

extern double __kernel_sin(double, double, int);
extern double __kernel_cos(double, double);
extern int __ieee754_rem_pio2(double, double *);
EOF

sed 's/#include "math.h"/#include <math.h>/; s/#include "math_private.h"/#include "math_private_skip.h"/' \
    "$MSUN/k_rem_pio2.c" > "$TMPDIR/k_rem_pio2.c"
sed 's/#include "math.h"/#include <math.h>/; s/#include "math_private.h"/#include "math_private_skip.h"/' \
    "$MSUN/e_rem_pio2.c" > "$TMPDIR/e_rem_pio2.c"
sed 's/#include "math.h"/#include <math.h>/; s/#include "math_private.h"/#include "math_private_skip.h"/' \
    "$MSUN/k_sin.c" > "$TMPDIR/k_sin.c"
sed 's/#include "math.h"/#include <math.h>/; s/#include "math_private.h"/#include "math_private_skip.h"/' \
    "$MSUN/k_cos.c" > "$TMPDIR/k_cos.c"
echo '#define _MATH_PRIVATE_H_' > "$TMPDIR/math_private_skip.h"

rm -rf gcm.cache b0277_run oracle.o port.o harness.o k_rem_pio2.o e_rem_pio2.o \
    k_sin.o k_cos.o invtrig.o "$MODULE_NAME.pcm"

CFLAGS="-std=c11 -O2 -include $PREREQ -I$LD80 -I$LIBC_INC -I$AMD64_INC"
CXXFLAGS="-std=c++23 -O2 -include $PREREQ"

$CC $CFLAGS -c oracle.c -o oracle.o
$CC $CFLAGS -c "$TMPDIR/k_rem_pio2.c" -o k_rem_pio2.o
$CC $CFLAGS -Dzero=e_rem_zero -Dtwo24=e_rem_two24 -c "$TMPDIR/e_rem_pio2.c" \
    -o e_rem_pio2.o
$CC $CFLAGS -c "$TMPDIR/k_sin.c" -o k_sin.o
$CC $CFLAGS -c "$TMPDIR/k_cos.c" -o k_cos.o
$CC $CFLAGS -c "$LD80/invtrig.c" -o invtrig.o

OBJS="oracle.o k_rem_pio2.o e_rem_pio2.o k_sin.o k_cos.o invtrig.o"

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o "$MODULE_NAME.pcm"
	$CXX $CXXFLAGS -c "$MODULE_NAME.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE_NAME=$MODULE_NAME.pcm" \
	    -c harness.cpp -o harness.o
	$CXX $CXXFLAGS $OBJS port.o harness.o -o b0277_run -lm
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
	$CXX $CXXFLAGS $MODFLAG $OBJS port.o harness.o -o b0277_run -lm
fi

exec ./b0277_run
