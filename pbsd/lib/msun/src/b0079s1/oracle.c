/*
 * oracle.c -- reference implementation for PBSD batch b0079s1.
 *
 * The original HardenedBSD source from lib/msun/src/e_lgammaf.c is reproduced
 * below.  The function has been renamed with a "ref_" prefix; the body is
 * otherwise unmodified.
 */

#include <math.h>

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
