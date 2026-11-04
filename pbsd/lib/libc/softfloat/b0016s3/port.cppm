/*
 * PBSD batch b0016s3 -- C++23 port of lib/libc/softfloat/getf2.c.
 *
 * The batch source is getf2.c.  The SoftFloat machinery it is compiled
 * against (the float128 format, float_raise(), le128() and float128_le())
 * is carried over with it so that the module is self-contained; those
 * pieces come from templates/milieu.h, templates/softfloat.h,
 * softfloat-specialize, bits64/softfloat-macros and bits64/softfloat.c.
 *
 * Behaviour is preserved exactly: the integer types keep the widths the
 * SoftFloat processor template gives them (`flag' and `int8' are plain
 * `int'), and the evaluation order and the mixed integer/boolean
 * arithmetic are untouched.
 */

/* $NetBSD: getf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

/*
===============================================================================

This C source file is part of the SoftFloat IEC/IEEE Floating-point
Arithmetic Package, Release 2a.

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

module;

#include <csignal>

export module pbsd.lib.libc.softfloat.b0016s3;

export namespace pbsd::lib_libc_softfloat::b0016s3 {

/*
-------------------------------------------------------------------------------
milieu.h / processors template: integer types and boolean literals.
-------------------------------------------------------------------------------
*/
using flag = int;
using uint8 = int;
using int8 = int;
using uint16 = int;
using int16 = int;
using uint32 = unsigned int;
using int32 = signed int;
using uint64 = unsigned long long int;
using int64 = signed long long int;

using bits8 = unsigned char;
using sbits8 = signed char;
using bits16 = unsigned short int;
using sbits16 = signed short int;
using bits32 = unsigned int;
using sbits32 = signed int;
using bits64 = unsigned long long int;
using sbits64 = signed long long int;

enum {
    FALSE = 0,
    TRUE  = 1
};

/*
-------------------------------------------------------------------------------
softfloat.h: the quadruple-precision format and the exception flags.
-------------------------------------------------------------------------------
*/
struct float128 {
    bits64 high, low;
};

enum {
    float_flag_inexact   =  1,
    float_flag_underflow =  2,
    float_flag_overflow  =  4,
    float_flag_divbyzero =  8,
    float_flag_invalid   = 16
};

int8 float_exception_flags = 0;

/*
-------------------------------------------------------------------------------
softfloat-specialize: raises the exceptions specified by `flags'.
-------------------------------------------------------------------------------
*/
int float_exception_mask = 0;
void float_raise( int flags )
{

    float_exception_flags |= flags;

    if ( flags & float_exception_mask ) {
	::raise( SIGFPE );
    }
}

/*
-------------------------------------------------------------------------------
Returns 1 if the 128-bit value formed by concatenating `a0' and `a1' is less
than or equal to the 128-bit value formed by concatenating `b0' and `b1'.
Otherwise, returns 0.
-------------------------------------------------------------------------------
*/
flag le128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 )
{

    return ( a0 < b0 ) || ( ( a0 == b0 ) && ( a1 <= b1 ) );

}

/*
-------------------------------------------------------------------------------
Returns the least-significant 64 fraction bits of the quadruple-precision
floating-point value `a'.
-------------------------------------------------------------------------------
*/
bits64 extractFloat128Frac1( float128 a )
{

    return a.low;

}

/*
-------------------------------------------------------------------------------
Returns the most-significant 48 fraction bits of the quadruple-precision
floating-point value `a'.
-------------------------------------------------------------------------------
*/
bits64 extractFloat128Frac0( float128 a )
{

    return a.high & 0x0000FFFFFFFFFFFFULL;

}

/*
-------------------------------------------------------------------------------
Returns the exponent bits of the quadruple-precision floating-point value
`a'.
-------------------------------------------------------------------------------
*/
int32 extractFloat128Exp( float128 a )
{

    return ( a.high>>48 ) & 0x7FFF;

}

/*
-------------------------------------------------------------------------------
Returns the sign bit of the quadruple-precision floating-point value `a'.
-------------------------------------------------------------------------------
*/
flag extractFloat128Sign( float128 a )
{

    return a.high>>63;

}

/*
-------------------------------------------------------------------------------
Returns 1 if the quadruple-precision floating-point value `a' is less than
or equal to the corresponding value `b', and 0 otherwise.  The comparison
is performed according to the IEC/IEEE Standard for Binary Floating-Point
Arithmetic.
-------------------------------------------------------------------------------
*/
flag float128_le( float128 a, float128 b )
{
    flag aSign, bSign;

    if (    (    ( extractFloat128Exp( a ) == 0x7FFF )
              && ( extractFloat128Frac0( a ) | extractFloat128Frac1( a ) ) )
         || (    ( extractFloat128Exp( b ) == 0x7FFF )
              && ( extractFloat128Frac0( b ) | extractFloat128Frac1( b ) ) )
       ) {
        float_raise( float_flag_invalid );
        return 0;
    }
    aSign = extractFloat128Sign( a );
    bSign = extractFloat128Sign( b );
    if ( aSign != bSign ) {
        return
               aSign
            || (    ( ( (bits64) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
                 == 0 );
    }
    return
          aSign ? le128( b.high, b.low, a.high, a.low )
        : le128( a.high, a.low, b.high, b.low );

}

/*
-------------------------------------------------------------------------------
getf2.c: __getf2(), the GCC-facing float128 "greater than or equal" helper.
-------------------------------------------------------------------------------
*/
flag getf2(float128 a, float128 b);

flag
getf2(float128 a, float128 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return float128_le(b, a) - 1;
}

} // namespace pbsd::lib_libc_softfloat::b0016s3
