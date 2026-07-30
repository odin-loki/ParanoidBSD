/*
 * oracle.c -- reference implementation for PBSD batch b0012s1.
 *
 * Original C sources, functions renamed with a ref_ prefix, bodies
 * UNMODIFIED.  This file is the specification.
 *
 * Sources:
 *   hbsd/src/lib/libc/softfloat/gedf2.c            -> ref___gedf2
 *
 * The headers gedf2.c includes (softfloat-for-gcc.h, milieu.h, softfloat.h)
 * are not self-contained in the tree -- milieu.h pulls in a generated
 * processors/<host>.h and softfloat.h is a template -- so the typedefs,
 * LIT64 and the float64_le() primitive gedf2.c calls are supplied below,
 * transcribed from the SoftFloat 2a package as shipped in
 * lib/libc/softfloat/bits64/softfloat.c.  They are support scaffolding and
 * keep their original names so that no ported function body has to change.
 */

/*
-------------------------------------------------------------------------------
Integer types and flags (milieu.h / processors/<host>-gcc.h).
-------------------------------------------------------------------------------
*/
typedef char flag;
typedef int int16;
typedef unsigned long long int bits64;

#define LIT64(a) a##ULL
#define INLINE static inline

/*
-------------------------------------------------------------------------------
Software IEC/IEEE double-precision type (softfloat.h).
-------------------------------------------------------------------------------
*/
typedef bits64 float64;

/*
-------------------------------------------------------------------------------
Floating-point exception flags (softfloat.h / softfloat-specialize).
-------------------------------------------------------------------------------
*/
typedef unsigned int fp_except;

enum {
    float_flag_invalid = 0x01
};

fp_except float_exception_flags = 0;

void float_raise( fp_except flags )
{

    float_exception_flags |= flags;
}

/*
-------------------------------------------------------------------------------
`FLOAT64_DEMANGLE' is the identity on targets that do not mangle float64 in
registers, which is the configuration gedf2.c is built for.
-------------------------------------------------------------------------------
*/
#define FLOAT64_DEMANGLE(a) (a)

/*
-------------------------------------------------------------------------------
Returns the fraction bits of the double-precision floating-point value `a'.
-------------------------------------------------------------------------------
*/
INLINE bits64 extractFloat64Frac( float64 a )
{

    return a & LIT64( 0x000FFFFFFFFFFFFF );

}

/*
-------------------------------------------------------------------------------
Returns the exponent bits of the double-precision floating-point value `a'.
-------------------------------------------------------------------------------
*/
INLINE int16 extractFloat64Exp( float64 a )
{

    return ( a>>52 ) & 0x7FF;

}

/*
-------------------------------------------------------------------------------
Returns the sign bit of the double-precision floating-point value `a'.
-------------------------------------------------------------------------------
*/
INLINE flag extractFloat64Sign( float64 a )
{

    return a>>63;

}

/*
-------------------------------------------------------------------------------
Returns 1 if the double-precision floating-point value `a' is less than or
equal to the corresponding value `b', and 0 otherwise.  The comparison is
performed according to the IEC/IEEE Standard for Binary Floating-Point
Arithmetic.
-------------------------------------------------------------------------------
*/
flag float64_le( float64 a, float64 b )
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
	return aSign ||
	    ( (bits64) ( ( FLOAT64_DEMANGLE(a) | FLOAT64_DEMANGLE(b) )<<1 ) ==
	      0 );
    return ( a == b ) ||
	( aSign ^ ( FLOAT64_DEMANGLE(a) < FLOAT64_DEMANGLE(b) ) );

}

/* ------------------------------------------------------------------------- */
/* $NetBSD: gedf2.c,v 1.1 2000/06/06 08:15:05 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref___gedf2(float64, float64);

flag
ref___gedf2(float64 a, float64 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return float64_le(b, a) - 1;
}
