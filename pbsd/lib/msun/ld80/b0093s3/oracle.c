/*
 * PBSD batch b0093s3 -- reference oracle.
 *
 * The original HardenedBSD C source lib/msun/ld80/s_sinpil.c concatenated
 * verbatim.  Every function is renamed with a "ref_" prefix.  The renaming
 * is done with the preprocessor so that no function body is modified in any
 * way: only the token that names the function is substituted, the body text
 * is untouched.
 *
 * Support code that s_sinpil.c requires but that does not live in it (union
 * IEEEl2bits from lib/libc/amd64/_fpmath.h, macros from
 * lib/msun/src/math_private.h, and the kernels of lib/msun/ld80/k_sinl.c and
 * k_cosl.c) is reproduced verbatim first.  Only defines were added; no
 * function body was changed.  The k_*l() kernels are given external linkage
 * so that the C++ port resolves them against these very definitions.
 */

#include <float.h>
#include <math.h>
#include <stdint.h>

#ifndef LONG_BIT
#define	LONG_BIT	(8 * (int)sizeof(long))
#endif

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

/* ------------------------------------------------------------------ */
/* from lib/msun/ld80/k_sinl.c					    */
/* ------------------------------------------------------------------ */

static const double
half =  0.5;

#if defined(__amd64__) || defined(__i386__)
static const volatile double
S1hi = -0.16666666666666666,
S1lo = -9.2563760475949941e-18;
#define	S1	((long double)S1hi + S1lo)
#else
static const long double
S1 = -0.166666666666666666671L;
#endif

static const double
S2 =  0.0083333333333333332,
S3 = -0.00019841269841269427,
S4 =  0.0000027557319223597490,
S5 = -0.000000025052108218074604,
S6 =  1.6059006598854211e-10,
S7 = -7.6429779983024564e-13,
S8 =  2.6174587166648325e-15;

long double
__kernel_sinl(long double x, long double y, int iy)
{
	long double z,r,v;

	z	=  x*x;
	v	=  z*x;
	r	=  S2+z*(S3+z*(S4+z*(S5+z*(S6+z*(S7+z*S8)))));
	if(iy==0) return x+v*(S1+z*r);
	else      return x-((z*(half*y-v*r)-y)-v*S1);
}

/* ------------------------------------------------------------------ */
/* from lib/msun/ld80/k_cosl.c					    */
/* ------------------------------------------------------------------ */

static const double
one = 1.0;

#if defined(__amd64__) || defined(__i386__)
static const volatile double
C1hi = 0.041666666666666664,
C1lo = 2.2598839032744733e-18;
#define	C1	((long double)C1hi + C1lo)
#else
static const long double
C1 =  0.0416666666666666666136L;
#endif

static const double
C2 = -0.0013888888888888874,
C3 =  0.000024801587301571716,
C4 = -0.00000027557319215507120,
C5 =  0.0000000020876754400407278,
C6 = -1.1470297442401303e-11,
C7 =  4.7383039476436467e-14;

long double
__kernel_cosl(long double x, long double y)
{
	long double hz,z,r,w;

	z  = x*x;
	r  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*(C6+z*C7))))));
	hz = 0.5*z;
	w  = one-hz;
	return w + (((one-w)-hz) + (z*r-x*y));
}

/* ================================================================== */
/* lib/msun/ld80/s_sinpil.c					      */
/* ================================================================== */

#define	pi_hi_u			ref_sinpil_pi_hi_u
#define	pi_lo_u			ref_sinpil_pi_lo_u
#define	__kernel_cospil		ref_sinpil_kernel_cospil
#define	__kernel_sinpil		ref_sinpil_kernel_sinpil
#define	vzero			ref_sinpil_vzero
#define	sinpil			ref_sinpil

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

long double
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
#undef pi_hi_u
#undef pi_lo_u
#undef __kernel_cospil
#undef __kernel_sinpil
#undef vzero
#undef sinpil
