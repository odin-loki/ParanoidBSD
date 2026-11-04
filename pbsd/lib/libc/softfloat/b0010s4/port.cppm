/*
 * PBSD batch b0010s4 -- C++23 port of the HardenedBSD libc softfloat
 * libgcc entry point:
 *
 *	lib/libc/softfloat/eqsf2.c
 *
 * Original copyright headers are retained above the ported function.
 * The double-underscore symbol name of the original (__eqsf2) is reserved
 * to the implementation in C++, so the exported name drops the leading
 * underscores; behaviour, signedness and evaluation order are otherwise
 * unchanged.
 *
 * The comparison primitive float32_eq comes from softfloat.h/softfloat.c,
 * which is not part of this batch; it is provided here as scaffolding so
 * the ported entry point has the same observable behaviour, including the
 * raised exception flags.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0010s4;

export namespace pbsd::lib_libc_softfloat::b0010s4 {

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

void float_raise(int flags)
{

	float_exception_flags |= flags;
}

#define extractFloat32Frac( a )	( ( a ) & 0x007FFFFF )
#define extractFloat32Exp( a )	( ( ( a )>>23 ) & 0xFF )
#define extractFloat32Sign( a )	( ( a )>>31 )

#define float32_is_signaling_nan( a )					\
	( ( ( ( ( a )>>22 ) & 0x1FF ) == 0x1FE ) && ( ( a ) & 0x003FFFFF ) )

flag float32_eq(float32 a, float32 b)
{

	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
	     || ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
	   ) {
		if ( float32_is_signaling_nan( a )
		     || float32_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid );
		}
		return 0;
	}
	return ( a == b ) || ( (bits32) ( ( a | b )<<1 ) == 0 );
}

/* ------------------------------------------------------------------ */
/* eqsf2.c                                                            */
/* $NetBSD: eqsf2.c,v 1.1 2000/06/06 08:15:03 bjh21 Exp $             */
/*                                                                    */
/* Written by Ben Harris, 2000.  This file is in the Public Domain.   */
/* ------------------------------------------------------------------ */

flag eqsf2(float32, float32);

flag
eqsf2(float32 a, float32 b)
{

	/* libgcc1.c says !(a == b) */
	return !float32_eq(a, b);
}

} /* namespace pbsd::lib_libc_softfloat::b0010s4 */
