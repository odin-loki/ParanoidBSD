/*
 * oracle.c -- reference implementation for PBSD batch b0135.
 *
 * The original HardenedBSD sources from lib/msun/src are concatenated below.
 * Every function has been renamed with a "ref_" prefix; the function bodies
 * are otherwise byte-for-byte unmodified.  The only additions are the
 * declarations and macros that the original files obtained from "math.h" and
 * "math_private.h", plus gammaf_r from e_gammaf_r.c (a dependency of e_gammaf.c
 * that is not part of this batch but is required for linking).
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

typedef union {
	double value;
	struct {
		u_int32_t lsw;
		u_int32_t msw;
	} parts;
} ieee_double_shape_type;

#define GET_FLOAT_WORD(i, d)				\
do {							\
	ieee_float_shape_type gf_u;			\
	gf_u.value = (d);				\
	(i) = gf_u.word;				\
} while (0)

#define SET_FLOAT_WORD(d, i)				\
do {							\
	ieee_float_shape_type sf_u;			\
	sf_u.word = (i);				\
	(d) = sf_u.value;				\
} while (0)

#define GET_HIGH_WORD(i, d)				\
do {							\
	ieee_double_shape_type gh_u;			\
	gh_u.value = (d);				\
	(i) = gh_u.parts.msw;				\
} while (0)

#define SET_HIGH_WORD(d, v)				\
do {							\
	ieee_double_shape_type sh_u;			\
	sh_u.value = (d);				\
	sh_u.parts.msw = (v);				\
	(d) = sh_u.value;				\
} while (0)

extern int signgam;

/* Dependency of e_gammaf.c (e_gammaf_r.c). */
float
gammaf_r(float x, int *signgamp)
{
	return lgammaf_r(x,signgamp);
}

/* ------------------------------------------------------------------ */
/* s_scalbnf.c							      */
/* ------------------------------------------------------------------ */

/*
 * Copyright (c) 2005-2020 Rich Felker, et al.
 *
 * SPDX-License-Identifier: MIT
 *
 * Please see https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT
 * for all contributors to musl.
 */

float ref_scalbnf(float x, int n)
{
	union {float f; uint32_t i;} u;
	float_t y = x;

	if (n > 127) {
		y *= 0x1p127f;
		n -= 127;
		if (n > 127) {
			y *= 0x1p127f;
			n -= 127;
			if (n > 127)
				n = 127;
		}
	} else if (n < -126) {
		y *= 0x1p-126f * 0x1p24f;
		n += 126 - 24;
		if (n < -126) {
			y *= 0x1p-126f * 0x1p24f;
			n += 126 - 24;
			if (n < -126)
				n = -126;
		}
	}
	u.i = (uint32_t)(0x7f+n)<<23;
	x = y * u.f;
	return x;
}

/* ------------------------------------------------------------------ */
/* s_fabsf.c							      */
/* ------------------------------------------------------------------ */

/* s_fabsf.c -- float version of s_fabs.c.
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
 * fabsf(x) returns the absolute value of x.
 */

float
ref_fabsf(float x)
{
	u_int32_t ix;
	GET_FLOAT_WORD(ix,x);
	SET_FLOAT_WORD(x,ix&0x7fffffff);
        return x;
}

/* ------------------------------------------------------------------ */
/* s_copysign.c							      */
/* ------------------------------------------------------------------ */

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
 * copysign(double x, double y)
 * copysign(x,y) returns a value with the magnitude of x and
 * with the sign bit of y.
 */

double
ref_copysign(double x, double y)
{
	u_int32_t hx,hy;
	GET_HIGH_WORD(hx,x);
	GET_HIGH_WORD(hy,y);
	SET_HIGH_WORD(x,(hx&0x7fffffff)|(hy&0x80000000));
        return x;
}

/* ------------------------------------------------------------------ */
/* e_gammaf.c							      */
/* ------------------------------------------------------------------ */

/* e_gammaf.c -- float version of e_gamma.c.
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

/* gammaf(x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call gammaf_r
 */

float
ref_gammaf(float x)
{
	return gammaf_r(x,&signgam);
}
