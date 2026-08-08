// PBSD port of HardenedBSD lib/msun/src batch b0135.
//
// Sources ported in this module:
//   s_scalbnf.c  -> scalbnf
//   s_fabsf.c    -> fabsf
//   s_copysign.c -> copysign
//   e_gammaf.c   -> gammaf
//
// Original copyright headers are reproduced verbatim above each ported
// function.  The ports are literal transcriptions: masks, signedness, shift
// widths and evaluation order are preserved exactly.

module;

#include <bit>
#include <cmath>
#include <cstdint>

export module pbsd.lib.msun.src.b0135;

extern "C" float gammaf_r(float x, int *signgamp);

namespace pbsd::lib_msun_src::b0135::detail {

inline std::uint32_t get_float_word(float d) noexcept
{
	return std::bit_cast<std::uint32_t>(d);
}

inline void set_float_word(float &d, std::uint32_t i) noexcept
{
	d = std::bit_cast<float>(i);
}

inline std::uint32_t get_high_word(double d) noexcept
{
	auto u = std::bit_cast<std::uint64_t>(d);
	if constexpr (std::endian::native == std::endian::little)
		return static_cast<std::uint32_t>(u >> 32);
	return static_cast<std::uint32_t>(u);
}

inline void set_high_word(double &d, std::uint32_t v) noexcept
{
	auto u = std::bit_cast<std::uint64_t>(d);
	if constexpr (std::endian::native == std::endian::little)
		u = (u & 0xffffffffull) | (static_cast<std::uint64_t>(v) << 32);
	else
		u = (u & 0xffffffff00000000ull) | v;
	d = std::bit_cast<double>(u);
}

} // namespace pbsd::lib_msun_src::b0135::detail

export namespace pbsd::lib_msun_src::b0135 {

using u_int32_t = std::uint32_t;

/*
 * Copyright (c) 2005-2020 Rich Felker, et al.
 *
 * SPDX-License-Identifier: MIT
 *
 * Please see https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT
 * for all contributors to musl.
 */

float scalbnf(float x, int n)
{
	union {
		float f;
		std::uint32_t i;
	} u;
	float_t y = x;

	if (n > 127) {
		y *= 0x1p127f;
		n -= 127;
		if (n > 127) {
			y *= 0x1p127f;
			n -= 127;
			if (n > 127)
				n = 127;
		}
	} else if (n < -126) {
		y *= 0x1p-126f * 0x1p24f;
		n += 126 - 24;
		if (n < -126) {
			y *= 0x1p-126f * 0x1p24f;
			n += 126 - 24;
			if (n < -126)
				n = -126;
		}
	}
	u.i = (std::uint32_t)(0x7f + n) << 23;
	x = y * u.f;
	return x;
}

/* s_fabsf.c -- float version of s_fabs.c.
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
 * fabsf(x) returns the absolute value of x.
 */

float
fabsf(float x)
{
	u_int32_t ix;
	ix = detail::get_float_word(x);
	detail::set_float_word(x, ix & 0x7fffffff);
	return x;
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

/*
 * copysign(double x, double y)
 * copysign(x,y) returns a value with the magnitude of x and
 * with the sign bit of y.
 */

double
copysign(double x, double y)
{
	u_int32_t hx, hy;
	hx = detail::get_high_word(x);
	hy = detail::get_high_word(y);
	detail::set_high_word(x, (hx & 0x7fffffff) | (hy & 0x80000000));
	return x;
}

/* e_gammaf.c -- float version of e_gamma.c.
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

/* gammaf(x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call gammaf_r
 */

float
gammaf(float x)
{
	return ::gammaf_r(x, &::signgam);
}

} // namespace pbsd::lib_msun_src::b0135
