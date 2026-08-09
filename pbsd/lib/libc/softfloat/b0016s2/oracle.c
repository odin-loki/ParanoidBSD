/*
 * oracle.c -- reference (specification) build for PBSD batch b0016s2.
 *
 * Contents:
 *   lib/libc/softfloat/gexf2.c            -- the batch source
 *
 * Supporting definitions required to link the batch source are taken
 * verbatim from the SoftFloat sources it is compiled against:
 *   lib/libc/softfloat/templates/milieu.h      (integer types, TRUE/FALSE)
 *   lib/libc/softfloat/templates/softfloat.h   (floatx80, exception flags)
 *   lib/libc/softfloat/softfloat-specialize     (float_raise)
 *   lib/libc/softfloat/bits64/softfloat-macros  (le128)
 *   lib/libc/softfloat/bits64/softfloat.c       (extractFloatx80*, floatx80_le)
 *
 * Every function is renamed with a ref_ prefix; the renaming is done with
 * #define so that no function body is altered in any way.
 */

/* $NetBSD: gexf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
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
#include <string.h>
#include <unistd.h>

/*
 * ---------------------------------------------------------------------------
 * Renaming of every ported entity.  Done with the preprocessor so the
 * function bodies below remain byte-for-byte the originals.
 * ---------------------------------------------------------------------------
 */
#define float_exception_flags	ref_float_exception_flags
#define float_exception_mask	ref_float_exception_mask
#define float_raise		ref_float_raise
#define extractFloatx80Frac	ref_extractFloatx80Frac
#define extractFloatx80Exp	ref_extractFloatx80Exp
#define extractFloatx80Sign	ref_extractFloatx80Sign
#define le128			ref_le128
#define floatx80_le		ref_floatx80_le
#define __gexf2			ref___gexf2

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

#define FLOATX80

/*
 * ---------------------------------------------------------------------------
 * softfloat.h: the extended double-precision format and the exception flags.
 * ---------------------------------------------------------------------------
 */
typedef struct {
    bits16 high;
    bits64 low;
} floatx80;

enum {
    float_flag_inexact   =  1,
    float_flag_underflow =  2,
    float_flag_overflow  =  4,
    float_flag_divbyzero =  8,
    float_flag_invalid   = 16
};

int8 float_exception_flags = 0;

/*
 * ---------------------------------------------------------------------------
 * softfloat-specialize: float_raise().
 * ---------------------------------------------------------------------------
 */
int float_exception_mask = 0;
void float_raise( int flags )
{

    float_exception_flags |= flags;

    if ( flags & float_exception_mask ) {
#if 0
	siginfo_t info;
	memset(&info, 0, sizeof info);
	info.si_signo = SIGFPE;
	info.si_pid = getpid();
	info.si_uid = geteuid();
	if (flags & float_flag_underflow)
	    info.si_code = FPE_FLTUND;
	else if (flags & float_flag_overflow)
	    info.si_code = FPE_FLTOVF;
	else if (flags & float_flag_divbyzero)
	    info.si_code = FPE_FLTDIV;
	else if (flags & float_flag_invalid)
	    info.si_code = FPE_FLTINV;
	else if (flags & float_flag_inexact)
	    info.si_code = FPE_FLTRES;
	sigqueueinfo(getpid(), &info);
#else
	raise( SIGFPE );
#endif
    }
}

/*
 * ---------------------------------------------------------------------------
 * bits64/softfloat-macros: le128().
 * ---------------------------------------------------------------------------
 */

/*
-------------------------------------------------------------------------------
Returns 1 if the 128-bit value formed by concatenating `a0' and `a1' is less
than or equal to the 128-bit value formed by concatenating `b0' and `b1'.
Otherwise, returns 0.
-------------------------------------------------------------------------------
*/
INLINE flag le128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 )
{

    return ( a0 < b0 ) || ( ( a0 == b0 ) && ( a1 <= b1 ) );

}

/*
 * ---------------------------------------------------------------------------
 * bits64/softfloat.c: floatx80 field extraction and floatx80_le().
 * ---------------------------------------------------------------------------
 */

/*
-------------------------------------------------------------------------------
Returns the fraction bits of the extended double-precision floating-point
value `a'.
-------------------------------------------------------------------------------
*/
INLINE bits64 extractFloatx80Frac( floatx80 a )
{

    return a.low;

}

/*
-------------------------------------------------------------------------------
Returns the exponent bits of the extended double-precision floating-point
value `a'.
-------------------------------------------------------------------------------
*/
INLINE int32 extractFloatx80Exp( floatx80 a )
{

    return a.high & 0x7FFF;

}

/*
-------------------------------------------------------------------------------
Returns the sign bit of the extended double-precision floating-point value
`a'.
-------------------------------------------------------------------------------
*/
INLINE flag extractFloatx80Sign( floatx80 a )
{

    return a.high>>15;

}

/*
-------------------------------------------------------------------------------
Returns 1 if the extended double-precision floating-point value `a' is less
than or equal to the corresponding value `b', and 0 otherwise.  The
comparison is performed according to the IEC/IEEE Standard for Binary
Floating-Point Arithmetic.
-------------------------------------------------------------------------------
*/
flag floatx80_le( floatx80 a, floatx80 b )
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
            || (    ( ( (bits16) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
                 == 0 );
    }
    return
          aSign ? le128( b.high, b.low, a.high, a.low )
        : le128( a.high, a.low, b.high, b.low );

}

/*
 * ---------------------------------------------------------------------------
 * lib/libc/softfloat/gexf2.c -- the batch source.
 *
 * softfloat-for-gcc.h renames floatx80_ge to __gexf2 and floatx80_le to
 * __lexf2; the body below is exactly as it appears in gexf2.c.
 * ---------------------------------------------------------------------------
 */

#ifdef FLOATX80

flag __gexf2(floatx80, floatx80);

flag
__gexf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return floatx80_le(b, a) - 1;
}
#endif /* FLOATX80 */
