/*
 * PBSD migration batch b0011 -- reference oracle.
 *
 * The four batch sources below are reproduced verbatim from HardenedBSD
 * (lib/libc/softfloat), with every function renamed with a "ref_" prefix.
 * Function bodies are UNMODIFIED.
 *
 * Everything above the "batch sources" banner is the softfloat runtime that
 * those sources depend on (types from milieu.h/softfloat.h, and the two
 * comparison primitives float64_eq()/float32_le() from bits64/softfloat.c).
 * It is not part of the batch; it is the library the batch links against,
 * and port.cppm links against these same definitions.
 */

#include <stdint.h>

/* ------------------------------------------------------------------ */
/* softfloat types (milieu.h / softfloat.h, bits64 configuration)      */
/* ------------------------------------------------------------------ */

typedef char flag;
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint32_t bits32;
typedef int32_t sbits32;
typedef uint64_t bits64;
typedef int64_t sbits64;

typedef bits32 float32;
typedef bits64 float64;

#define LIT64(a) a##ULL

/*
 * In the bits64 configuration float64 already is a 64-bit integer, so
 * mangling between bit pattern and float64 is the identity.
 */
#define FLOAT64_MANGLE(a)	(a)
#define FLOAT64_DEMANGLE(a)	(a)

enum {
	float_flag_inexact   =  1,
	float_flag_underflow =  2,
	float_flag_overflow  =  4,
	float_flag_divbyzero =  8,
	float_flag_invalid   = 16
};

int8 float_exception_flags = 0;

void
float_raise(int8 flags)
{

	float_exception_flags |= flags;
}

/* ------------------------------------------------------------------ */
/* softfloat primitives (bits64/softfloat.c, softfloat-specialize)     */
/* ------------------------------------------------------------------ */

#define extractFloat32Frac(a)	((a) & 0x007FFFFF)
#define extractFloat32Exp(a)	(((a) >> 23) & 0xFF)
#define extractFloat32Sign(a)	((a) >> 31)

#define extractFloat64Frac(a)	((a) & LIT64(0x000FFFFFFFFFFFFF))
#define extractFloat64Exp(a)	(((a) >> 52) & 0x7FF)
#define extractFloat64Sign(a)	((a) >> 63)

flag
float32_is_signaling_nan(float32 a)
{

	return (((a >> 22) & 0x1FF) == 0x1FE) && (a & 0x003FFFFF);
}

flag
float64_is_signaling_nan(float64 a)
{

	return ((((bits32)(a >> 51)) & 0xFFF) == 0xFFE)
	    && (a & LIT64(0x0007FFFFFFFFFFFF));
}

flag
float64_eq(float64 a, float64 b)
{

	if (((extractFloat64Exp(a) == 0x7FF) && extractFloat64Frac(a))
	    || ((extractFloat64Exp(b) == 0x7FF) && extractFloat64Frac(b))) {
		if (float64_is_signaling_nan(a) || float64_is_signaling_nan(b))
			float_raise(float_flag_invalid);
		return 0;
	}
	return (a == b) || ((bits64)((a | b) << 1) == 0);
}

flag
float32_le(float32 a, float32 b)
{
	flag aSign, bSign;

	if (((extractFloat32Exp(a) == 0xFF) && extractFloat32Frac(a))
	    || ((extractFloat32Exp(b) == 0xFF) && extractFloat32Frac(b))) {
		float_raise(float_flag_invalid);
		return 0;
	}
	aSign = extractFloat32Sign(a);
	bSign = extractFloat32Sign(b);
	if (aSign != bSign)
		return aSign || ((bits32)((a | b) << 1) == 0);
	return (a == b) || (aSign ^ (a < b));
}

/* ================================================================== */
/* batch sources                                                      */
/* ================================================================== */

/* $NetBSD: eqdf2.c,v 1.1 2000/06/06 08:15:02 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref___eqdf2(float64, float64);

flag
ref___eqdf2(float64 a, float64 b)
{

	/* libgcc1.c says !(a == b) */
	return !float64_eq(a, b);
}

/* $NetBSD: negdf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

float64 ref___negdf2(float64);

float64
ref___negdf2(float64 a)
{

	/* libgcc1.c says -a */
	return a ^ FLOAT64_MANGLE(0x8000000000000000ULL);
}

/* $NetBSD: gesf2.c,v 1.1 2000/06/06 08:15:05 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref___gesf2(float32, float32);

flag
ref___gesf2(float32 a, float32 b)
{

	/* libgcc1.c says (a >= b) - 1 */
	return float32_le(b, a) - 1;
}

/* $NetBSD: lesf2.c,v 1.1 2000/06/06 08:15:06 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

flag ref___lesf2(float32, float32);

flag
ref___lesf2(float32 a, float32 b)
{

	/* libgcc1.c says 1 - (a <= b) */
	return 1 - float32_le(a, b);
}
