/*
 * PBSD batch b0093 -- C++23 port of
 *
 *	lib/msun/ld80/s_cexpl.c
 *	lib/msun/ld80/s_cospil.c
 *	lib/msun/ld80/s_sinpil.c
 *	lib/msun/ld80/s_tanpil.c
 *
 * The bodies are transcribed unchanged, down to the evaluation order, the
 * mixed double/long double arithmetic and the integer types and signedness.
 *
 * s_cospil.c, s_sinpil.c and s_tanpil.c each define their own pi_hi/pi_lo,
 * and s_sinpil.c's are long double while the other two are double; each also
 * gets its own copy of the static inline kernels of ld80/k_cospil.h and
 * ld80/k_sinpil.h, which therefore compute in a different precision per
 * file.  That is load bearing, so every file gets a private namespace here
 * holding its own constants, its own kernels and its own vzero.
 *
 * __kernel_sinl(), __kernel_cosl() and __kernel_tanl() are separate
 * translation units of libm (ld80/k_sinl.c, k_cosl.c, k_tanl.c) which
 * math_private.h merely declares, and __ldexp_cexpl() comes from
 * ld80/k_expl.h; none of them belongs to this batch, so they are declared
 * here exactly as msun declares them and resolved at link time.
 */

module;

#include <cstdint>

export module pbsd.lib.msun.ld80.b0093;

