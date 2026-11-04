/*
 * PBSD C++23 module port of HardenedBSD lib/msun/src for batch b0079s2.
 *
 * Sources ported here:
 *   lib/msun/src/s_finitef.c
 *
 * The port is intentionally literal: the original expression, its integer
 * signedness and its evaluation order are preserved exactly.
 */

module;

#include <cstdint>

export module pbsd.lib.msun.src.b0079s2;

/*
 * Faithful reproduction of the msun math_private.h accessor used by the
 * sources in this batch.  The original is a macro built around a union;
 * keeping the union here preserves the exact bit-level behaviour.
 */
namespace {

using pbsd_int32_t = std::int32_t;
using pbsd_u_int32_t = std::uint32_t;

inline void
pbsd_get_float_word(pbsd_int32_t &i, float d)
{
	union {
		float value;
		pbsd_u_int32_t word;
	} gf_u;

	gf_u.value = d;
	i = static_cast<pbsd_int32_t>(gf_u.word);
}

} /* namespace */

export namespace pbsd::lib_msun_src::b0079s2 {

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
	pbsd_int32_t ix;
	pbsd_get_float_word(ix, x);
	return (int)((pbsd_u_int32_t)((ix & 0x7fffffff) - 0x7f800000) >> 31);
}

} /* namespace pbsd::lib_msun_src::b0079s2 */
