/*
 * oracle.c -- reference implementation for PBSD batch b0079.
 *
 * The original HardenedBSD sources from lib/msun/src are concatenated below.
 * Every function has been renamed with a "ref_" prefix; the function bodies
 * are otherwise byte-for-byte unmodified.  The only additions are the
 * declarations and macros that the original files obtained from "math.h" and
 * "math_private.h" (GET_FLOAT_WORD, SET_FLOAT_WORD, u_int32_t).
 *
 * This file is the specification.  Do not modify any function body.
 */

#include <math.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

typedef union {
	float value;
	uint32_t word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float. */
#define GET_FLOAT_WORD(i, d)				\
do {							\
	ieee_float_shape_type gf_u;			\
	gf_u.value = (d);				\
	(i) = gf_u.word;				\
} while (0)

/* Set a float from a 32 bit int. */
#define SET_FLOAT_WORD(d, i)				\
do {							\
	ieee_float_shape_type sf_u;			\
	sf_u.word = (i);				\
	(d) = sf_u.value;				\
} while (0)

/* ------------------------------------------------------------------ */
/* e_lgammaf.c							      */
/* ------------------------------------------------------------------ */

/* e_lgammaf.c -- float version of e_lgamma.c.
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

/* lgammaf(x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call lgammaf_r
 */

extern int signgam;

float
ref_lgammaf(float x)
{
	return lgammaf_r(x,&signgam);
}

/* ------------------------------------------------------------------ */
/* s_finitef.c							      */
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

/* ------------------------------------------------------------------ */
/* e_gammaf_r.c							      */
/* ------------------------------------------------------------------ */

/* e_gammaf_r.c -- float version of e_gamma_r.c.
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

/* gammaf_r(x, signgamp)
 * Reentrant version of the logarithm of the Gamma function
 * with user provide pointer for the sign of Gamma(x).
 *
 * Method: See lgammaf_r
 */

float
ref_gammaf_r(float x, int *signgamp)
{
	return lgammaf_r(x,signgamp);
}

/* ------------------------------------------------------------------ */
/* s_copysignf.c						      */
/* ------------------------------------------------------------------ */

/* s_copysignf.c -- float version of s_copysign.c.
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
 * copysignf(float x, float y)
 * copysignf(x,y) returns a value with the magnitude of x and
 * with the sign bit of y.
 */

float
ref_copysignf(float x, float y)
{
	u_int32_t ix,iy;
	GET_FLOAT_WORD(ix,x);
	GET_FLOAT_WORD(iy,y);
	SET_FLOAT_WORD(x,(ix&0x7fffffff)|(iy&0x80000000));
        return x;
}
