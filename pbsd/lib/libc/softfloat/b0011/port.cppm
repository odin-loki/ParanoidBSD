/*
 * PBSD batch b0011 -- C++23 port of the HardenedBSD libc softfloat
 * libgcc entry points:
 *
 *	lib/libc/softfloat/eqdf2.c
 *	lib/libc/softfloat/negdf2.c
 *	lib/libc/softfloat/gesf2.c
 *	lib/libc/softfloat/lesf2.c
 *
 * Original copyright headers are retained above each ported function.
 * The double-underscore symbol names of the originals (__eqdf2 &c.) are
 * reserved to the implementation in C++, so the exported names drop the
 * leading underscores; behaviour, signedness and evaluation order are
 * otherwise unchanged.
 *
 * The comparison primitives float32_le and float64_eq come from
 * softfloat.h/softfloat.c, which are not part of this batch; they
 * are provided here as scaffolding so the ported entry points have the
 * same observable behaviour, including the raised exception flags.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0011;

export namespace pbsd::lib_libc_softfloat::b0011 {

/* ------------------------------------------------------------------ */
/* milieu.h / softfloat.h scaffolding                                 */
/* ------------------------------------------------------------------ */

using flag = char;

using bits32 = std::uint32_t;
using bits64 = std::uint64_t;

using float32 = std::uint32_t;
using float64 = std::uint64_t;

#ifndef FLOAT64_MANGLE
#define FLOAT64_MANGLE(a) (a)
#endif

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
	return static_cast<flag>(
	    (((a >> 22) & 0x1FF) == 0x1FE) && (a & 0x003FFFFF));
}

constexpr flag float64_is_signaling_nan(float64 a) noexcept
{
	return static_cast<flag>(
	    (((a >> 51) & 0xFFF) == 0xFFE) && (a & 0x0007FFFFFFFFFFFFULL));
}

flag float32_le(float32 a, float32 b) noexcept
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
		return static_cast<flag>(
		    aSign || (static_cast<bits32>((a | b) << 1) == 0));
	return static_cast<flag>((a == b) || (aSign ^ static_cast<flag>(a < b)));
}

flag float64_eq(float64 a, float64 b) noexcept
{

	if (((extractFloat64Exp(a) == 0x7FF) && extractFloat64Frac(a))
	    || ((extractFloat64Exp(b) == 0x7FF) && extractFloat64Frac(b))) {
		if (float64_is_signaling_nan(a) || float64_is_signaling_nan(b))
			float_raise(float_flag_invalid);
		return 0;
	}
	return static_cast<flag>(
	    (a == b) || (static_cast<bits64>((a | b) << 1) == 0));
}

/* ------------------------------------------------------------------ */
/* eqdf2.c							      */
/* $NetBSD: eqdf2.c,v 1.1 2000/06/06 08:15:02 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain. */
/* ------------------------------------------------------------------ */

flag eqdf2(float64 a, float64 b) noexcept
{

	/* libgcc1.c says !(a == b) */
	return !float64_eq(a, b);
}

/* ------------------------------------------------------------------ */
/* negdf2.c							      */
/* $NetBSD: negdf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

float64 negdf2(float64 a) noexcept
{

	/* libgcc1.c says -a */
	return a ^ FLOAT64_MANGLE(0x8000000000000000ULL);
}

/* ------------------------------------------------------------------ */
/* gesf2.c							      */
/* $NetBSD: gesf2.c,v 1.1 2000/06/06 08:15:05 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

flag gesf2(float32 a, float32 b) noexcept
{

	/* libgcc1.c says (a >= b) - 1 */
	return float32_le(b, a) - 1;
}

/* ------------------------------------------------------------------ */
/* lesf2.c							      */
/* $NetBSD: lesf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $	      */
/*								      */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

flag lesf2(float32 a, float32 b) noexcept
{

	/* libgcc1.c says 1 - (a <= b) */
	return 1 - float32_le(a, b);
}

} /* namespace pbsd::lib_libc_softfloat::b0011 */
