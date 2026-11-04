/*
 * oracle.c -- reference (specification) build for PBSD batch b0016s1.
 *
 * Contents:
 *   lib/libc/softfloat/lttf2.c            -- the batch source
 *
 * Supporting definitions required to link the batch source are taken
 * verbatim from the SoftFloat sources it is compiled against:
 *   lib/libc/softfloat/templates/milieu.h      (integer types, TRUE/FALSE)
 *   lib/libc/softfloat/templates/softfloat.h   (float128, exception flags)
 *   lib/libc/softfloat/softfloat-specialize     (float_raise)
 *   lib/libc/softfloat/bits64/softfloat-macros  (lt128)
 *   lib/libc/softfloat/bits64/softfloat.c       (extractFloat128*, float128_lt)
 *
 * Every function is renamed with a ref_ prefix; the renaming is done with
 * #define so that no function body is altered in any way.
 */

/* $NetBSD: lttf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

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

#include <signal.h>

/*
 * ---------------------------------------------------------------------------
 * Renaming of every ported entity.  Done with the preprocessor so the
 * function bodies below remain byte-for-byte the originals.
 * ---------------------------------------------------------------------------
 */
#define float_exception_flags	__softfloat_float_exception_flags
#define float_exception_mask	ref_float_exception_mask
#define float_raise		ref_float_raise
#define extractFloat128Frac1	ref_extractFloat128Frac1
#define extractFloat128Frac0	ref_extractFloat128Frac0
#define extractFloat128Exp	ref_extractFloat128Exp
#define extractFloat128Sign	ref_extractFloat128Sign
#define lt128			ref_lt128
#define float128_lt		ref_float128_lt
#define __lttf2			ref___lttf2

/*
 * ---------------------------------------------------------------------------
 * milieu.h / processors template: integer types and boolean literals.
 * ---------------------------------------------------------------------------
 */
typedef int flag;
typedef int uint8;
typedef int int8;
typedef int uint16;
typedef int int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned long long int uint64;
typedef signed long long int int64;

typedef unsigned char bits8;
typedef signed char sbits8;
typedef unsigned short int bits16;
typedef signed short int sbits16;
typedef unsigned int bits32;
typedef signed int sbits32;
typedef unsigned long long int bits64;
typedef signed long long int sbits64;

#define LIT64( a ) a##ULL
#define INLINE static

enum {
    FALSE = 0,
    TRUE  = 1
};

#define FLOAT128

/*
 * ---------------------------------------------------------------------------
 * softfloat.h: the quadruple-precision format and the exception flags.
 * ---------------------------------------------------------------------------
 */
typedef struct {
    bits64 high;
    bits64 low;
} float128;

enum {
    float_flag_inexact   =  1,
    float_flag_underflow =  2,
    float_flag_overflow  =  4,
    float_flag_divbyzero =  8,
    float_flag_invalid   = 16
};

int __softfloat_float_exception_flags = 0;

/*
 * ---------------------------------------------------------------------------
 * softfloat-specialize: float_raise().
 * ---------------------------------------------------------------------------
 */
int ref_float_exception_mask = 0;
void float_raise( int flags )
{

    float_exception_flags |= flags;

    if ( flags & float_exception_mask ) {
	raise( SIGFPE );
    }
}

/*
 * ---------------------------------------------------------------------------
 * bits64/softfloat-macros: lt128().
 * ---------------------------------------------------------------------------
 */

/*
-------------------------------------------------------------------------------
Returns 1 if the 128-bit value formed by concatenating `a0' and `a1' is less
than the 128-bit value formed by concatenating `b0' and `b1'.  Otherwise,
returns 0.
-------------------------------------------------------------------------------
*/
INLINE flag lt128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 )
{

    return ( a0 < b0 ) || ( ( a0 == b0 ) && ( a1 < b1 ) );

}

/*
 * ---------------------------------------------------------------------------
 * bits64/softfloat.c: extractFloat128* and float128_lt().
 * ---------------------------------------------------------------------------
 */

/*
-------------------------------------------------------------------------------
Returns the least-significant 64 fraction bits of the quadruple-precision
floating-point value `a'.
-------------------------------------------------------------------------------
*/
INLINE bits64 extractFloat128Frac1( float128 a )
{

    return a.low;

}

/*
-------------------------------------------------------------------------------
Returns the most-significant 48 fraction bits of the quadruple-precision
floating-point value `a'.
-------------------------------------------------------------------------------
*/
INLINE bits64 extractFloat128Frac0( float128 a )
{

    return a.high & LIT64( 0x0000FFFFFFFFFFFF );

}

/*
-------------------------------------------------------------------------------
Returns the exponent bits of the quadruple-precision floating-point value
`a'.
-------------------------------------------------------------------------------
*/
INLINE int32 extractFloat128Exp( float128 a )
{

    return ( a.high>>48 ) & 0x7FFF;

}

/*
-------------------------------------------------------------------------------
Returns the sign bit of the quadruple-precision floating-point value `a'.
-------------------------------------------------------------------------------
*/
INLINE flag extractFloat128Sign( float128 a )
{

    return a.high>>63;

}

/*
-------------------------------------------------------------------------------
Returns 1 if the quadruple-precision floating-point value `a' is less than
the corresponding value `b', and 0 otherwise.  The comparison is performed
according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
-------------------------------------------------------------------------------
*/
flag float128_lt( float128 a, float128 b )
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
            && (    ( ( (bits64) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
                 != 0 );
    }
    return
          aSign ? lt128( b.high, b.low, a.high, a.low )
        : lt128( a.high, a.low, b.high, b.low );

}

/*
 * ---------------------------------------------------------------------------
 * lib/libc/softfloat/lttf2.c -- the batch source.
 *
 * softfloat-for-gcc.h renames float128_lt to __lttf2; the body below is
 * exactly as it appears in lttf2.c.
 * ---------------------------------------------------------------------------
 */

#ifdef FLOAT128

flag __lttf2(float128, float128);

flag
__lttf2(float128 a, float128 b)
{

	/* libgcc1.c says -(a < b) */
	return -float128_lt(a, b);
}

#endif /* FLOAT128 */
