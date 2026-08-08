/*
 * oracle.c -- reference implementation for PBSD batch b0079s4.
 *
 * The original HardenedBSD source from lib/msun/src/s_copysignf.c is
 * reproduced below.  The function has been renamed with a "ref_" prefix; the
 * function body is otherwise byte-for-byte unmodified.  The only additions are
 * the declarations and macros that the original file obtained from "math.h" and
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

typedef union
{
	float value;
	/* FIXME: Assumes 32 bit int.  */
	unsigned int word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */

#define GET_FLOAT_WORD(i,d)					\
do {								\
	ieee_float_shape_type gf_u;				\
	gf_u.value = (d);					\
	(i) = gf_u.word;					\
} while (0)

/* Set a float from a 32 bit int.  */

#define SET_FLOAT_WORD(d,i)					\
do {								\
	ieee_float_shape_type sf_u;				\
	sf_u.word = (i);					\
	(d) = sf_u.value;					\
} while (0)

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
