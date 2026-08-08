/*
 * oracle.c -- reference (specification) build for PBSD batch b0010s2.
 *
 * Contents:
 *   lib/libc/softfloat/ltsf2.c            -- the batch source
 *
 * Supporting definitions required to link the batch source are taken
 * from the SoftFloat sources it is compiled against:
 *   milieu.h / softfloat.h (integer types, float32, comparison helpers)
 *
 * Every batch function is renamed with a ref_ prefix; the renaming is done
 * on the function definition so that no function body is altered in any way.
 */

#include <stdint.h>

/* ------------------------------------------------------------------ */
/* milieu.h / softfloat.h scaffolding                                 */
/* ------------------------------------------------------------------ */

typedef int flag;

typedef uint32_t bits32;

typedef uint32_t float32;

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif

enum {
	float_flag_inexact   =  1,
	float_flag_underflow =  2,
	float_flag_overflow  =  4,
	float_flag_divbyzero =  8,
	float_flag_invalid   = 16
};

int ref_float_exception_flags = 0;

static void
float_raise(int flags)
{

	ref_float_exception_flags |= flags;
}

#define extractFloat32Frac( a )	( ( a ) & 0x007FFFFF )
#define extractFloat32Exp( a )	( ( ( a )>>23 ) & 0xFF )
#define extractFloat32Sign( a )	( ( a )>>31 )

#define float32_is_signaling_nan( a )					\
	( ( ( ( ( a )>>22 ) & 0x1FF ) == 0x1FE ) && ( ( a ) & 0x003FFFFF ) )

static flag
float32_lt(float32 a, float32 b)
{
	flag aSign, bSign;

	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
	     || ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
	   ) {
		float_raise( float_flag_invalid );
		return 0;
	}
	aSign = extractFloat32Sign( a );
	bSign = extractFloat32Sign( b );
	if ( aSign != bSign )
		return aSign && ( (bits32) ( ( a | b )<<1 ) != 0 );
	return ( a != b ) && ( aSign ^ ( a < b ) );
}

flag
ref_float32_lt(float32 a, float32 b)
{

	return float32_lt(a, b);
}

/* ------------------------------------------------------------------ */
/* ltsf2.c -- the batch source                                        */
/* ------------------------------------------------------------------ */

/* $NetBSD: ltsf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref___ltsf2(float32, float32);

flag
ref___ltsf2(float32 a, float32 b)
{

	/* libgcc1.c says -(a < b) */
	return -float32_lt(a, b);
}
