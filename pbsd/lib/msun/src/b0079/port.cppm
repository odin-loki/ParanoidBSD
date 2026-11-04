// PBSD port of HardenedBSD lib/msun/src batch b0079.
//
// Sources ported in this module:
//   e_lgammaf.c    -> lgammaf
//   s_finitef.c    -> finitef
//   e_gammaf_r.c   -> gammaf_r
//   s_copysignf.c  -> copysignf
//
// Original copyright headers are reproduced verbatim above each ported
// function.  The ports are literal transcriptions: masks, signedness, shift
// widths and evaluation order are preserved exactly.

module;

#include <bit>
#include <cmath>
#include <cstdint>

export module pbsd.lib.msun.src.b0079;

namespace pbsd::lib_msun_src::b0079::detail {

// Equivalents of the GET_FLOAT_WORD / SET_FLOAT_WORD macros from
// lib/msun/src/math_private.h.  The upstream macros type-pun through
// union ieee_float_shape_type; std::bit_cast reproduces that bit-for-bit.
inline std::uint32_t get_float_word(float d) noexcept
{
	return std::bit_cast<std::uint32_t>(d);
}

inline void set_float_word(float &d, std::uint32_t i) noexcept
{
	d = std::bit_cast<float>(i);
}

} // namespace pbsd::lib_msun_src::b0079::detail

export namespace pbsd::lib_msun_src::b0079 {

using int32_t = std::int32_t;
using u_int32_t = std::uint32_t;

/* e_lgammaf.c -- float version of e_lgamma.c.
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

/* lgammaf(x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call lgammaf_r
 */

float
lgammaf(float x)
{
	return ::lgammaf_r(x, &::signgam);
}

/* s_finitef.c -- float version of s_finite.c.
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
 * finitef(x) returns 1 is x is finite, else 0;
 * no branching!
 */

int finitef(float x)
{
	int32_t ix;
	ix = detail::get_float_word(x);
	return (int)((u_int32_t)((ix & 0x7fffffff) - 0x7f800000) >> 31);
}

/* e_gammaf_r.c -- float version of e_gamma_r.c.
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

/* gammaf_r(x, signgamp)
 * Reentrant version of the logarithm of the Gamma function
 * with user provide pointer for the sign of Gamma(x).
 *
 * Method: See lgammaf_r
 */

float
gammaf_r(float x, int *signgamp)
{
	return ::lgammaf_r(x, signgamp);
}

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
	u_int32_t ix, iy;
	ix = detail::get_float_word(x);
	iy = detail::get_float_word(y);
	detail::set_float_word(x, (ix & 0x7fffffff) | (iy & 0x80000000));
	return x;
}

} // namespace pbsd::lib_msun_src::b0079
