/*
 * oracle.c -- reference implementation for PBSD batch b0079s3.
 *
 * The original HardenedBSD source from lib/msun/src/e_gammaf_r.c is reproduced
 * below.  The function has been renamed with a "ref_" prefix; the function
 * body is otherwise byte-for-byte unmodified.
 *
 * This file is the specification.  Do not modify any function body.
 */

#include <math.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

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
