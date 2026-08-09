#!/bin/sh
# Build and run the PBSD b0276 differential test.
# Usage:  sh build.sh    (from pbsd/lib/msun/src/b0276/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE=pbsd.lib.msun.src.b0276
BIN=b0276_run

ROOT=$(cd ../../../../.. && pwd)
MSUN=$ROOT/hbsd/src/lib/msun/src

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT INT HUP TERM

# s_tan.c is a wrapper around the msun argument-reduction and tangent kernels.
# Those kernels are not part of this batch, so they are compiled verbatim from
# the untouched HardenedBSD sources and linked into both sides of the test.
# The two private headers they include are supplied here; a quoted #include is
# resolved relative to the including file, so dropping shims next to the copied
# sources is enough.
cat > "$WORK/math.h" << 'EOF'
#include <math.h>
EOF

cat > "$WORK/math_private.h" << 'EOF'
#ifndef _PBSD_B0276_MATH_PRIVATE_H_
#define _PBSD_B0276_MATH_PRIVATE_H_

#include <math.h>
#include <stdint.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

typedef unsigned int u_int32_t;
typedef unsigned long long u_int64_t;
typedef float __float_t;
typedef double __double_t;

typedef union {
	double value;
	struct {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		uint32_t msw;
		uint32_t lsw;
#else
		uint32_t lsw;
		uint32_t msw;
#endif
	} parts;
	struct {
		uint64_t w;
	} xparts;
} ieee_double_shape_type;

typedef union {
	float value;
	uint32_t word;
} ieee_float_shape_type;

#define EXTRACT_WORDS(ix0,ix1,d)					\
do { ieee_double_shape_type ew_u; ew_u.value = (d);			\
     (ix0) = ew_u.parts.msw; (ix1) = ew_u.parts.lsw; } while (0)
#define EXTRACT_WORD64(ix,d)						\
do { ieee_double_shape_type ew_u; ew_u.value = (d);			\
     (ix) = ew_u.xparts.w; } while (0)
#define GET_HIGH_WORD(i,d)						\
do { ieee_double_shape_type gh_u; gh_u.value = (d);			\
     (i) = gh_u.parts.msw; } while (0)
#define GET_LOW_WORD(i,d)						\
do { ieee_double_shape_type gl_u; gl_u.value = (d);			\
     (i) = gl_u.parts.lsw; } while (0)
#define INSERT_WORDS(d,ix0,ix1)						\
do { ieee_double_shape_type iw_u; iw_u.parts.msw = (ix0);		\
     iw_u.parts.lsw = (ix1); (d) = iw_u.value; } while (0)
#define INSERT_WORD64(d,ix)						\
do { ieee_double_shape_type iw_u; iw_u.xparts.w = (ix);			\
     (d) = iw_u.value; } while (0)
#define SET_HIGH_WORD(d,v)						\
do { ieee_double_shape_type sh_u; sh_u.value = (d);			\
     sh_u.parts.msw = (v); (d) = sh_u.value; } while (0)
#define SET_LOW_WORD(d,v)						\
do { ieee_double_shape_type sl_u; sl_u.value = (d);			\
     sl_u.parts.lsw = (v); (d) = sl_u.value; } while (0)
#define GET_FLOAT_WORD(i,d)						\
do { ieee_float_shape_type gf_u; gf_u.value = (d);			\
     (i) = gf_u.word; } while (0)
#define SET_FLOAT_WORD(d,i)						\
do { ieee_float_shape_type sf_u; sf_u.word = (i);			\
     (d) = sf_u.value; } while (0)

#ifndef STRICT_ASSIGN
#define STRICT_ASSIGN(type, lval, rval)	((lval) = (rval))
#endif

static __inline double
rnint(__double_t x)
{
	return (rint((double)x));
}

static __inline int
irint(double x)
{
	return ((int)lrint(x));
}

int	__kernel_rem_pio2(double *, double *, int, int, int);
int	__ieee754_rem_pio2(double, double *);
double	__kernel_tan(double, double, int);

#endif /* _PBSD_B0276_MATH_PRIVATE_H_ */
EOF

for f in k_rem_pio2.c e_rem_pio2.c k_tan.c; do
	cp "$MSUN/$f" "$WORK/$f"
done

rm -rf gcm.cache "$BIN" oracle.o port.o harness.o k_rem_pio2.o e_rem_pio2.o \
    k_tan.o "$MODULE.pcm"

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

$CC $CFLAGS -c oracle.c -o oracle.o
$CC $CFLAGS -c "$WORK/k_rem_pio2.c" -o k_rem_pio2.o
$CC $CFLAGS -c "$WORK/e_rem_pio2.c" -o e_rem_pio2.o
$CC $CFLAGS -c "$WORK/k_tan.c" -o k_tan.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm -o "$MODULE.pcm"
	$CXX $CXXFLAGS -c "$MODULE.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE=$MODULE.pcm" -c harness.cpp \
	    -o harness.o
	MODFLAG=
else
	MODFLAG=-fmodules-ts
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS $MODFLAG port.o harness.o oracle.o k_rem_pio2.o e_rem_pio2.o \
    k_tan.o -o "$BIN" -lm

exec ./"$BIN"
