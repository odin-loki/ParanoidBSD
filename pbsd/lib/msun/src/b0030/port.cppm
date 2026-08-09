// SPDX-License-Identifier: BSD-2-Clause
//
// PBSD port of HardenedBSD lib/msun/src/s_logbl.c and lib/msun/src/s_rintf.c
//
// s_logbl.c:
// /*
//  * ====================================================
//  * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
//  *
//  * Developed at SunPro, a Sun Microsystems, Inc. business.
//  * Permission to use, copy, modify, and distribute this
//  * software is freely granted, provided that this notice
//  * is preserved.
//  * ====================================================
//  */
//
// s_rintf.c:
// /* s_rintf.c -- float version of s_rint.c.
//  * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
//  */
// /*
//  * ====================================================
//  * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
//  *
//  * Developed at SunPro, a Sun Microsystems, Inc. business.
//  * Permission to use, copy, modify, and distribute this
//  * software is freely granted, provided that this notice
//  * is preserved.
//  * ====================================================
//  */

module;

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

/*
 * From lib/msun/amd64/fpmath.h -- the x86 80-bit extended layout has an
 * explicit integer bit, so LDBL_IMPLICIT_NBIT is deliberately NOT defined.
 */
#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32
#define	LDBL_NBIT	0x80000000

/*
 * From lib/msun/src/math_private.h.
 */
#ifdef FLT_EVAL_METHOD
#if FLT_EVAL_METHOD == 0 || __GNUC__ == 0
#define	STRICT_ASSIGN(type, lval, rval)	((lval) = (rval))
#else
#define	STRICT_ASSIGN(type, lval, rval) do {	\
	volatile type __lval;			\
						\
	if (sizeof(type) >= sizeof(long double))	\
		(lval) = (rval);		\
	else {					\
		__lval = (rval);		\
		(lval) = __lval;		\
	}					\
} while (0)
#endif
#endif

#define	GET_FLOAT_WORD(i,d)					\
do {								\
  pbsd_ieee_float_shape_type gf_u;				\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

#define	SET_FLOAT_WORD(d,i)					\
do {								\
  pbsd_ieee_float_shape_type sf_u;				\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)

export module pbsd.lib.msun.src.b0030;

namespace pbsd::lib_msun_src::b0030 {

/* lib/msun/amd64/fpmath.h */
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

/* lib/msun/src/math_private.h */
union pbsd_ieee_float_shape_type {
	float		value;
	uint32_t	word;
};

/* s_rintf.c */
static const float
TWO23[2]={
  8.3886080000e+06, /* 0x4b000000 */
 -8.3886080000e+06, /* 0xcb000000 */
};

/* s_logbl.c */
export long double
logbl(long double x)
{
	union IEEEl2bits u;
	unsigned long m;
	int b;

	u.e = x;
	if (u.bits.exp == 0) {
		if ((u.bits.manl | u.bits.manh) == 0) {	/* x == 0 */
			u.bits.sign = 1;
			return (1.0L / u.e);
		}
		/* denormalized */
		if (u.bits.manh == 0) {
			m = 1lu << (LDBL_MANL_SIZE - 1);
			for (b = LDBL_MANH_SIZE; !(u.bits.manl & m); m >>= 1)
				b++;
		} else {
			m = 1lu << (LDBL_MANH_SIZE - 1);
			for (b = 0; !(u.bits.manh & m); m >>= 1)
				b++;
		}
#ifdef LDBL_IMPLICIT_NBIT
		b++;
#endif
		return ((long double)(LDBL_MIN_EXP - b - 1));
	}
	if (u.bits.exp < (LDBL_MAX_EXP << 1) - 1)	/* normal */
		return ((long double)(u.bits.exp - LDBL_MAX_EXP + 1));
	else						/* +/- inf or nan */
		return (x * x);
}

/* s_rintf.c */
export float
rintf(float x)
{
	int32_t i0,j0,sx;
	float w,t;
	GET_FLOAT_WORD(i0,x);
	sx = (i0>>31)&1;
	j0 = ((i0>>23)&0xff)-0x7f;
	if(j0<23) {
	    if(j0<0) {
		if((i0&0x7fffffff)==0) return x;
		STRICT_ASSIGN(float,w,TWO23[sx]+x);
	        t =  w-TWO23[sx];
		GET_FLOAT_WORD(i0,t);
		SET_FLOAT_WORD(t,(i0&0x7fffffff)|(sx<<31));
	        return t;
	    }
	    STRICT_ASSIGN(float,w,TWO23[sx]+x);
	    return w-TWO23[sx];
	}
	if(j0==0x80) return x+x;	/* inf or NaN */
	else return x;			/* x is integral */
}

} // namespace pbsd::lib_msun_src::b0030
