/* s_tanhf.c -- float version of s_tanh.c.
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

module;

#include <math.h>
#include <stdint.h>

export module pbsd.lib.msun.src.b0152;

namespace pbsd::lib_msun_src::b0152 {

/*
 * GET_FLOAT_WORD(i,d): the union member that is read is uint32_t while the
 * destination is int32_t, so the top bit of the encoding lands in the sign
 * of jx.  That conversion is load-bearing here.
 */
static inline void get_float_word(int32_t &i, float d)
{
	union {
		float f;
		uint32_t i;
	} gf_u;
	gf_u.f = d;
	i = static_cast<int32_t>(gf_u.i);
}

static const volatile float tiny = 1.0e-30;
static const float one=1.0, two=2.0, huge = 1.0e30;

export float
tanhf(float x)
{
	float t,z;
	int32_t jx,ix;

	get_float_word(jx,x);
	ix = jx&0x7fffffff;

    /* x is INF or NaN */
	if(ix>=0x7f800000) {
	    if (jx>=0) return one/x+one;    /* tanh(+-inf)=+-1 */
	    else       return one/x-one;    /* tanh(NaN) = NaN */
	}

    /* |x| < 9 */
	if (ix < 0x41100000) {		/* |x|<9 */
	    if (ix<0x39800000) {	/* |x|<2**-12 */
		if(huge+x>one) return x; /* tanh(tiny) = tiny with inexact */
	    }
	    if (ix>=0x3f800000) {	/* |x|>=1  */
		t = ::expm1f(two*::fabsf(x));
		z = one - two/(t+two);
	    } else {
	        t = ::expm1f(-two*::fabsf(x));
	        z= -t/(t+two);
	    }
    /* |x| >= 9, return +-1 */
	} else {
	    z = one - tiny;		/* raise inexact flag */
	}
	return (jx>=0)? z: -z;
}

}