namespace pbsd::lib_msun_ld80::b0093 {

using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

/* <complex.h> spells the C imaginary types this way. */
#define	complex		_Complex

extern "C" {
long double expl(long double);
long double fabsl(long double);
long double copysignl(long double, long double);
void sincosl(long double, long double *, long double *);

/* lib/msun/src/math_private.h */
long double __kernel_sinl(long double, long double, int);
long double __kernel_cosl(long double, long double);
long double __kernel_tanl(long double, long double, int);

/* lib/msun/ld80/k_expl.h */
long double complex __ldexp_cexpl(long double complex, int);
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

/*
 * The non-__i386__ spelling: "The above works on non-i386 too, but we use
 * this to check v."
 */
#define	LD80C(m, ex, v)	{ .e = (v), }

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

/*
 * math_private.h builds a long double complex out of its two parts through
 * a union; __real__/__imag__ is the same store.
 */
static inline long double complex
CMPLXL(long double x, long double y)
{
	long double complex z;

	__real__ z = x;
	__imag__ z = y;
	return (z);
}

#define	creall(z)	(__real__ (z))
#define	cimagl(z)	(__imag__ (z))

/* ================================================================== */
/* lib/msun/ld80/s_cexpl.c					      */
/* ================================================================== */

namespace s_cexpl {

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * src/s_cexp.c converted to long double complex by Steven G. Kargl
 */

export long double complex
cexpl (long double complex z)
{
	long double c, exp_x, s, x, y;
	uint64_t lx, ly;
	uint16_t hx, hy;

	ENTERI();

	x = creall(z);
	y = cimagl(z);

	EXTRACT_LDBL80_WORDS(hy, ly, y);
	hy &= 0x7fff;

	/* cexp(x + I 0) = exp(x) + I 0 */
	if ((hy | ly) == 0)
		RETURNI(CMPLXL(expl(x), y));
	EXTRACT_LDBL80_WORDS(hx, lx, x);
	/* cexp(0 + I y) = cos(y) + I sin(y) */
	if (((hx & 0x7fff) | lx) == 0) {
		sincosl(y, &s, &c);
		RETURNI(CMPLXL(c, s));
	}

	if (hy >= 0x7fff) {
		if ((hx & 0x7fff) < 0x7fff || ((hx & 0x7fff) == 0x7fff &&
		    (lx & 0x7fffffffffffffffULL) != 0)) {
			/* cexp(finite|NaN +- I Inf|NaN) = NaN + I NaN */
			RETURNI(CMPLXL(y - y, y - y));
		} else if (hx & 0x8000) {
			/* cexp(-Inf +- I Inf|NaN) = 0 + I 0 */
			RETURNI(CMPLXL(0.0, 0.0));
		} else {
			/* cexp(+Inf +- I Inf|NaN) = Inf + I NaN */
			RETURNI(CMPLXL(x, y - y));
		}
	}

	/*
	 *  exp_ovfl = 11356.5234062941439497
	 * cexp_ovfl = 22755.3287906024445633
	 */
	if ((hx == 0x400c && lx > 0xb17217f7d1cf79acULL) ||
	    (hx == 0x400d && lx < 0xb1c6a8573de9768cULL)) {
		/*
		 * x is between exp_ovfl and cexp_ovfl, so we must scale to
		 * avoid overflow in exp(x).
		 */
		RETURNI(__ldexp_cexpl(z, 0));
	} else {
		/*
		 * Cases covered here:
		 *  -  x < exp_ovfl and exp(x) won't overflow (common case)
		 *  -  x > cexp_ovfl, so exp(x) * s overflows for all s > 0
		 *  -  x = +-Inf (generated by exp())
		 *  -  x = NaN (spurious inexact exception from y)
		 */
		exp_x = expl(x);
		sincosl(y, &s, &c);
		RETURNI(CMPLXL(exp_x * c, exp_x * s));
	}
}

} /* namespace s_cexpl */

/* ================================================================== */
/* lib/msun/ld80/s_cospil.c					      */
/* ================================================================== */

namespace s_cospil {

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

} /* namespace s_cospil */

/* ================================================================== */
/* lib/msun/ld80/s_sinpil.c					      */
/* ================================================================== */

namespace s_sinpil {

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

} /* namespace s_sinpil */

/* ================================================================== */
/* lib/msun/ld80/s_tanpil.c					      */
/* ================================================================== */

namespace s_tanpil {

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
 * See ../src/s_tanpi.c for implementation details.
 */

static const double
pi_hi =  3.1415926814079285e+00,	/* 0x400921fb 0x58000000 */
pi_lo = -2.7818135228334233e-08;	/* 0xbe5dde97 0x3dcb3b3a */

static inline long double
__kernel_tanpil(long double x)
{
	long double hi, lo, t;

	if (x < 0.25) {
		hi = (float)x;
		lo = x - hi;
		lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
		hi *= pi_hi;
		_2sumF(hi, lo);
		t = __kernel_tanl(hi, lo, -1);
	} else if (x > 0.25) {
		x = 0.5 - x;
		hi = (float)x;
		lo = x - hi;
		lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
		hi *= pi_hi;
		_2sumF(hi, lo);
		t = - __kernel_tanl(hi, lo, 1);
	} else
		t = 1;

	return (t);
}

volatile static const double vzero = 0;

export long double
tanpil(long double x)
{
	long double ax, hi, lo, odd, t;
	uint64_t lx, m;
	uint32_t j0;
	uint16_t hx, ix;

	EXTRACT_LDBL80_WORDS(hx, lx, x);
	ix = hx & 0x7fff;
	INSERT_LDBL80_WORDS(ax, ix, lx);

	ENTERI();

	if (ix < 0x3fff) {			/* |x| < 1 */
		if (ix < 0x3ffe) {		/* |x| < 0.5 */
			if (ix < 0x3fdd) {	/* |x| < 0x1p-34 */
				if (x == 0)
					RETURNI(x);
				INSERT_LDBL80_WORDS(hi, hx,
				    lx & 0xffffffff00000000ull);
				hi *= 0x1p63L;
				lo = x * 0x1p63L - hi;
				t = (pi_lo + pi_hi) * lo + pi_lo * hi +
				    pi_hi * hi;
				RETURNI(t * 0x1p-63L);
			}
			t = __kernel_tanpil(ax);
		} else if (ax == 0.5)
			t = 1 / vzero;
		else
			t = -__kernel_tanpil(1 - ax);
		RETURNI((hx & 0x8000) ? -t : t);
	}

	if (ix < 0x403e) {			/* 1 <= |x| < 0x1p63 */
		FFLOORL80(x, j0, ix, lx);	/* Integer part of ax. */
		odd = (uint64_t)x & 1 ? -1 : 1;
		ax -= x;
		EXTRACT_LDBL80_WORDS(ix, lx, ax);

		if (ix < 0x3ffe)		/* |x| < 0.5 */
			t = ix == 0 ? copysignl(0, odd) : __kernel_tanpil(ax);
		else if (ax == 0.5L)
			t = odd / vzero;
		else
			t = -__kernel_tanpil(1 - ax);
		RETURNI((hx & 0x8000) ? -t : t);
	}

	/* x = +-inf or nan. */
	if (ix >= 0x7fff)
		RETURNI(vzero / vzero);

	/*
	 * For 0x1p63 <= |x| < 0x1p64 need to determine if x is an even
	 * or odd integer to set t = +0 or -0.
	 * For |x| >= 0x1p64, it is always an even integer, so t = 0.
	 */
	t = ix >= 0x403f ? 0 : (copysignl(0, (lx & 1) ? -1 : 1));
	RETURNI((hx & 0x8000) ? -t : t);
}

} /* namespace s_tanpil */

} /* namespace pbsd::lib_msun_ld80::b0093 */

export namespace pbsd::lib_msun_ld80::b0093 {

using s_cexpl::cexpl;
using s_cospil::cospil;
using s_sinpil::sinpil;
using s_tanpil::tanpil;

}
