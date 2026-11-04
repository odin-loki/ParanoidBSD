/*
 * oracle.c -- reference implementation for PBSD batch b0009.
 *
 * The original HardenedBSD/NetBSD C sources for
 *
 *	lib/libc/softfloat/negsf2.c
 *	lib/libc/softfloat/gtsf2.c
 *	lib/libc/softfloat/gtdf2.c
 *	lib/libc/softfloat/nesf2.c
 *
 * concatenated, with each ported function renamed with a ref_ prefix.
 * The function bodies are UNMODIFIED.  This file is the specification.
 *
 * The #include lines of the originals name private softfloat headers
 * (softfloat-for-gcc.h, milieu.h, softfloat.h).  The declarations those
 * headers supply -- the softfloat integer typedefs and the three
 * comparison primitives the batch calls -- are reproduced below so that
 * the bodies compile verbatim, keeping the primitives under their
 * original names.
 */

#include <stdint.h>

/* ------------------------------------------------------------------ */
/* milieu.h / softfloat.h scaffolding                                 */
/* ------------------------------------------------------------------ */

typedef int flag;

typedef uint32_t bits32;
typedef uint64_t bits64;

typedef uint32_t float32;
typedef uint64_t float64;

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif

#define LIT64( a )	a##ULL

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

#define extractFloat64Frac( a )	( ( a ) & LIT64( 0x000FFFFFFFFFFFFF ) )
#define extractFloat64Exp( a )	( ( ( a )>>52 ) & 0x7FF )
#define extractFloat64Sign( a )	( ( a )>>63 )

#define float32_is_signaling_nan( a )					\
	( ( ( ( ( a )>>22 ) & 0x1FF ) == 0x1FE ) && ( ( a ) & 0x003FFFFF ) )

#define float64_is_signaling_nan( a )					\
	( ( ( ( ( a )>>51 ) & 0xFFF ) == 0xFFE )			\
	  && ( ( a ) & LIT64( 0x0007FFFFFFFFFFFF ) ) )

static flag
float32_eq(float32 a, float32 b)
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

static flag
float64_lt(float64 a, float64 b)
{
	flag aSign, bSign;

	if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
	     || ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
	   ) {
		float_raise( float_flag_invalid );
		return 0;
	}
	aSign = extractFloat64Sign( a );
	bSign = extractFloat64Sign( b );
	if ( aSign != bSign )
		return aSign && ( (bits64) ( ( a | b )<<1 ) != 0 );
	return ( a != b ) && ( aSign ^ ( a < b ) );
}

/* ------------------------------------------------------------------ */
/* negsf2.c                                                           */
/* ------------------------------------------------------------------ */

/* $NetBSD: negsf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

float32 ref_negsf2(float32);

float32
ref_negsf2(float32 a)
{

	/* libgcc1.c says INTIFY(-a) */
	return a ^ 0x80000000;
}

/* ------------------------------------------------------------------ */
/* gtsf2.c                                                            */
/* ------------------------------------------------------------------ */

/* $NetBSD: gtsf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref_gtsf2(float32, float32);

flag
ref_gtsf2(float32 a, float32 b)
{

	/* libgcc1.c says a > b */
	return float32_lt(b, a);
}

/* ------------------------------------------------------------------ */
/* gtdf2.c                                                            */
/* ------------------------------------------------------------------ */

/* $NetBSD: gtdf2.c,v 1.1 2000/06/06 08:15:05 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref_gtdf2(float64, float64);

flag
ref_gtdf2(float64 a, float64 b)
{

	/* libgcc1.c says a > b */
	return float64_lt(b, a);
}

/* ------------------------------------------------------------------ */
/* nesf2.c                                                            */
/* ------------------------------------------------------------------ */

/* $NetBSD: nesf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref_nesf2(float32, float32);

flag
ref_nesf2(float32 a, float32 b)
{

	/* libgcc1.c says a != b */
	return !float32_eq(a, b);
}
