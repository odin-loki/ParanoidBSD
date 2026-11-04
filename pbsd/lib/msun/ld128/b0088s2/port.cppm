/*
 * PBSD batch b0088s2 -- C++23 port of the HardenedBSD ld128 sinpil() family.
 *
 * Sources, in order:
 *	lib/msun/ld128/k_cosl.c
 *	lib/msun/ld128/k_sinl.c
 *	lib/msun/ld128/k_cospil.h
 *	lib/msun/ld128/k_sinpil.h
 *	lib/msun/ld128/s_sinpil.c		(the batch)
 *
 * Supporting definitions reproduced verbatim from headers outside the batch:
 *	union IEEEl2bits	lib/libc/aarch64/_fpmath.h
 *	_2sumF			lib/msun/src/math_private.h  (!DEBUG variant)
 *	FFLOORL128		lib/msun/src/math_private.h
 *
 * These are the ld128 sources: `long double' must be IEEE 754 binary128,
 * which is what union IEEEl2bits describes.  On x86-64 the default long
 * double is x87 80-bit, so this file must be compiled with
 * -mlong-double-128; the check below enforces it.
 *
 * The port is behaviour-preserving down to the last bit: expression shape,
 * operand order, the double/long double mixing, the (double) narrowing casts
 * and the exact branch structure are all unchanged.
 */

module;

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <math.h>
#include <quadmath.h>

#if LDBL_MANT_DIG != 113
#error "b0088s2 is ld128 code: compile with -mlong-double-128"
#endif

/*
 * With -mlong-double-128 the compiler still emits a call to the host libm's
 * fmodl(), which on this platform is the x87 80-bit entry point.  fmodq() is
 * the binary128 entry point, i.e. what fmodl() is on a real ld128 platform.
 */
#define	fmodl	fmodq

export module pbsd.lib.msun.ld128.b0088s2;

namespace pbsd::lib_msun_ld128::b0088s2 {

/* lib/libc/aarch64/_fpmath.h */
union IEEEl2bits {
	long double	e;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
	} bits;
	/* TODO andrew: Check the packing here */
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	expsign	:16;
	} xbits;
};

/* lib/msun/src/math_private.h */
#define	_2sumF(a, b) do {	\
	__typeof__(a) __w;	\
				\
	__w = (a) + (b);	\
	(b) = ((a) - __w) + (b); \
	(a) = __w;		\
} while (0)

/* lib/msun/src/math_private.h */
#define FFLOORL128(x, ai, ar) do {			\
	union IEEEl2bits u;				\
	std::uint64_t m;				\
	int e;						\
	u.e = (x);					\
	e = u.bits.exp - 16383;				\
	if (e < 48) {					\
		m = ((1llu << 49) - 1) >> (e + 1);	\
		u.bits.manh &= ~m;			\
		u.bits.manl = 0;			\
	} else {					\
		m = (std::uint64_t)-1 >> (e - 48);	\
		u.bits.manl &= ~m;			\
	}						\
	(ai) = u.e;					\
	(ar) = (x) - (ai);				\
} while (0)

/* ------------------------------------------------------------------ */
/* lib/msun/ld128/k_cosl.c						*/
/* ------------------------------------------------------------------ */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
 * ld128 version of k_cos.c.  See ../src/k_cos.c for most comments.
 */

/*
 * Domain [-0.7854, 0.7854], range ~[-1.17e-39, 1.19e-39]:
 * |cos(x) - c(x))| < 2**-129.3
 *
 * 113-bit precision requires more care than 64-bit precision, since
 * simple methods give a minimax polynomial with coefficient for x^2
 * that is 1 ulp below 0.5, but we want it to be precisely 0.5.  See
 * ../ld80/k_cosl.c for more details.
 */
static const double
one = 1.0;
static const long double
C1 =  4.16666666666666666666666666666666667e-02L,
C2 = -1.38888888888888888888888888888888834e-03L,
C3 =  2.48015873015873015873015873015446795e-05L,
C4 = -2.75573192239858906525573190949988493e-07L,
C5 =  2.08767569878680989792098886701451072e-09L,
C6 = -1.14707455977297247136657111139971865e-11L,
C7 =  4.77947733238738518870113294139830239e-14L,
C8 = -1.56192069685858079920640872925306403e-16L,
C9 =  4.11031762320473354032038893429515732e-19L,
C10= -8.89679121027589608738005163931958096e-22L,
C11=  1.61171797801314301767074036661901531e-24L,
C12= -2.46748624357670948912574279501044295e-27L;

export long double
__kernel_cosl(long double x, long double y)
{
	long double hz,z,r,w;

	z  = x*x;
	r  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*(C6+z*(C7+
	    z*(C8+z*(C9+z*(C10+z*(C11+z*C12)))))))))));
	hz = 0.5*z;
	w  = one-hz;
	return w + (((one-w)-hz) + (z*r-x*y));
}

/* ------------------------------------------------------------------ */
/* lib/msun/ld128/k_sinl.c						*/
/* ------------------------------------------------------------------ */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
 * ld128 version of k_sin.c.  See ../src/k_sin.c for most comments.
 */

static const double
half =  0.5;

