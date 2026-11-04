/*
 * oracle.c -- reference (specification) build for PBSD batch b0014.
 *
 * The original C sources are concatenated below, with every function renamed
 * with a "ref_" prefix.  Function bodies are otherwise UNMODIFIED (the only
 * textual change inside a body is the ref_ prefix on the names of the
 * functions it calls).
 *
 * The four batch files are one-line wrappers around the NetBSD softfloat
 * comparison primitives (floatx80_eq, float128_eq, float128_lt) and around
 * the types from milieu.h / softfloat.h.  Those primitives are not part of
 * this batch, so the supporting layer they live in is supplied here verbatim
 * from the same softfloat implementation so that the wrappers can be linked
 * and executed.  FLOATX80 and FLOAT128 are both defined, which is what
 * activates all four batch files.
 */

#include <stdint.h>

#define FLOATX80 1
#define FLOAT128 1

/* ------------------------------------------------------------------ */
/* milieu.h: integer types                                            */
/* ------------------------------------------------------------------ */

typedef int flag;
typedef int int32;

typedef uint16_t bits16;
typedef uint64_t bits64;

#define LIT64(a) a##ULL

/* ------------------------------------------------------------------ */
/* softfloat.h: software IEC/IEEE floating-point types and flags      */
/* ------------------------------------------------------------------ */

typedef struct {
	bits16 high;
	bits64 low;
} floatx80;

typedef struct {
	bits64 high, low;
} float128;

enum {
	float_flag_inexact   =  1,
	float_flag_underflow =  2,
	float_flag_overflow  =  4,
	float_flag_divbyzero =  8,
	float_flag_invalid   = 16
};

int ref_float_exception_flags = 0;

void
ref_float_raise(int flags)
{

	ref_float_exception_flags |= flags;
}

/* ------------------------------------------------------------------ */
/* softfloat.c: floatx80 field extraction                             */
/* ------------------------------------------------------------------ */

bits64
ref_extractFloatx80Frac(floatx80 a)
{

	return a.low;
}

int32
ref_extractFloatx80Exp(floatx80 a)
{

	return a.high & 0x7FFF;
}

flag
ref_extractFloatx80Sign(floatx80 a)
{

	return a.high >> 15;
}

/* ------------------------------------------------------------------ */
/* softfloat.c: float128 field extraction                             */
/* ------------------------------------------------------------------ */

bits64
ref_extractFloat128Frac1(float128 a)
{

	return a.low;
}

bits64
ref_extractFloat128Frac0(float128 a)
{

	return a.high & LIT64(0x0000FFFFFFFFFFFF);
}

int32
ref_extractFloat128Exp(float128 a)
{

	return (a.high >> 48) & 0x7FFF;
}

flag
ref_extractFloat128Sign(float128 a)
{

	return a.high >> 63;
}

/* ------------------------------------------------------------------ */
/* softfloat-macros: 128-bit unsigned compare                         */
/* ------------------------------------------------------------------ */

flag
ref_lt128(bits64 a0, bits64 a1, bits64 b0, bits64 b1)
{

	return (a0 < b0) || ((a0 == b0) && (a1 < b1));
}

/* ------------------------------------------------------------------ */
/* softfloat-specialize: signaling NaN predicates                     */
/* ------------------------------------------------------------------ */

flag
ref_floatx80_is_signaling_nan(floatx80 a)
{
	bits64 aLow;

	aLow = a.low & ~LIT64(0x4000000000000000);
	return
	       ((a.high & 0x7FFF) == 0x7FFF)
	    && (bits64) (aLow << 1)
	    && (a.low == aLow);
}

flag
ref_float128_is_signaling_nan(float128 a)
{

	return
	       (((a.high >> 47) & 0xFFFF) == 0xFFFE)
	    && (a.low || (a.high & LIT64(0x00007FFFFFFFFFFF)));
}

/* ------------------------------------------------------------------ */
/* softfloat.c: the comparison primitives used by this batch           */
/* ------------------------------------------------------------------ */

flag
ref_floatx80_eq(floatx80 a, floatx80 b)
{

	if (   (   (ref_extractFloatx80Exp(a) == 0x7FFF)
		&& (bits64) (ref_extractFloatx80Frac(a) << 1))
	    || (   (ref_extractFloatx80Exp(b) == 0x7FFF)
		&& (bits64) (ref_extractFloatx80Frac(b) << 1))
	   ) {
		if (   ref_floatx80_is_signaling_nan(a)
		    || ref_floatx80_is_signaling_nan(b)) {
			ref_float_raise(float_flag_invalid);
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
ref_float128_eq(float128 a, float128 b)
{

	if (   (   (ref_extractFloat128Exp(a) == 0x7FFF)
		&& (ref_extractFloat128Frac0(a) | ref_extractFloat128Frac1(a)))
	    || (   (ref_extractFloat128Exp(b) == 0x7FFF)
		&& (ref_extractFloat128Frac0(b) | ref_extractFloat128Frac1(b)))
	   ) {
		if (   ref_float128_is_signaling_nan(a)
		    || ref_float128_is_signaling_nan(b)) {
			ref_float_raise(float_flag_invalid);
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
ref_float128_lt(float128 a, float128 b)
{
	flag aSign, bSign;

	if (   (   (ref_extractFloat128Exp(a) == 0x7FFF)
		&& (ref_extractFloat128Frac0(a) | ref_extractFloat128Frac1(a)))
	    || (   (ref_extractFloat128Exp(b) == 0x7FFF)
		&& (ref_extractFloat128Frac0(b) | ref_extractFloat128Frac1(b)))
	   ) {
		ref_float_raise(float_flag_invalid);
		return 0;
	}
	aSign = ref_extractFloat128Sign(a);
	bSign = ref_extractFloat128Sign(b);
	if (aSign != bSign) {
		return
		       aSign
		    && (((bits64) ((a.high | b.high) << 1) | a.low | b.low)
			!= 0);
	}
	return
	      aSign ? ref_lt128(b.high, b.low, a.high, a.low)
	    : ref_lt128(a.high, a.low, b.high, b.low);
}

/* ================================================================== */
/* $NetBSD: nexf2.c,v 1.2 2004/09/27 10:16:24 he Exp $                 */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#ifdef FLOATX80

flag ref___nexf2(floatx80, floatx80);

flag
ref___nexf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says a != b */
	return !ref_floatx80_eq(a, b);
}
#endif /* FLOATX80 */

/* ================================================================== */
/* $NetBSD: gttf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $               */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#ifdef FLOAT128

flag ref___gttf2(float128, float128);

flag
ref___gttf2(float128 a, float128 b)
{

	/* libgcc1.c says a > b */
	return ref_float128_lt(b, a);
}

#endif /* FLOAT128 */

/* ================================================================== */
/* $NetBSD: netf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $               */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#ifdef FLOAT128

flag ref___netf2(float128, float128);

flag
ref___netf2(float128 a, float128 b)
{

	/* libgcc1.c says a != b */
	return !ref_float128_eq(a, b);
}

#endif /* FLOAT128 */

/* ================================================================== */
/* $NetBSD: eqtf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $               */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#ifdef FLOAT128
flag ref___eqtf2(float128, float128);

flag
ref___eqtf2(float128 a, float128 b)
{

	/* libgcc1.c says !(a == b) */
	return !ref_float128_eq(a, b);
}
#endif /* FLOAT128 */
