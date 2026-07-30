// port.cppm -- PBSD C++23 module for HardenedBSD batch b0014.
//
// Faithful port of:
//   lib/libc/softfloat/nexf2.c
//   lib/libc/softfloat/gttf2.c
//   lib/libc/softfloat/netf2.c
//   lib/libc/softfloat/eqtf2.c
//
// The four batch files are one-line wrappers around the softfloat comparison
// primitives declared by softfloat.h (floatx80_eq, float128_eq, float128_lt)
// and around the types of milieu.h.  Those primitives are not part of this
// batch; the supporting layer is reproduced here unchanged from the same
// softfloat implementation so that the wrappers are executable.  FLOATX80 and
// FLOAT128 are both considered defined, which is what activates all four
// batch files.
//
// Behaviour, signedness, evaluation order and the exact form of every
// expression are preserved.  Nothing is improved.

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0014;

export namespace pbsd::lib_libc_softfloat::b0014 {

// ---------------------------------------------------------------------
// milieu.h: integer types
// ---------------------------------------------------------------------

using flag = int;
using int32 = int;

using bits16 = std::uint16_t;
using bits64 = std::uint64_t;

// ---------------------------------------------------------------------
// softfloat.h: software IEC/IEEE floating-point types and flags
// ---------------------------------------------------------------------

struct floatx80 {
	bits16 high;
	bits64 low;
};

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

int float_exception_flags = 0;

void
float_raise(int flags)
{

	float_exception_flags |= flags;
}

int
get_float_exception_flags()
{

	return float_exception_flags;
}

void
set_float_exception_flags(int flags)
{

	float_exception_flags = flags;
}

// ---------------------------------------------------------------------
// softfloat.c: floatx80 field extraction
// ---------------------------------------------------------------------

bits64
extractFloatx80Frac(floatx80 a)
{

	return a.low;
}

int32
extractFloatx80Exp(floatx80 a)
{

	return a.high & 0x7FFF;
}

flag
extractFloatx80Sign(floatx80 a)
{

	return a.high >> 15;
}

// ---------------------------------------------------------------------
// softfloat.c: float128 field extraction
// ---------------------------------------------------------------------

bits64
extractFloat128Frac1(float128 a)
{

	return a.low;
}

bits64
extractFloat128Frac0(float128 a)
{

	return a.high & 0x0000FFFFFFFFFFFFULL;
}

int32
extractFloat128Exp(float128 a)
{

	return (a.high >> 48) & 0x7FFF;
}

flag
extractFloat128Sign(float128 a)
{

	return a.high >> 63;
}

// ---------------------------------------------------------------------
// softfloat-macros: 128-bit unsigned compare
// ---------------------------------------------------------------------

flag
lt128(bits64 a0, bits64 a1, bits64 b0, bits64 b1)
{

	return (a0 < b0) || ((a0 == b0) && (a1 < b1));
}

// ---------------------------------------------------------------------
// softfloat-specialize: signaling NaN predicates
// ---------------------------------------------------------------------

flag
floatx80_is_signaling_nan(floatx80 a)
{
	bits64 aLow;

	aLow = a.low & ~0x4000000000000000ULL;
	return
	       ((a.high & 0x7FFF) == 0x7FFF)
	    && (bits64) (aLow << 1)
	    && (a.low == aLow);
}

flag
float128_is_signaling_nan(float128 a)
{

	return
	       (((a.high >> 47) & 0xFFFF) == 0xFFFE)
	    && (a.low || (a.high & 0x00007FFFFFFFFFFFULL));
}

// ---------------------------------------------------------------------
// softfloat.c: the comparison primitives used by this batch
// ---------------------------------------------------------------------

flag
floatx80_eq(floatx80 a, floatx80 b)
{

	if (   (   (extractFloatx80Exp(a) == 0x7FFF)
		&& (bits64) (extractFloatx80Frac(a) << 1))
	    || (   (extractFloatx80Exp(b) == 0x7FFF)
		&& (bits64) (extractFloatx80Frac(b) << 1))
	   ) {
		if (   floatx80_is_signaling_nan(a)
		    || floatx80_is_signaling_nan(b)) {
			float_raise(float_flag_invalid);
		}
		return 0;
	}
	return
	       (a.low == b.low)
	    && (   (a.high == b.high)
		|| (   (a.low == 0)
		    && ((bits16) ((a.high | b.high) << 1) == 0))
	       );
}

flag
float128_eq(float128 a, float128 b)
{

	if (   (   (extractFloat128Exp(a) == 0x7FFF)
		&& (extractFloat128Frac0(a) | extractFloat128Frac1(a)))
	    || (   (extractFloat128Exp(b) == 0x7FFF)
		&& (extractFloat128Frac0(b) | extractFloat128Frac1(b)))
	   ) {
		if (   float128_is_signaling_nan(a)
		    || float128_is_signaling_nan(b)) {
			float_raise(float_flag_invalid);
		}
		return 0;
	}
	return
	       (a.low == b.low)
	    && (   (a.high == b.high)
		|| (   (a.low == 0)
		    && ((bits64) ((a.high | b.high) << 1) == 0))
	       );
}

flag
float128_lt(float128 a, float128 b)
{
	flag aSign, bSign;

	if (   (   (extractFloat128Exp(a) == 0x7FFF)
		&& (extractFloat128Frac0(a) | extractFloat128Frac1(a)))
	    || (   (extractFloat128Exp(b) == 0x7FFF)
		&& (extractFloat128Frac0(b) | extractFloat128Frac1(b)))
	   ) {
		float_raise(float_flag_invalid);
		return 0;
	}
	aSign = extractFloat128Sign(a);
	bSign = extractFloat128Sign(b);
	if (aSign != bSign) {
		return
		       aSign
		    && (((bits64) ((a.high | b.high) << 1) | a.low | b.low)
			!= 0);
	}
	return
	      aSign ? lt128(b.high, b.low, a.high, a.low)
	    : lt128(a.high, a.low, b.high, b.low);
}

// =====================================================================
// $NetBSD: nexf2.c,v 1.2 2004/09/27 10:16:24 he Exp $
//
// Written by Ben Harris, 2000.  This file is in the Public Domain.

flag
__nexf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says a != b */
	return !floatx80_eq(a, b);
}

// =====================================================================
// $NetBSD: gttf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $
//
// Written by Matt Thomas, 2011.  This file is in the Public Domain.

flag
__gttf2(float128 a, float128 b)
{

	/* libgcc1.c says a > b */
	return float128_lt(b, a);
}

// =====================================================================
// $NetBSD: netf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $
//
// Written by Matt Thomas, 2011.  This file is in the Public Domain.

flag
__netf2(float128 a, float128 b)
{

	/* libgcc1.c says a != b */
	return !float128_eq(a, b);
}

// =====================================================================
// $NetBSD: eqtf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $
//
// Written by Matt Thomas, 2011.  This file is in the Public Domain.

flag
__eqtf2(float128 a, float128 b)
{

	/* libgcc1.c says !(a == b) */
	return !float128_eq(a, b);
}

} // namespace pbsd::lib_libc_softfloat::b0014
