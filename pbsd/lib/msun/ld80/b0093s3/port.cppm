/*
 * PBSD batch b0093s3 -- C++23 port of lib/msun/ld80/s_sinpil.c
 *
 * Faithful transliteration: evaluation order, integer signedness, the long
 * double pi_hi/pi_lo constants and the static inline kernels from
 * ld80/k_cospil.h and ld80/k_sinpil.h are preserved exactly.
 *
 * __kernel_sinl() and __kernel_cosl() are separate translation units of
 * libm (ld80/k_sinl.c, k_cosl.c); they are declared here as msun declares
 * them and resolved at link time against oracle.c.
 */

module;

#include <cstdint>

export module pbsd.lib.msun.ld80.b0093s3;

namespace pbsd::lib_msun_ld80::b0093s3 {

using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

extern "C" {
long double copysignl(long double, long double);
long double __kernel_sinl(long double, long double, int);
long double __kernel_cosl(long double, long double);
}

/* ------------------------------------------------------------------ */
/* from lib/libc/amd64/_fpmath.h					    */
/* ------------------------------------------------------------------ */

union IEEEl2bits {
	long double	e;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:32;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
		unsigned int	junkl	:16;
		unsigned int	junkh	:32;
	} bits;
	struct {
		unsigned long	man	:64;
		unsigned int	expsign	:16;
		unsigned long	junk	:48;
	} xbits;
};

/* ------------------------------------------------------------------ */
/* from lib/msun/src/math_private.h				    */
/* ------------------------------------------------------------------ */

#define	EXTRACT_LDBL80_WORDS(ix0,ix1,d)				\
do {								\
  union IEEEl2bits ew_u;					\
  ew_u.e = (d);							\
  (ix0) = ew_u.xbits.expsign;					\
  (ix1) = ew_u.xbits.man;					\
} while (0)

#define	INSERT_LDBL80_WORDS(d,ix0,ix1)				\
do {								\
  union IEEEl2bits iw_u;					\
  iw_u.xbits.expsign = (ix0);					\
  iw_u.xbits.man = (ix1);					\
  (d) = iw_u.e;							\
} while (0)

#define	LD80C(m, ex, v)	{ .e = (v), }

#define	ENTERI()
#define	RETURNI(x)	RETURNF(x)
#define	RETURNF(v)	return (v)

#define	_2sumF(a, b) do {	\
	__typeof(a) __w;	\
				\
	__w = (a) + (b);	\
	(b) = ((a) - __w) + (b); \
	(a) = __w;		\
} while (0)

#define	FFLOORL80(x, j0, ix, lx) do {			\
	j0 = ix - 0x3fff + 1;				\
	if ((j0) < 32) {				\
		(lx) = ((lx) >> 32) << 32;		\
		(lx) &= ~((((lx) << 32)-1) >> (j0));	\
	} else {					\
		uint64_t _m;				\
		_m = (uint64_t)-1 >> (j0);		\
		if ((lx) & _m) (lx) &= ~_m;		\
	}						\
	INSERT_LDBL80_WORDS((x), (ix), (lx));		\
} while (0)

/* ================================================================== */
/* lib/msun/ld80/s_sinpil.c					      */
/* ================================================================== */

/*-
 * Copyright (c) 2017, 2023 Steven G. Kargl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * See ../src/s_sinpi.c for implementation details.
 */

static const union IEEEl2bits
pi_hi_u = LD80C(0xc90fdaa200000000,   1, 3.14159265346825122833e+00L),
pi_lo_u = LD80C(0x85a308d313198a2e, -33, 1.21542010130123852029e-10L);
#define	pi_hi	(pi_hi_u.e)
#define	pi_lo	(pi_lo_u.e)

/* ---- lib/msun/ld80/k_cospil.h ----
 * See ../src/k_cospi.c for implementation details.
 */

static inline long double
__kernel_cospil(long double x)
{
	long double hi, lo;

	hi = (float)x;
	lo = x - hi;
	lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
	hi *= pi_hi;
	_2sumF(hi, lo);
	return (__kernel_cosl(hi, lo));
}

/* ---- lib/msun/ld80/k_sinpil.h ----
 * See ../src/k_sinpi.c for implementation details.
 */

static inline long double
__kernel_sinpil(long double x)
{
	long double hi, lo;

	hi = (float)x;
	lo = x - hi;
	lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
	hi *= pi_hi;
	_2sumF(hi, lo);
	return (__kernel_sinl(hi, lo, 1));
}

volatile static const double vzero = 0;

export long double
sinpil(long double x)
{
	long double ax, hi, lo, s;
	uint64_t lx, m;
	uint32_t j0;
	uint16_t hx, ix;

	EXTRACT_LDBL80_WORDS(hx, lx, x);
	ix = hx & 0x7fff;
	INSERT_LDBL80_WORDS(ax, ix, lx);

	ENTERI();

	if (ix < 0x3fff) {			/* |x| < 1 */
		if (ix < 0x3ffd) {		/* |x| < 0.25 */
			if (ix < 0x3fdd) {	/* |x| < 0x1p-34 */
				if (x == 0)
					RETURNI(x);
				INSERT_LDBL80_WORDS(hi, hx,
				    lx & 0xffffffff00000000ull);
				hi *= 0x1p63L;
				lo = x * 0x1p63L - hi;
				s = (pi_lo + pi_hi) * lo + pi_lo * hi +
				    pi_hi * hi;
				RETURNI(s * 0x1p-63L);
			}
			s = __kernel_sinpil(ax);
			RETURNI((hx & 0x8000) ? -s : s);
		}

		if (ix < 0x3ffe)			/* |x| < 0.5 */
			s = __kernel_cospil(0.5 - ax);
		else if (lx < 0xc000000000000000ull)	/* |x| < 0.75 */
			s = __kernel_cospil(ax - 0.5);
		else
			s = __kernel_sinpil(1 - ax);
		RETURNI((hx & 0x8000) ? -s : s);
	}

	if (ix < 0x403e) {			/* 1 <= |x| < 0x1p63 */
		FFLOORL80(x, j0, ix, lx);	/* Integer part of ax. */
		ax -= x;
		EXTRACT_LDBL80_WORDS(ix, lx, ax);

		if (ix == 0) {
			s = 0;
		} else {
			if (ix < 0x3ffe) {		/* |x| < 0.5 */
				if (ix < 0x3ffd)	/* |x| < 0.25 */
					s = __kernel_sinpil(ax);
				else 
					s = __kernel_cospil(0.5 - ax);
			} else {
							/* |x| < 0.75 */
				if (lx < 0xc000000000000000ull)
					s = __kernel_cospil(ax - 0.5);
				else
					s = __kernel_sinpil(1 - ax);
			}

			if (j0 > 40)
				x -= 0x1p40;
			if (j0 > 30)
				x -= 0x1p30;
			j0 = (uint32_t)x;
			if (j0 & 1) s = -s;
		}
		RETURNI((hx & 0x8000) ? -s : s);
	}

	/* x = +-inf or nan. */
	if (ix >= 0x7fff)
		RETURNI(vzero / vzero);

	/*
	 * |x| >= 0x1p63 is always an integer, so return +-0.
	 */
	RETURNI(copysignl(0, x));
}

#undef pi_hi
#undef pi_lo

} /* namespace pbsd::lib_msun_ld80::b0093s3 */
