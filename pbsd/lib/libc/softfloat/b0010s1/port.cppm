// PBSD -- C++23 port of HardenedBSD lib/libc/softfloat, batch b0010s1.
//
// Ported source:
//   hbsd/src/lib/libc/softfloat/nedf2.c
//
// Original copyright headers are reproduced verbatim below.  The port is
// deliberately literal: signedness, promotion, evaluation order and the
// `flag' (== char) return type are all load bearing and are preserved.

/* $NetBSD: nedf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

/*
 * The SoftFloat integer milieu (milieu.h, processors/<host>-gcc.h) and the
 * float64_eq() primitive that nedf2.c calls are reproduced here from the
 * same package, since nedf2.c cannot be exercised without them:
 *
 *   milieu.h, softfloat.h, bits64/softfloat.c
 *
===============================================================================

This C source file is part of the SoftFloat IEC/IEEE Floating-point Arithmetic
Package, Release 2a.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://HTTP.CS.Berkeley.EDU/~jhauser/
arithmetic/SoftFloat.html'.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort
has been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT
TIMES RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO
PERSONS AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ANY
AND ALL LOSSES, COSTS, OR OTHER PROBLEMS ARISING FROM ITS USE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) they include prominent notice that the work is derivative, and (2) they
include prominent notice akin to these four paragraphs for those parts of
this code that are retained.

===============================================================================
*/

export module pbsd.lib.libc.softfloat.b0010s1;

#define LIT64(a) a##ULL

export namespace pbsd::lib_libc_softfloat::b0010s1 {

/*
-------------------------------------------------------------------------------
Integer types, from the SoftFloat processor milieu.  `flag' is char, so a
non-zero return from nedf2() is a char with value 1, not an int.
-------------------------------------------------------------------------------
*/
typedef char flag;
typedef int int16;
typedef unsigned long long int bits64;

/*
-------------------------------------------------------------------------------
Software IEC/IEEE double-precision type.
-------------------------------------------------------------------------------
*/
typedef bits64 float64;

/*
-------------------------------------------------------------------------------
Floating-point exception flags and their accumulator.
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
bits64 extractFloat64Frac( float64 a )
{

    return a & LIT64( 0x000FFFFFFFFFFFFF );

}

/*
-------------------------------------------------------------------------------
Returns the exponent bits of the double-precision floating-point value `a'.
-------------------------------------------------------------------------------
*/
int16 extractFloat64Exp( float64 a )
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

/*
-------------------------------------------------------------------------------
nedf2() is __nedf2() of nedf2.c; the leading underscores are dropped because
they are reserved in C++.  Behaviour is unchanged: the logical negation of the
equality test, so 1 when a != b and 0 when a == b (including +0 == -0).
-------------------------------------------------------------------------------
*/
flag nedf2( float64 a, float64 b )
{

	/* libgcc1.c says a != b */
	return !float64_eq(a, b);
}

} // namespace pbsd::lib_libc_softfloat::b0010s1
