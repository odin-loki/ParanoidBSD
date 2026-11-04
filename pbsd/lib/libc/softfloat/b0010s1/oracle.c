/*
 * oracle.c -- reference implementation for PBSD batch b0010s1.
 *
 * Original C sources, functions renamed with a ref_ prefix, bodies
 * UNMODIFIED.  This file is the specification.
 *
 * Sources:
 *   hbsd/src/lib/libc/softfloat/nedf2.c            -> ref___nedf2
 *
 * The headers nedf2.c includes (softfloat-for-gcc.h, milieu.h, softfloat.h)
 * are not self-contained in the tree -- milieu.h pulls in a generated
 * processors/<host>.h and softfloat.h is a template -- so the typedefs,
 * LIT64 and the float64_eq() primitive nedf2.c calls are supplied below,
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
Returns 1 if the double-precision floating-point value `a' is a signaling
NaN; 0 otherwise.
-------------------------------------------------------------------------------
*/
flag float64_is_signaling_nan( float64 a )
{

    return
           ( ( ( a>>51 ) & 0xFFF ) == 0xFFE )
        && ( a & LIT64( 0x0007FFFFFFFFFFFF ) );

}

/*
-------------------------------------------------------------------------------
Returns 1 if the double-precision floating-point values `a' and `b' are equal,
0 otherwise.  Equality is according to the IEC/IEEE Standard for Binary
Floating-Point Arithmetic.
-------------------------------------------------------------------------------
*/
flag float64_eq( float64 a, float64 b )
{

    if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
         || ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
       ) {
        if ( float64_is_signaling_nan( a )
          || float64_is_signaling_nan( b ) ) {
            float_raise( float_flag_invalid );
        }
        return 0;
    }
    return ( a == b ) || ( (bits64) ( ( a | b )<<1 ) == 0 );

}

/* ------------------------------------------------------------------------- */
/* $NetBSD: nedf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref___nedf2(float64, float64);

flag
ref___nedf2(float64 a, float64 b)
{

	/* libgcc1.c says a != b */
	return !float64_eq(a, b);
}
