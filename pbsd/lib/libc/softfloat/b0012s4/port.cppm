/* $NetBSD: gtxf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

/*
 * PBSD port of hbsd/src/lib/libc/softfloat/gtxf2.c to C++23.
 *
 * gtxf2.c consists of a single function whose entire body is a call to
 * floatx80_lt() on the softfloat `floatx80' type.  Neither the type nor
 * floatx80_lt() live in this batch, so the softfloat pieces they need are
 * carried here verbatim from the SoftFloat sources those headers describe
 * (bits64/softfloat.c, softfloat-macros, softfloat-specialize).  They are
 * reproduced exactly, bugs and all: the `flag' return type, the int
 * promotion in `(a.high | b.high) << 1', the invalid-operation raise on
 * signalling and quiet NaNs alike, and the -0 == +0 special case.
 */

/*
 * This is a derivative work of SoftFloat, by John R. Hauser:
 *
 * ===========================================================================
 * This C source file is part of the SoftFloat IEC/IEEE Floating-point
 * Arithmetic Package, Release 2a.
 *
 * Written by John R. Hauser.  This work was made possible in part by the
 * International Computer Science Institute, located at Suite 600, 1947 Center
 * Street, Berkeley, California 94704.  Funding was partially provided by the
 * National Science Foundation under grant MIP-9311980.  The original version
 * of this code was written as part of a project to build a fixed-point vector
 * processor in collaboration with the University of California at Berkeley,
 * overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
 * is available through the Web page
 * `http://HTTP.CS.Berkeley.EDU/~jhauser/arithmetic/SoftFloat.html'.
 *
 * THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort
 * has been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT
 * TIMES RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO
 * PERSONS AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL
 * LOSSES, COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO
 * SATISFY ALL OTHER RESPONSIBILITIES BELOW.
 *
 * Derivative works are acceptable, even for commercial purposes, so long as
 * (1) they include prominent notice that the work is derivative, and (2) they
 * include prominent notice akin to these three paragraphs for those parts of
 * this code that are retained.
 * ===========================================================================
 */

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0012s4;

export namespace pbsd::lib_libc_softfloat::b0012s4 {

/* -------------------------------------------------------------------------
 * milieu.h / softfloat-for-gcc.h: primitive types.
 * ------------------------------------------------------------------------- */

typedef int flag;
typedef int int8;
typedef int uint8;
typedef std::uint16_t bits16;
typedef std::uint32_t bits32;
typedef std::uint64_t bits64;

#define FLOATX80 1

/* -------------------------------------------------------------------------
 * softfloat.h: the extended double-precision type and exception flags.
 * ------------------------------------------------------------------------- */

struct floatx80 {
	bits64 low;
	bits16 high;
};

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

/* -------------------------------------------------------------------------
 * softfloat-macros: 128-bit comparison.
 * ------------------------------------------------------------------------- */

inline flag
lt128( bits64 a0, bits64 a1, bits64 b0, bits64 b1 )
{

	return ( a0 < b0 ) || ( ( a0 == b0 ) && ( a1 < b1 ) );
}

/* -------------------------------------------------------------------------
 * bits64/softfloat.c: floatx80 field extraction and the `<' predicate that
 * __gtxf2 is defined in terms of.
 * ------------------------------------------------------------------------- */

inline bits64
extractFloatx80Frac( floatx80 a )
{

	return a.low;
}

inline std::int32_t
extractFloatx80Exp( floatx80 a )
{

	return a.high & 0x7FFF;
}

inline flag
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

/* -------------------------------------------------------------------------
 * gtxf2.c: the batch function itself.  Named `gtxf2' rather than `__gtxf2'
 * because leading double underscores are reserved to the C++ implementation;
 * behaviour is unchanged.
 * ------------------------------------------------------------------------- */

#ifdef FLOATX80

flag gtxf2(floatx80, floatx80);

flag
gtxf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says a > b */
	return floatx80_lt(b, a);
}
#endif /* FLOATX80 */

} /* namespace pbsd::lib_libc_softfloat::b0012s4 */
