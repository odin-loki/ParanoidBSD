module;

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>

export module pbsd.lib.msun.src.b0262;

namespace pbsd::lib_msun_src::b0262 {

/* from lib/msun/src/math_private.h */
typedef union {
	__complex__ long double f;
	long double a[2];
} long_double_complex;

#define	IMAGPART(z)	((z).a[1])

typedef union {
	double value;
	struct {
		std::uint32_t lsw;
		std::uint32_t msw;
	} parts;
} ieee_double_shape_type;

#define GET_HIGH_WORD(i, d)				\
do {							\
	ieee_double_shape_type gh_u;			\
	gh_u.value = (d);				\
	(i) = gh_u.parts.msw;				\
} while (0)

/* from lib/libc/amd64/_fpmath.h */
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
		unsigned long long	man	:64;
		unsigned int		expsign	:16;
		unsigned long long	junk	:48;
	} xbits;
};

#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32

/* amd64/i386 use the explicit-integer-bit format: no LDBL_IMPLICIT_NBIT. */

#ifdef LDBL_IMPLICIT_NBIT
#define	MANH_SIZE	(LDBL_MANH_SIZE + 1)
#else
#define	MANH_SIZE	LDBL_MANH_SIZE
#endif

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 Stefan Farfeleder
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
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

using long_double_complex_t = __complex__ long double;

export long double
cimagl(long_double_complex_t z)
{
	const long_double_complex z1 = { .f = z };

	return (IMAGPART(z1));
}

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

namespace truncl_statics {

static const long double huge = 1.0e300;
static const float zero[] = { 0.0, -0.0 };

} // namespace truncl_statics

#define huge truncl_statics::huge
#define zero truncl_statics::zero

export long double
truncl(long double x)
{
	union IEEEl2bits u = { .e = x };
	int e = u.bits.exp - LDBL_MAX_EXP + 1;

	if (e < MANH_SIZE - 1) {
		if (e < 0) {			/* raise inexact if x != 0 */
			if (huge + x > 0.0)
				u.e = zero[u.bits.sign];
		} else {
			uint64_t m = ((1llu << MANH_SIZE) - 1) >> (e + 1);
			if (((u.bits.manh & m) | u.bits.manl) == 0)
				return (x);	/* x is integral */
			if (huge + x > 0.0) {	/* raise inexact flag */
				u.bits.manh &= ~m;
				u.bits.manl = 0;
			}
		}
	} else if (e < LDBL_MANT_DIG - 1) {
		uint64_t m = (uint64_t)-1 >> (64 - LDBL_MANT_DIG + e + 1);
		if ((u.bits.manl & m) == 0)
			return (x);	/* x is integral */
		if (huge + x > 0.0)		/* raise inexact flag */
			u.bits.manl &= ~m;
	}
	return (u.e);
}

#undef huge
#undef zero

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

namespace {

static const double
one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
ln2 =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
huge=  1.00000000000000000000e+300;

} // namespace

export double
asinh(double x)
{
	double t,w;
	int32_t hx,ix;
	GET_HIGH_WORD(hx,x);
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) return x+x;	/* x is inf or NaN */
	if(ix< 0x3e300000) {	/* |x|<2**-28 */
	    if(huge+x>one) return x;	/* return x inexact except 0 */
	}
	if(ix>0x41b00000) {	/* |x| > 2**28 */
	    w = log(fabs(x))+ln2;
	} else if (ix>0x40000000) {	/* 2**28 > |x| > 2.0 */
	    t = fabs(x);
	    w = log(2.0*t+one/(sqrt(x*x+one)+t));
	} else {		/* 2.0 > |x| > 2**-28 */
	    t = x*x;
	    w =log1p(fabs(x)+t/(one+sqrt(one+t)));
	}
	if(hx>0) return w; else return -w;
}

} // namespace pbsd::lib_msun_src::b0262
