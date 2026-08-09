/*
 * oracle.c -- reference implementation for PBSD batch b0237.
 *
 * The original HardenedBSD sources from lib/msun/src are concatenated below.
 * Every function has been renamed with a "ref_" prefix; the function bodies
 * are otherwise byte-for-byte unmodified.
 *
 * This file is the specification.  Do not modify any function body.
 */

#include <float.h>
#include <math.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

/* ------------------------------------------------------------------ */
/* s_significandf.c						      */
/* ------------------------------------------------------------------ */

/* s_significandf.c -- float version of s_significand.c.
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

float
ref_significandf(float x)
{
	return scalbf(x,(float) -ilogbf(x));
}

/* ------------------------------------------------------------------ */
/* s_significand.c						      */
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
 * significand(x) computes just
 * 	scalb(x, (double) -ilogb(x)),
 * for exercising the fraction-part(F) IEEE 754-1985 test vector.
 */

double
ref_significand(double x)
{
	return scalb(x,(double) -ilogb(x));
}

/* ------------------------------------------------------------------ */
/* e_gamma_r.c							      */
/* ------------------------------------------------------------------ */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

/* gamma_r(x, signgamp)
 * Reentrant version of the logarithm of the Gamma function 
 * with user provide pointer for the sign of Gamma(x). 
 *
 * Method: See lgamma_r
 */

double
ref_gamma_r(double x, int *signgamp)
{
	return lgamma_r(x,signgamp);
}

/* ------------------------------------------------------------------ */
/* e_lgamma.c							      */
/* ------------------------------------------------------------------ */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

/* lgamma(x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call lgamma_r
 */

extern int signgam;

double
ref_lgamma(double x)
{
	return lgamma_r(x,&signgam);
}
