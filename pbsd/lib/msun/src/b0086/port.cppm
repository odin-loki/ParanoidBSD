/*
 * PBSD port of HardenedBSD lib/msun/src batch b0086.
 *
 * Sources ported here:
 *   e_lgammal.c, s_fabs.c, s_finite.c
 *
 * Original copyright notices are reproduced verbatim above each function.
 */

module;

#include <bit>
#include <cmath>
#include <cstdint>

export module pbsd.lib.msun.src.b0086;

/*
 * The bodies below use the word-access macros from lib/msun/src/math_private.h.
 * They are reproduced here as inline functions because a macro cannot be
 * exported from a module interface unit.  ieee_double_shape_type punning is
 * expressed with std::bit_cast, which yields the identical word values.
 */
namespace pbsd::lib_msun_src::b0086::detail {

inline void
get_high_word(std::uint32_t &i, double d)
{
	i = static_cast<std::uint32_t>(std::bit_cast<std::uint64_t>(d) >> 32);
}

inline void
get_high_word(std::int32_t &i, double d)
{
	i = static_cast<std::int32_t>(
	    static_cast<std::uint32_t>(std::bit_cast<std::uint64_t>(d) >> 32));
}

inline void
set_high_word(double &d, std::uint32_t v)
{
	std::uint64_t sh_u = std::bit_cast<std::uint64_t>(d);
	sh_u = (sh_u & 0x00000000ffffffffULL) |
	    (static_cast<std::uint64_t>(v) << 32);
	d = std::bit_cast<double>(sh_u);
}

} /* namespace pbsd::lib_msun_src::b0086::detail */

extern "C" {
extern int signgam;
}

export namespace pbsd::lib_msun_src::b0086 {

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

long double
lgammal(long double x)
{
	return ::lgammal_r(x, &::signgam);
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
 * fabs(x) returns the absolute value of x.
 */

double
fabs(double x)
{
	std::uint32_t high;
	detail::get_high_word(high, x);
	detail::set_high_word(x, high & 0x7fffffff);
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
 * finite(x) returns 1 is x is finite, else 0;
 * no branching!
 */

	int finite(double x)
{
	std::int32_t hx;
	detail::get_high_word(hx, x);
	return (int)((std::uint32_t)((hx & 0x7fffffff) - 0x7ff00000) >> 31);
}

} /* namespace pbsd::lib_msun_src::b0086 */
