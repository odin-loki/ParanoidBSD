// PBSD port of HardenedBSD lib/msun/src batch b0237.
//
// Sources ported in this module:
//   s_significandf.c -> significandf
//   s_significand.c  -> significand
//   e_gamma_r.c      -> gamma_r
//   e_lgamma.c       -> lgamma
//
// Original copyright headers are reproduced verbatim above each ported
// function.  The ports are literal transcriptions.

module;

#include <cmath>

export module pbsd.lib.msun.src.b0237;

export namespace pbsd::lib_msun_src::b0237 {

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
significandf(float x)
{
	return ::scalbf(x,(float) -::ilogbf(x));
}

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
significand(double x)
{
	return ::scalb(x,(double) -::ilogb(x));
}

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
gamma_r(double x, int *signgamp)
{
	return ::lgamma_r(x,signgamp);
}

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

double
lgamma(double x)
{
	return ::lgamma_r(x,&::signgam);
}

} // namespace pbsd::lib_msun_src::b0237
