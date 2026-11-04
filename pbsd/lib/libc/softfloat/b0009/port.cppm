/*
 * PBSD batch b0009 -- C++23 port of the HardenedBSD libc softfloat
 * libgcc entry points:
 *
 *	lib/libc/softfloat/negsf2.c
 *	lib/libc/softfloat/gtsf2.c
 *	lib/libc/softfloat/gtdf2.c
 *	lib/libc/softfloat/nesf2.c
 *
 * Original copyright headers are retained above each ported function.
 * The double-underscore symbol names of the originals (__negsf2 &c.) are
 * reserved to the implementation in C++, so the exported names drop the
 * leading underscores; behaviour, signedness and evaluation order are
 * otherwise unchanged.
 *
 * The comparison primitives float32_lt, float64_lt and float32_eq come
 * from softfloat.h/softfloat.c, which are not part of this batch; they
 * are provided here as scaffolding so the ported entry points have the
 * same observable behaviour, including the raised exception flags.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0009;

export namespace pbsd::lib_libc_softfloat::b0009 {

/* ------------------------------------------------------------------ */
/* milieu.h / softfloat.h scaffolding                                 */
/* ------------------------------------------------------------------ */

using flag = int;

using bits32 = std::uint32_t;
using bits64 = std::uint64_t;

using float32 = std::uint32_t;
using float64 = std::uint64_t;

inline constexpr int float_flag_inexact = 1;
inline constexpr int float_flag_underflow = 2;
inline constexpr int float_flag_overflow = 4;
inline constexpr int float_flag_divbyzero = 8;
inline constexpr int float_flag_invalid = 16;

int float_exception_flags = 0;

void float_raise(int flags) noexcept
{

	float_exception_flags |= flags;
}

constexpr bits32 extractFloat32Frac(float32 a) noexcept { return a & 0x007FFFFF; }
constexpr std::int16_t extractFloat32Exp(float32 a) noexcept
{
	return static_cast<std::int16_t>((a >> 23) & 0xFF);
}
constexpr flag extractFloat32Sign(float32 a) noexcept
{
	return static_cast<flag>(a >> 31);
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

constexpr flag float32_is_signaling_nan(float32 a) noexcept
{
	return (((a >> 22) & 0x1FF) == 0x1FE) && (a & 0x003FFFFF);
}

constexpr flag float64_is_signaling_nan(float64 a) noexcept
{
	return (((a >> 51) & 0xFFF) == 0xFFE) && (a & 0x0007FFFFFFFFFFFFULL);
}

flag float32_eq(float32 a, float32 b) noexcept
{

	if (((extractFloat32Exp(a) == 0xFF) && extractFloat32Frac(a))
	    || ((extractFloat32Exp(b) == 0xFF) && extractFloat32Frac(b))) {
		if (float32_is_signaling_nan(a) || float32_is_signaling_nan(b))
			float_raise(float_flag_invalid);
		return 0;
	}
	return (a == b) || (static_cast<bits32>((a | b) << 1) == 0);
}

flag float32_lt(float32 a, float32 b) noexcept
{
	flag aSign, bSign;

	if (((extractFloat32Exp(a) == 0xFF) && extractFloat32Frac(a))
	    || ((extractFloat32Exp(b) == 0xFF) && extractFloat32Frac(b))) {
		float_raise(float_flag_invalid);
		return 0;
	}
	aSign = extractFloat32Sign(a);
	bSign = extractFloat32Sign(b);
	if (aSign != bSign)
		return aSign && (static_cast<bits32>((a | b) << 1) != 0);
	return (a != b) && (aSign ^ (a < b));
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
/* negsf2.c							      */
/* $NetBSD: negsf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

float32 negsf2(float32 a) noexcept
{

	/* libgcc1.c says INTIFY(-a) */
	return a ^ 0x80000000;
}

/* ------------------------------------------------------------------ */
/* gtsf2.c							      */
/* $NetBSD: gtsf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

flag gtsf2(float32 a, float32 b) noexcept
{

	/* libgcc1.c says a > b */
	return float32_lt(b, a);
}

/* ------------------------------------------------------------------ */
/* gtdf2.c							      */
/* $NetBSD: gtdf2.c,v 1.1 2000/06/06 08:15:05 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

flag gtdf2(float64 a, float64 b) noexcept
{

	/* libgcc1.c says a > b */
	return float64_lt(b, a);
}

/* ------------------------------------------------------------------ */
/* nesf2.c							      */
/* $NetBSD: nesf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

flag nesf2(float32 a, float32 b) noexcept
{

	/* libgcc1.c says a != b */
	return !float32_eq(a, b);
}

} /* namespace pbsd::lib_libc_softfloat::b0009 */
