/*
 * PBSD batch b0093s2 -- C++23 port of lib/msun/ld80/s_cospil.c
 *
 * The body is transcribed unchanged, down to the evaluation order, the mixed
 * double/long double arithmetic and the integer types and signedness.
 *
 * s_cospil.c defines its own pi_hi/pi_lo as double, and pulls in its own copy
 * of the static inline kernels of ld80/k_cospil.h and ld80/k_sinpil.h.
 *
 * __kernel_sinl() and __kernel_cosl() are separate translation units of libm
 * (ld80/k_sinl.c, k_cosl.c) which math_private.h merely declares; they are
 * declared here exactly as msun declares them and resolved at link time.
 */

module;

#include <cstdint>

export module pbsd.lib.msun.ld80.b0093s2;

namespace pbsd::lib_msun_ld80::b0093s2 {

using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

extern "C" {
/* lib/msun/src/math_private.h */
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

#define	LDBL_NBIT	0x80000000

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

/* Support switching the mode to FP_PE if necessary; not __i386__ here. */
#define	ENTERI()
#define	RETURNI(x)	RETURNF(x)

/* Default return statement if hack*_t() is not used. */
#define      RETURNF(v)      return (v)

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
/* lib/msun/ld80/s_cospil.c					      */
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
 * See ../src/s_cospi.c for implementation details.
 */

static const double
pi_hi = 3.1415926814079285e+00,	/* 0x400921fb 0x58000000 */
pi_lo =-2.7818135228334233e-08;	/* 0xbe5dde97 0x3dcb3b3a */

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
cospil(long double x)
{
	long double ax, c;
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
				if ((int)x == 0)
					RETURNI(1);
			}
			RETURNI(__kernel_cospil(ax));
		}

		if (ix < 0x3ffe)			/* |x| < 0.5 */
			c = __kernel_sinpil(0.5 - ax);
		else if (lx < 0xc000000000000000ull) {	/* |x| < 0.75 */
			if (ax == 0.5)
				RETURNI(0);
			c = -__kernel_sinpil(ax - 0.5);
		} else
			c = -__kernel_cospil(1 - ax);
		RETURNI(c);
	}

	if (ix < 0x403e) {			/* 1 <= |x| < 0x1p63 */
		FFLOORL80(x, j0, ix, lx);	/* Integer part of ax. */
		ax -= x;
		EXTRACT_LDBL80_WORDS(ix, lx, ax);

		if (ix < 0x3ffe) {			/* |x| < 0.5 */
			if (ix < 0x3ffd)		/* |x| < 0.25 */
				c = ix == 0 ? 1 : __kernel_cospil(ax);
			else
				c = __kernel_sinpil(0.5 - ax);

		} else {
			if (lx < 0xc000000000000000ull) { /* |x| < 0.75 */
				if (ax == 0.5)
					RETURNI(0);
				c = -__kernel_sinpil(ax - 0.5);
			} else
				c = -__kernel_cospil(1 - ax);
		}

		if (j0 > 40)
			x -= 0x1p40;
		if (j0 > 30)
			x -= 0x1p30;
		j0 = (uint32_t)x;

		RETURNI(j0 & 1 ? -c : c);
	}

	if (ix >= 0x7fff)
		RETURNI(vzero / vzero);

	/*
	 * For 0x1p63 <= |x| < 0x1p64 need to determine if x is an even
	 * or odd integer to return t = +1 or -1.
	 * For |x| >= 0x1p64, it is always an even integer, so t = 1.
	 */
	RETURNI(ix >= 0x403f ? 1 : ((lx & 1) ? -1 : 1));
}

} /* namespace pbsd::lib_msun_ld80::b0093s2 */
