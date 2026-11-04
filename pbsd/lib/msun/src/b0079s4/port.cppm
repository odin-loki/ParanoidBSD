// PBSD port of HardenedBSD lib/msun/src batch b0079s4.
//
// Sources ported in this module:
//   s_copysignf.c  -> copysignf

module;

#include <cstdint>

export module pbsd.lib.msun.src.b0079s4;

typedef union
{
	float value;
	/* FIXME: Assumes 32 bit int.  */
	unsigned int word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i,d)					\
do {								\
	ieee_float_shape_type gf_u;				\
	gf_u.value = (d);					\
	(i) = gf_u.word;					\
} while (0)

#define SET_FLOAT_WORD(d,i)					\
do {								\
	ieee_float_shape_type sf_u;				\
	sf_u.word = (i);					\
	(d) = sf_u.value;					\
} while (0)

export namespace pbsd::lib_msun_src::b0079s4 {

using u_int32_t = std::uint32_t;

/* s_copysignf.c -- float version of s_copysign.c.
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

/*
 * copysignf(float x, float y)
 * copysignf(x,y) returns a value with the magnitude of x and
 * with the sign bit of y.
 */

float
copysignf(float x, float y)
{
	u_int32_t ix,iy;
	GET_FLOAT_WORD(ix,x);
	GET_FLOAT_WORD(iy,y);
	SET_FLOAT_WORD(x,(ix&0x7fffffff)|(iy&0x80000000));
        return x;
}

} // namespace pbsd::lib_msun_src::b0079s4
