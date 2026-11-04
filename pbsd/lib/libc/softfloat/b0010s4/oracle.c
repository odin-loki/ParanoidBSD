/*
 * oracle.c -- reference implementation for PBSD batch b0010s4.
 *
 * The original HardenedBSD/NetBSD C source for
 *
 *	lib/libc/softfloat/eqsf2.c
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

int float_exception_flags = 0;

static void
float_raise(int flags)
{

	float_exception_flags |= flags;
}

#define extractFloat32Frac( a )	( ( a ) & 0x007FFFFF )
#define extractFloat32Exp( a )	( ( ( a )>>23 ) & 0xFF )
#define extractFloat32Sign( a )	( ( a )>>31 )

#define float32_is_signaling_nan( a )					\
	( ( ( ( ( a )>>22 ) & 0x1FF ) == 0x1FE ) && ( ( a ) & 0x003FFFFF ) )

flag
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

/* Exported under a ref_ name so the harness can drive it directly; this is a
 * pure forwarder, added so that the batch function's body below stays byte for
 * byte identical to the original. */
flag
ref_float32_eq(float32 a, float32 b)
{

	return float32_eq(a, b);
}

/* ------------------------------------------------------------------ */
/* eqsf2.c                                                            */
/* ------------------------------------------------------------------ */

/* $NetBSD: eqsf2.c,v 1.1 2000/06/06 08:15:03 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref_eqsf2(float32, float32);

flag
ref_eqsf2(float32 a, float32 b)
{

	/* libgcc1.c says !(a == b) */
	return !float32_eq(a, b);
}