/*
 * Domain [-0.7854, 0.7854], range ~[-1.53e-37, 1.659e-37]
 * |sin(x)/x - s(x)| < 2**-122.1
 *
 * See ../ld80/k_cosl.c for more details about the polynomial.
 */
static const long double
S1 = -0.16666666666666666666666666666666666606732416116558L,
S2 =  0.0083333333333333333333333333333331135404851288270047L,
S3 = -0.00019841269841269841269841269839935785325638310428717L,
S4 =  0.27557319223985890652557316053039946268333231205686e-5L,
S5 = -0.25052108385441718775048214826384312253862930064745e-7L,
S6 =  0.16059043836821614596571832194524392581082444805729e-9L,
S7 = -0.76471637318198151807063387954939213287488216303768e-12L,
S8 =  0.28114572543451292625024967174638477283187397621303e-14L;

static const double
S9  = -0.82206352458348947812512122163446202498005154296863e-17,
S10 =  0.19572940011906109418080609928334380560135358385256e-19,
S11 = -0.38680813379701966970673724299207480965452616911420e-22,
S12 =  0.64038150078671872796678569586315881020659912139412e-25;

export long double
__kernel_sinl(long double x, long double y, int iy)
{
	long double z,r,v;

	z	=  x*x;
	v	=  z*x;
	r	=  S2+z*(S3+z*(S4+z*(S5+z*(S6+z*(S7+z*(S8+
	    z*(S9+z*(S10+z*(S11+z*S12)))))))));
	if(iy==0) return x+v*(S1+z*r);
	else      return x-((z*(half*y-v*r)-y)-v*S1);
}

/*
 * pi_hi contains the leading 56 bits of a 169 bit approximation for pi.
 * (from lib/msun/ld128/s_sinpil.c, hoisted here because k_cospil.h and
 * k_sinpil.h are included by s_sinpil.c after it defines these.)
 */
static const long double
pi_hi = 3.14159265358979322702026593105983920e+00L,
pi_lo = 1.14423774522196636802434264184180742e-17L;

/* ------------------------------------------------------------------ */
/* lib/msun/ld128/k_cospil.h						*/
/* ------------------------------------------------------------------ */
/*-
 * Copyright (c) 2017 Steven G. Kargl
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
 * See ../src/k_cospi.c for implementation details.
 */

/* `static inline' dropped so the harness can call this directly. */
export long double
__kernel_cospil(long double x)
{
	long double hi, lo;

	hi = (double)x;
	lo = x - hi;
	lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
	hi *= pi_hi;
	_2sumF(hi, lo);
	return (__kernel_cosl(hi, lo));
}

/* ------------------------------------------------------------------ */
/* lib/msun/ld128/k_sinpil.h						*/
/* ------------------------------------------------------------------ */
/*-
 * Copyright (c) 2017 Steven G. Kargl
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
 * See ../src/k_sinpi.c for implementation details.
 */

/* `static inline' dropped so the harness can call this directly. */
export long double
__kernel_sinpil(long double x)
{
	long double hi, lo;

	hi = (double)x;
	lo = x - hi;
	lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
	hi *= pi_hi;
	_2sumF(hi, lo);
	return (__kernel_sinl(hi, lo, 1));
}

/* ------------------------------------------------------------------ */
/* lib/msun/ld128/s_sinpil.c						*/
/* ------------------------------------------------------------------ */
/*-
 * Copyright (c) 2017-2023 Steven G. Kargl
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

volatile static const double vzero = 0;

export long double
sinpil(long double x)
{
	long double ai, ar, ax, hi, lo, s, xhi, xlo;

	ax = fabsl(x);

	if (ax < 1) {
		if (ax < 0.25) {
			if (ax < 0x1p-60) {
				if (x == 0)
					return (x);
				hi = (double)x;
				hi *= 0x1p113L;
				lo = x * 0x1p113L - hi;
				s = (pi_lo + pi_hi) * lo + pi_lo * lo +
				    pi_hi * hi;
				return (s * 0x1p-113L);
			}

			s = __kernel_sinpil(ax);
			return (x < 0 ? -s : s);
		}

		if (ax < 0.5)
			s = __kernel_cospil(0.5 - ax);
		else if (ax < 0.75)
			s = __kernel_cospil(ax - 0.5);
		else
			s = __kernel_sinpil(1 - ax);
		return (x < 0 ? -s : s);
	}

	if (ax < 0x1p112) {
		/* Split ax = ai + ar with 0 <= ar < 1. */
		FFLOORL128(ax, ai, ar);

		if (ar == 0) {
			s = 0;
		} else {
			if (ar < 0.5) {
				if (ar <= 0.25)
					s = __kernel_sinpil(ar);
				else
					s = __kernel_cospil(0.5 - ar);
			} else {
				if (ar < 0.75)
					s = __kernel_cospil(ar - 0.5);
				else
					s = __kernel_sinpil(1 - ar);
			}

			s = fmodl(ai, 2.L) == 0 ? s : -s;
		}
		return (x < 0 ? -s : s);
	}

	if (isinf(x) || isnan(x))
		return (vzero / vzero);

	/*
	 * |x| >= 0x1p112 is always an integer, so return +-0.
	 */
	return (copysignl(0, x));
}

} // namespace pbsd::lib_msun_ld128::b0088s2
