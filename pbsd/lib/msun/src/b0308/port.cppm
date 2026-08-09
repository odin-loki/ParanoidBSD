module;

#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <math.h>

export module pbsd.lib.msun.src.b0308;

typedef union {
	float value;
	unsigned int word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i, d)					\
	do {							\
		ieee_float_shape_type gf_u;			\
		gf_u.value = (d);				\
		(i) = gf_u.word;				\
	} while (0)

#define SET_FLOAT_WORD(d, i)					\
	do {							\
		ieee_float_shape_type sf_u;			\
		sf_u.word = (i);				\
		(d) = sf_u.value;				\
	} while (0)

namespace pbsd::lib_msun_src::b0308::detail {

union IEEEd2bits {
	double	d;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
	} bits;
};

#if LDBL_MANT_DIG == 64
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
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)
#elif LDBL_MANT_DIG == 113
union IEEEl2bits {
	long double	e;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
	} bits;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	expsign	:16;
	} xbits;
};
#define	LDBL_NBIT	0x8000000000000000ULL
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)
#else
#error "Unsupported long double format"
#endif

} // namespace pbsd::lib_msun_src::b0308::detail

namespace pbsd::lib_msun_src::b0308 {

/* e_acosf.c -- float version of e_acos.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

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

static const float
one =  1.0000000000e+00, /* 0x3F800000 */
pi =  3.1415925026e+00, /* 0x40490fda */
pio2_hi =  1.5707962513e+00; /* 0x3fc90fda */
static volatile float
pio2_lo =  7.5497894159e-08; /* 0x33a22168 */

/*
 * The coefficients for the rational approximation were generated over
 *  0x1p-12f <= x <= 0.5f.  The maximum error satisfies log2(e) < -30.084.
 */
static const float
pS0 =  1.66666672e-01f, /* 0x3e2aaaab */
pS1 = -1.19510300e-01f, /* 0xbdf4c1d1 */
pS2 =  5.47002675e-03f, /* 0x3bb33de9 */
qS1 = -1.16706085e+00f, /* 0xbf956240 */
qS2 =  2.90115148e-01f; /* 0x3e9489f9 */

export float
acosf(float x)
{
	float z,p,q,r,w,s,c,df;
	std::int32_t hx,ix;
	GET_FLOAT_WORD(hx,x);
	ix = hx&0x7fffffff;
	if(ix>=0x3f800000) {		/* |x| >= 1 */
	    if(ix==0x3f800000) {	/* |x| == 1 */
		if(hx>0) return 0.0;	/* acos(1) = 0 */
		else return pi+(float)2.0*pio2_lo;	/* acos(-1)= pi */
	    }
	    return (x-x)/(x-x);		/* acos(|x|>1) is NaN */
	}
	if(ix<0x3f000000) {	/* |x| < 0.5 */
	    if(ix<=0x32800000) return pio2_hi+pio2_lo;/*if|x|<2**-26*/
	    z = x*x;
	    p = z*(pS0+z*(pS1+z*pS2));
	    q = one+z*(qS1+z*qS2);
	    r = p/q;
	    return pio2_hi - (x - (pio2_lo-x*r));
	} else  if (hx<0) {		/* x < -0.5 */
	    z = (one+x)*(float)0.5;
	    p = z*(pS0+z*(pS1+z*pS2));
	    q = one+z*(qS1+z*qS2);
	    s = sqrtf(z);
	    r = p/q;
	    w = r*s-pio2_lo;
	    return pi - (float)2.0*(s+w);
	} else {			/* x > 0.5 */
	    std::int32_t idf;
	    z = (one-x)*(float)0.5;
	    s = sqrtf(z);
	    df = s;
	    GET_FLOAT_WORD(idf,df);
	    SET_FLOAT_WORD(df,idf&0xfffff000);
	    c  = (z-df*df)/(s+df);
	    p = z*(pS0+z*(pS1+z*pS2));
	    q = one+z*(qS1+z*qS2);
	    r = p/q;
	    w = r*s+c;
	    return (float)2.0*(df+w);
	}
}

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
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
 */

export double
fmaximum_mag(double x, double y)
{
	detail::IEEEd2bits u[2];

	u[0].d = x;
	u[1].d = y;

	/* Handle NaN according to ISO/IEC 60559. NaN argument -> NaN return */
	if ((u[0].bits.exp == 2047 && (u[0].bits.manh | u[0].bits.manl) != 0) ||
	    (u[1].bits.exp == 2047 && (u[1].bits.manh | u[1].bits.manl) != 0))
		return (NAN);

	double ax = fabs(x);
	double ay = fabs(y);

	if (ay > ax)
		return (y);
	if (ax > ay)
		return (x);

	/* If magnitudes are equal, we break the tie with the sign */
	if (u[0].bits.sign != u[1].bits.sign)
		return (u[u[0].bits.sign].d);

	return (x);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
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
 */

/*
 * We save and restore the floating-point environment to avoid raising
 * an inexact exception.  We can get away with using fesetenv()
 * instead of feclearexcept()/feupdateenv() to restore the environment
 * because the only exception defined for rint() is overflow, and
 * rounding can't overflow as long as emax >= p.
 *
 * The volatile keyword is needed below because clang incorrectly assumes
 * that rint won't raise any floating-point exceptions. Declaring ret volatile
 * is sufficient to trick the compiler into doing the right thing.
 */

export double
nearbyint(double x)
{
	volatile double ret;
	fenv_t env;

	fegetenv(&env);
	ret = rint(x);
	fesetenv(&env);
	return (ret);
}

export float
nearbyintf(float x)
{
	volatile float ret;
	fenv_t env;

	fegetenv(&env);
	ret = rintf(x);
	fesetenv(&env);
	return (ret);
}

export long double
nearbyintl(long double x)
{
	volatile long double ret;
	fenv_t env;

	fegetenv(&env);
	ret = rintl(x);
	fesetenv(&env);
	return (ret);
}

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
 * Copyright (c) 2026 Jesús Blázquez <jesuscblazquez@gmail.com>
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
 */

export long double
fmaximum_numl(long double x, long double y)
{
	detail::IEEEl2bits u[2];
	bool nan_x, nan_y;

	u[0].e = x;
	mask_nbit_l(u[0]);
	u[1].e = y;
	mask_nbit_l(u[1]);

	nan_x = u[0].bits.exp == 32767 && (u[0].bits.manh | u[0].bits.manl) != 0;
	nan_y = u[1].bits.exp == 32767 && (u[1].bits.manh | u[1].bits.manl) != 0;

	if (nan_x || nan_y) {
		/* If both are NaN, adding returns qNaN */
		if (nan_x && nan_y)
		    return (x + y);

		/* force_except makes sure sNaN's raise exceptions */
		volatile long double force_except = x + y;
		force_except;

		if (nan_x)
			return (y);
		else
			return (x);
	}

	/* Handle comparisons of signed zeroes. */
	if (u[0].bits.sign != u[1].bits.sign)
		return (u[0].bits.sign ? y : x);

	return (x > y ? x : y);
}

} // namespace pbsd::lib_msun_src::b0308
