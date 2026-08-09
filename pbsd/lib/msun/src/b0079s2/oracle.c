/*
 * oracle.c -- reference implementation for PBSD batch b0079s2.
 *
 * The original HardenedBSD sources concatenated verbatim; every exported
 * function has been renamed with a "ref_" prefix and nothing else has been
 * touched.  Defines that would otherwise come from msun's private headers
 * (GET_FLOAT_WORD) and typedefs that would come from <sys/types.h> are
 * supplied below so that the file builds standalone with cc -std=c11.
 *
 * THIS FILE IS THE SPECIFICATION.  DO NOT MODIFY ANY FUNCTION BODY.
 */

#include <stdint.h>

#ifndef __BSD_TYPES_DEFINED__
typedef uint32_t u_int32_t;
#endif

/* From lib/msun/src/math_private.h, unmodified. */
#define	GET_FLOAT_WORD(i,d)					\
do {								\
  union {							\
    float value;						\
    u_int32_t word;						\
  } gf_u;							\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

/* ------------------------------------------------------------------ */
/* lib/msun/src/s_finitef.c                                            */
/* ------------------------------------------------------------------ */

/* s_finitef.c -- float version of s_finite.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * finitef(x) returns 1 is x is finite, else 0;
 * no branching!
 */

	int ref_finitef(float x)
{
	int32_t ix;
	GET_FLOAT_WORD(ix,x);
	return (int)((u_int32_t)((ix&0x7fffffff)-0x7f800000)>>31);
}
