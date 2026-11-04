/*
 * PBSD batch b0010s2 -- C++23 port of lib/libc/softfloat/ltsf2.c.
 *
 * The comparison primitive float32_lt comes from softfloat.h/softfloat.c,
 * which is not part of this batch; it is provided here as scaffolding so
 * the ported entry point has the same observable behaviour, including the
 * raised exception flags.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0010s2;

export namespace pbsd::lib_libc_softfloat::b0010s2 {

/* ------------------------------------------------------------------ */
/* milieu.h / softfloat.h scaffolding                                 */
/* ------------------------------------------------------------------ */

using flag = int;

using bits32 = std::uint32_t;

using float32 = std::uint32_t;

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

constexpr flag float32_is_signaling_nan(float32 a) noexcept
{
	return (((a >> 22) & 0x1FF) == 0x1FE) && (a & 0x003FFFFF);
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

/* ------------------------------------------------------------------ */
/* ltsf2.c                                                            */
/* $NetBSD: ltsf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $             */
/*                                                                    */
/* Written by Ben Harris, 2000.  This file is in the Public Domain. */
/* ------------------------------------------------------------------ */

flag ltsf2(float32 a, float32 b) noexcept
{

	/* libgcc1.c says -(a < b) */
	return -float32_lt(a, b);
}

} /* namespace pbsd::lib_libc_softfloat::b0010s2 */
