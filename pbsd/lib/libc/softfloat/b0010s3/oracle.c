/*
 * oracle.c -- reference implementation for PBSD batch b0010s3.
 *
 * The original HardenedBSD/NetBSD C source for
 *
 *	lib/libc/softfloat/ltdf2.c
 *
 * concatenated, with the ported function renamed with a ref_ prefix.
 * The function body is UNMODIFIED.  This file is the specification.
 *
 * The #include lines of the original name private softfloat headers
 * (softfloat-for-gcc.h, milieu.h, softfloat.h).  The declarations those
 * headers supply -- the softfloat integer typedefs and the comparison
 * primitive the batch calls -- are reproduced below so that the body
 * compiles verbatim, keeping the primitive under its original name.
 */

#include <stdint.h>

/* ------------------------------------------------------------------ */
/* milieu.h / softfloat.h scaffolding                                 */
/* ------------------------------------------------------------------ */

typedef int flag;

typedef uint64_t bits64;

typedef uint64_t float64;

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif

#define LIT64( a )	a##ULL

enum {
	float_flag_invalid   = 16
};

int ref_float_exception_flags = 0;

static void
float_raise(int flags)
{

	ref_float_exception_flags |= flags;
}

#define extractFloat64Frac( a )	( ( a ) & LIT64( 0x000FFFFFFFFFFFFF ) )
#define extractFloat64Exp( a )	( ( ( a )>>52 ) & 0x7FF )
#define extractFloat64Sign( a )	( ( a )>>63 )

#define float64_is_signaling_nan( a )					\
	( ( ( ( ( a )>>51 ) & 0xFFF ) == 0xFFE )			\
	  && ( ( a ) & LIT64( 0x0007FFFFFFFFFFFF ) ) )

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
/* ltdf2.c                                                            */
/* ------------------------------------------------------------------ */

/* $NetBSD: ltdf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref_ltdf2(float64, float64);

flag
ref_ltdf2(float64 a, float64 b)
{

	/* libgcc1.c says -(a < b) */
	return -float64_lt(a, b);
}
