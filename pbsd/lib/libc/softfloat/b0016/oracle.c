/*
 * oracle.c -- reference specification for PBSD batch b0016.
 *
 * The original C sources are concatenated below with every ported function
 * renamed with a "ref_" prefix.  The function bodies are UNMODIFIED.
 *
 * Batch members:
 *	lib/libc/softfloat/lttf2.c	__lttf2 -> ref___lttf2
 *	lib/libc/softfloat/gexf2.c	__gexf2 -> ref___gexf2
 *	lib/libc/softfloat/getf2.c	__getf2 -> ref___getf2
 *	lib/libc/softfloat/letf2.c	__letf2 -> ref___letf2
 *
 * The four batch members are thin wrappers around the softfloat comparison
 * primitives declared by <softfloat.h> and defined in softfloat.c, which is
 * NOT part of this batch.  Those primitives, the milieu.h/softfloat.h types
 * they need, and the float_raise() exception sink are supplied here as the
 * link-time dependency shared by both the oracle and the port, exactly as
 * softfloat.c would supply them to a real libc link.  They keep their real
 * names (no ref_ prefix) because they are not ported functions: the port
 * declares them extern and calls these very definitions, so any observable
 * difference between port and oracle comes from the wrappers alone.
 */

#include <stdint.h>

/* --- milieu.h ---------------------------------------------------------- */

typedef int flag;
typedef uint16_t bits16;
typedef uint32_t bits32;
typedef uint64_t bits64;

#define LIT64(a) a##ULL

/* --- softfloat.h: enabled formats and IEC/IEEE types ------------------- */

#define FLOATX80
#define FLOAT128

typedef struct {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	bits16 high;
	bits64 low;
#else
	bits64 low;
	bits16 high;
#endif
} floatx80;

typedef struct {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	bits64 high, low;
#else
	bits64 low, high;
#endif
} float128;

enum {
	float_flag_inexact   =  1,
	float_flag_underflow =  2,
	float_flag_overflow  =  4,
	float_flag_divbyzero =  8,
	float_flag_invalid   = 16
};

/* --- softfloat-specialize: exception sink ------------------------------ */

int float_exception_flags = 0;

void
float_raise(int flags)
{

	float_exception_flags |= flags;
}

/* --- softfloat-macros: 128-bit magnitude comparison ------------------- */

static flag
le128(bits64 a0, bits64 a1, bits64 b0, bits64 b1)
{

	return (a0 < b0) || ((a0 == b0) && (a1 <= b1));
}

static flag
lt128(bits64 a0, bits64 a1, bits64 b0, bits64 b1)
{

	return (a0 < b0) || ((a0 == b0) && (a1 < b1));
}

/* --- softfloat.c: field extraction ------------------------------------ */

#define extractFloat128Sign(a)	((a).high >> 63)
#define extractFloat128Exp(a)	(((a).high >> 48) & 0x7FFF)
#define extractFloat128Frac0(a)	((a).high & LIT64(0x0000FFFFFFFFFFFF))
#define extractFloat128Frac1(a)	((a).low)

#define extractFloatx80Sign(a)	((a).high >> 15)
#define extractFloatx80Exp(a)	((a).high & 0x7FFF)
#define extractFloatx80Frac(a)	((a).low)

/* --- softfloat.c: the comparison primitives the batch calls ----------- */

flag
float128_lt(float128 a, float128 b)
{
	flag aSign, bSign;

	if (((extractFloat128Exp(a) == 0x7FFF)
	     && (extractFloat128Frac0(a) | extractFloat128Frac1(a)))
	    || ((extractFloat128Exp(b) == 0x7FFF)
		&& (extractFloat128Frac0(b) | extractFloat128Frac1(b)))) {
		float_raise(float_flag_invalid);
		return 0;
	}
	aSign = extractFloat128Sign(a);
	bSign = extractFloat128Sign(b);
	if (aSign != bSign) {
		return aSign
		    && (((bits64) ((a.high | b.high) << 1)
			 | a.low | b.low) != 0);
	}
	return aSign ? lt128(b.high, b.low, a.high, a.low)
	    : lt128(a.high, a.low, b.high, b.low);
}

flag
float128_le(float128 a, float128 b)
{
	flag aSign, bSign;

	if (((extractFloat128Exp(a) == 0x7FFF)
	     && (extractFloat128Frac0(a) | extractFloat128Frac1(a)))
	    || ((extractFloat128Exp(b) == 0x7FFF)
		&& (extractFloat128Frac0(b) | extractFloat128Frac1(b)))) {
		float_raise(float_flag_invalid);
		return 0;
	}
	aSign = extractFloat128Sign(a);
	bSign = extractFloat128Sign(b);
	if (aSign != bSign) {
		return aSign
		    || (((bits64) ((a.high | b.high) << 1)
			 | a.low | b.low) == 0);
	}
	return aSign ? le128(b.high, b.low, a.high, a.low)
	    : le128(a.high, a.low, b.high, b.low);
}

flag
floatx80_le(floatx80 a, floatx80 b)
{
	flag aSign, bSign;

	if (((extractFloatx80Exp(a) == 0x7FFF)
	     && (bits64) (extractFloatx80Frac(a) << 1))
	    || ((extractFloatx80Exp(b) == 0x7FFF)
		&& (bits64) (extractFloatx80Frac(b) << 1))) {
		float_raise(float_flag_invalid);
		return 0;
	}
	aSign = extractFloatx80Sign(a);
	bSign = extractFloatx80Sign(b);
	if (aSign != bSign) {
		return aSign
		    || (((bits16) ((a.high | b.high) << 1)
			 | a.low | b.low) == 0);
	}
	return aSign ? le128(b.high, b.low, a.high, a.low)
	    : le128(a.high, a.low, b.high, b.low);
}

/* ====================================================================== */
/* $NetBSD: lttf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#ifdef FLOAT128

flag ref___lttf2(float128, float128);

flag
ref___lttf2(float128 a, float128 b)
{

	/* libgcc1.c says -(a < b) */
	return -float128_lt(a, b);
}

#endif /* FLOAT128 */

/* ====================================================================== */
/* $NetBSD: gexf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#ifdef FLOATX80

flag ref___gexf2(floatx80, floatx80);

flag
ref___gexf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return floatx80_le(b, a) - 1;
}
#endif /* FLOATX80 */

/* ====================================================================== */
/* $NetBSD: getf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#ifdef FLOAT128

flag ref___getf2(float128, float128);

flag
ref___getf2(float128 a, float128 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return float128_le(b, a) - 1;
}

#endif /* FLOAT128 */

/* ====================================================================== */
/* $NetBSD: letf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#ifdef FLOAT128

flag ref___letf2(float128, float128);

flag
ref___letf2(float128 a, float128 b)
{

	/* libgcc1.c says 1 - (a <= b) */
	return 1 - float128_le(a, b);
}

#endif /* FLOAT128 */
