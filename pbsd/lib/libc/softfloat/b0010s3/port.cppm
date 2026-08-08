/*
 * PBSD batch b0010s3 -- C++23 port of the HardenedBSD libc softfloat
 * libgcc entry point:
 *
 *	lib/libc/softfloat/ltdf2.c
 *
 * Original copyright headers are retained above the ported function.
 * The double-underscore symbol name of the original (__ltdf2) is
 * reserved to the implementation in C++, so the exported name drops the
 * leading underscores; behaviour, signedness and evaluation order are
 * otherwise unchanged.
 *
 * The comparison primitive float64_lt comes from softfloat.h/softfloat.c,
 * which are not part of this batch; it is provided here as scaffolding so
 * the ported entry point has the same observable behaviour, including the
 * raised exception flags.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0010s3;

export namespace pbsd::lib_libc_softfloat::b0010s3 {

/* ------------------------------------------------------------------ */
/* milieu.h / softfloat.h scaffolding                                 */
/* ------------------------------------------------------------------ */

using flag = int;

using bits64 = std::uint64_t;

using float64 = std::uint64_t;

inline constexpr int float_flag_invalid = 16;

int float_exception_flags = 0;

void float_raise(int flags) noexcept
{

	float_exception_flags |= flags;
}

constexpr bits64 extractFloat64Frac(float64 a) noexcept
{
	return a & 0x000FFFFFFFFFFFFFULL;
}
constexpr std::int16_t extractFloat64Exp(float64 a) noexcept
{
	return static_cast<std::int16_t>((a >> 52) & 0x7FF);
}
constexpr flag extractFloat64Sign(float64 a) noexcept
{
	return static_cast<flag>(a >> 63);
}

constexpr flag float64_is_signaling_nan(float64 a) noexcept
{
	return (((a >> 51) & 0xFFF) == 0xFFE) && (a & 0x0007FFFFFFFFFFFFULL);
}

flag float64_lt(float64 a, float64 b) noexcept
{
	flag aSign, bSign;

	if (((extractFloat64Exp(a) == 0x7FF) && extractFloat64Frac(a))
	    || ((extractFloat64Exp(b) == 0x7FF) && extractFloat64Frac(b))) {
		float_raise(float_flag_invalid);
		return 0;
	}
	aSign = extractFloat64Sign(a);
	bSign = extractFloat64Sign(b);
	if (aSign != bSign)
		return aSign && (static_cast<bits64>((a | b) << 1) != 0);
	return (a != b) && (aSign ^ (a < b));
}

/* ------------------------------------------------------------------ */
/* ltdf2.c							      */
/* $NetBSD: ltdf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

flag ltdf2(float64 a, float64 b) noexcept
{

	/* libgcc1.c says -(a < b) */
	return -float64_lt(a, b);
}

} /* namespace pbsd::lib_libc_softfloat::b0010s3 */
