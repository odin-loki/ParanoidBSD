module;

#include <cfloat>
#include <math.h>
#include <stdint.h>

export module pbsd.lib.msun.src.b0309;

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

namespace pbsd::lib_msun_src::b0309::detail {

union IEEEf2bits {
	float	f;
	struct {
		unsigned int	man	:23;
		unsigned int	exp	:8;
		unsigned int	sign	:1;
	} bits;
};

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

} // namespace pbsd::lib_msun_src::b0309::detail

namespace pbsd::lib_msun_src::b0309 {

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

export long double
fmaximum_magl(long double x, long double y)
{
	detail::IEEEl2bits u[2];

	u[0].e = x;
	mask_nbit_l(u[0]);
	u[1].e = y;
	mask_nbit_l(u[1]);

	/* Handle NaN according to ISO/IEC 60559. NaN argument -> NaN return */
	if ((u[0].bits.exp == 32767 && (u[0].bits.manh | u[0].bits.manl) != 0) ||
	    (u[1].bits.exp == 32767 && (u[1].bits.manh | u[1].bits.manl) != 0))
		return (NAN);

	long double ax = fabsl(x);
	long double ay = fabsl(y);

	if (ay > ax)
		return (y);
	if (ax > ay)
		return (x);

	/* If magnitudes are equal, we break the tie with the sign */
	if (u[0].bits.sign != u[1].bits.sign)
		return (u[0].bits.sign ? y : x);

	return (x);
}

/* e_hypotf.c -- float version of e_hypot.c.
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

export float
hypotf(float x, float y)
{
	float a,b,t1,t2,y1,y2,w;
	int32_t j,k,ha,hb;

	GET_FLOAT_WORD(ha,x);
	ha &= 0x7fffffff;
	GET_FLOAT_WORD(hb,y);
	hb &= 0x7fffffff;
	if(hb > ha) {a=y;b=x;j=ha; ha=hb;hb=j;} else {a=x;b=y;}
	a = fabsf(a);
	b = fabsf(b);
	if((ha-hb)>0xf000000) {return a+b;} /* x/y > 2**30 */
	k=0;
	if(ha > 0x58800000) {	/* a>2**50 */
	   if(ha >= 0x7f800000) {	/* Inf or NaN */
	       /* Use original arg order iff result is NaN; quieten sNaNs. */
	       w = fabsl(x+0.0L)-fabsf(y+0);
	       if(ha == 0x7f800000) w = a;
	       if(hb == 0x7f800000) w = b;
	       return w;
	   }
	   /* scale a and b by 2**-68 */
	   ha -= 0x22000000; hb -= 0x22000000;	k += 68;
	   SET_FLOAT_WORD(a,ha);
	   SET_FLOAT_WORD(b,hb);
	}
	if(hb < 0x26800000) {	/* b < 2**-50 */
	    if(hb <= 0x007fffff) {	/* subnormal b or 0 */
	        if(hb==0) return a;
		SET_FLOAT_WORD(t1,0x7e800000);	/* t1=2^126 */
		b *= t1;
		a *= t1;
		k -= 126;
	    } else {		/* scale a and b by 2^68 */
	        ha += 0x22000000; 	/* a *= 2^68 */
		hb += 0x22000000;	/* b *= 2^68 */
		k -= 68;
		SET_FLOAT_WORD(a,ha);
		SET_FLOAT_WORD(b,hb);
	    }
	}
    /* medium size a and b */
	w = a-b;
	if (w>b) {
	    SET_FLOAT_WORD(t1,ha&0xfffff000);
	    t2 = a-t1;
	    w  = sqrtf(t1*t1-(b*(-b)-t2*(a+t1)));
	} else {
	    a  = a+a;
	    SET_FLOAT_WORD(y1,hb&0xfffff000);
	    y2 = b - y1;
	    SET_FLOAT_WORD(t1,(ha+0x00800000)&0xfffff000);
	    t2 = a - t1;
	    w  = sqrtf(t1*y1-(w*(-w)-(t1*y2+t2*b)));
	}
	if(k!=0) {
	    SET_FLOAT_WORD(t1,(127+k)<<23);
	    return t1*w;
	} else return w;
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

export float
fminimum_magf(float x, float y)
{
	detail::IEEEf2bits u[2];

	u[0].f = x;
	u[1].f = y;

	/* Handle NaN according to ISO/IEC 60559. NaN argument -> NaN return */
	if (u[0].bits.exp == 255 && u[0].bits.man != 0 ||
	    u[1].bits.exp == 255 && u[1].bits.man != 0)
		return (NAN);

	float ax = fabsf(x);
	float ay = fabsf(y);

	if (ay < ax)
		return (y);
	if (ax < ay)
		return (x);

	/* If magnitudes are equal, we break the tie with the sign */
	if (u[0].bits.sign != u[1].bits.sign)
		return (u[u[1].bits.sign].f);

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

export int
isnan(double d)
{
	detail::IEEEd2bits u;

	u.d = d;
	return (u.bits.exp == 2047 && (u.bits.manl != 0 || u.bits.manh != 0));
}

export int
__isnanf(float f)
{
	detail::IEEEf2bits u;

	u.f = f;
	return (u.bits.exp == 255 && u.bits.man != 0);
}

export int
__isnanl(long double e)
{
	detail::IEEEl2bits u;

	u.e = e;
	mask_nbit_l(u);
	return (u.bits.exp == 32767 && (u.bits.manl != 0 || u.bits.manh != 0));
}

} // namespace pbsd::lib_msun_src::b0309
