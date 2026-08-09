/*
 * PBSD batch b0012s4 oracle: the original HardenedBSD C sources, concatenated,
 * with every function from the batch renamed with a `ref_' prefix and its body
 * otherwise unmodified.  This file is the specification.
 *
 * Sources:
 *   hbsd/src/lib/libc/softfloat/gtxf2.c
 *
 * gtxf2.c's body is a single call to floatx80_lt() on the softfloat
 * `floatx80' type; neither is part of this batch, so both are supplied below
 * exactly as they appear in the SoftFloat sources the batch headers describe.
 */

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * milieu.h / softfloat-for-gcc.h substitutes.
 * ------------------------------------------------------------------------- */

typedef int flag;
typedef int int8;
typedef int uint8;
typedef int32_t int32;
typedef uint16_t bits16;
typedef uint32_t bits32;
typedef uint64_t bits64;

#define FLOATX80 1

/* ---------------------------------------------------------------------------
 * softfloat.h substitutes.
 * ------------------------------------------------------------------------- */

typedef struct {
	bits64 low;
	bits16 high;
} floatx80;

enum {
	float_flag_inexact   =  1,
	float_flag_underflow =  2,
	float_flag_overflow  =  4,
	float_flag_divbyzero =  8,
	float_flag_invalid   = 16
};

int8 float_exception_flags = 0;

void
float_raise( int8 flags )
{

	float_exception_flags |= flags;
}

/* ---------------------------------------------------------------------------
 * softfloat-macros substitute.
 * ------------------------------------------------------------------------- */

static flag
lt128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 )
{

	return ( a0 < b0 ) || ( ( a0 == b0 ) && ( a1 < b1 ) );
}

/* ---------------------------------------------------------------------------
 * bits64/softfloat.c substitutes.
 * ------------------------------------------------------------------------- */

static bits64
extractFloatx80Frac( floatx80 a )
{

	return a.low;
}

static int32
extractFloatx80Exp( floatx80 a )
{

	return a.high & 0x7FFF;
}

static flag
extractFloatx80Sign( floatx80 a )
{

	return a.high >> 15;
}

flag
floatx80_lt( floatx80 a, floatx80 b )
{
	flag aSign, bSign;

	if (    (    ( extractFloatx80Exp( a ) == 0x7FFF )
	          && (bits64) ( extractFloatx80Frac( a )<<1 ) )
	     || (    ( extractFloatx80Exp( b ) == 0x7FFF )
	          && (bits64) ( extractFloatx80Frac( b )<<1 ) )
	   ) {
		float_raise( float_flag_invalid );
		return 0;
	}
	aSign = extractFloatx80Sign( a );
	bSign = extractFloatx80Sign( b );
	if ( aSign != bSign ) {
		return
		       aSign
		    && (    ( ( (bits16) ( ( a.high | b.high )<<1 ) )
		              | a.low | b.low )
		         != 0 );
	}
	return
	      aSign ? lt128( b.high, b.low, a.high, a.low )
	    : lt128( a.high, a.low, b.high, b.low );
}

/* Exported under a ref_ name so the harness can drive it directly; this is a
 * pure forwarder, added so that the batch function's body below stays byte for
 * byte identical to the original. */
flag
ref_floatx80_lt(floatx80 a, floatx80 b)
{

	return floatx80_lt(a, b);
}

/* ===========================================================================
 * hbsd/src/lib/libc/softfloat/gtxf2.c
 * =========================================================================== */

/* $NetBSD: gtxf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#ifdef FLOATX80

flag ref___gtxf2(floatx80, floatx80);

flag
ref___gtxf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says a > b */
	return floatx80_lt(b, a);
}
#endif /* FLOATX80 */
