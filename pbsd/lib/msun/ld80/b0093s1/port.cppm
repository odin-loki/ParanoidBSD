/*
 * PBSD batch b0093s1 -- C++23 port of
 *
 *	lib/msun/ld80/s_cexpl.c
 *
 * The body is transcribed unchanged, down to the evaluation order, the
 * mixed double/long double arithmetic and the integer types and signedness.
 *
 * __ldexp_cexpl() comes from ld80/k_expl.h; it does not belong to this
 * batch, so it is declared here exactly as msun declares it and resolved at
 * link time.
 */

module;

#include <cstdint>

export module pbsd.lib.msun.ld80.b0093s1;

namespace pbsd::lib_msun_ld80::b0093s1 {

using std::uint16_t;
using std::uint64_t;

/* <complex.h> spells the C imaginary types this way. */
#define	complex		_Complex

extern "C" {
long double expl(long double);
void sincosl(long double, long double *, long double *);

/* lib/msun/ld80/k_expl.h */
long double complex __ldexp_cexpl(long double complex, int);
}

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

} /* namespace pbsd::lib_msun_ld80::b0093s1 */
