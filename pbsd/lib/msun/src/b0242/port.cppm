// PBSD port of HardenedBSD lib/msun/src batch b0242.
//
// Sources ported in this module:
//   s_ilogbf.c -> ilogbf
//   s_ilogb.c  -> ilogb
//
// Original copyright headers are reproduced verbatim above each ported
// function.  The ports are literal transcriptions.

module;

#include <climits>
#include <cstdint>
#include <endian.h>

export module pbsd.lib.msun.src.b0242;

namespace pbsd::lib_msun_src::b0242 {

#ifndef __INT_MAX
#define __INT_MAX INT_MAX
#endif

#ifndef FP_ILOGB0
#define FP_ILOGB0 (-__INT_MAX)
#endif
#ifndef FP_ILOGBNAN
#define FP_ILOGBNAN __INT_MAX
#endif

static inline void get_float_word(int32_t &i, float d)
{
	union {
		float f;
		unsigned int word;
	} gf_u;
	gf_u.f = d;
	i = gf_u.word;
}

static inline void extract_words(int32_t &ix0, int32_t &ix1, double d)
{
	union {
		double value;
		struct {
#if __BYTE_ORDER == __BIG_ENDIAN
			uint32_t msw;
			uint32_t lsw;
#else
			uint32_t lsw;
			uint32_t msw;
#endif
		} parts;
	} ew_u;
	ew_u.value = d;
	ix0 = static_cast<int32_t>(ew_u.parts.msw);
	ix1 = static_cast<int32_t>(ew_u.parts.lsw);
}

static inline int32_t subnormal_ilogb(int32_t hi, int32_t lo)
{
	int32_t j;
	uint32_t i;

	j = -1022;
	if (hi == 0) {
	    j -= 21;
	    i = static_cast<uint32_t>(lo);
	} else
	    i = static_cast<uint32_t>(hi) << 11;

	for (; i < 0x7fffffffU; i <<= 1) j -= 1;

	return (j);
}

static inline int32_t subnormal_ilogbf(int32_t hx)
{
	int32_t j;
	uint32_t i;
	i = static_cast<uint32_t>(hx) << 8;
	for (j = -126; i < 0x7fffffffU; i <<= 1) j -=1;

	return (j);
}

/* s_ilogbf.c -- float version of s_ilogb.c.
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

export int
ilogbf(float x)
{
	int32_t hx,ix;

	get_float_word(hx,x);
	hx &= 0x7fffffff;
	if(hx<0x00800000) {
	    if(hx==0)
		return FP_ILOGB0;
	    else			/* subnormal x */
		ix = subnormal_ilogbf(hx);
	    return ix;
	}
	else if (hx<0x7f800000) return (hx>>23)-127;
	else if (hx>0x7f800000) return FP_ILOGBNAN;
	else return INT_MAX;
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

/* ilogb(double x)
 * return the binary exponent of non-zero x
 * ilogb(0) = FP_ILOGB0
 * ilogb(NaN) = FP_ILOGBNAN (no signal is raised)
 * ilogb(inf) = INT_MAX (no signal is raised)
 */

export int
ilogb(double x)
{
	int32_t hx, ix, lx;

	extract_words(hx,lx,x);
	hx &= 0x7fffffff;
	if(hx<0x00100000) {
	    if((hx|lx)==0)
		return FP_ILOGB0;
	    else
		ix = subnormal_ilogb(hx, lx);
	    return ix;
	}
	else if (hx<0x7ff00000) return (hx>>20)-1023;
	else if (hx>0x7ff00000 || lx!=0) return FP_ILOGBNAN;
	else return INT_MAX;
}

} // namespace pbsd::lib_msun_src::b0242
