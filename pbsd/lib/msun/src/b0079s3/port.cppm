// PBSD port of HardenedBSD lib/msun/src batch b0079s3.
//
// Sources ported in this module:
//   e_gammaf_r.c   -> gammaf_r

module;

#include <cmath>

export module pbsd.lib.msun.src.b0079s3;

export namespace pbsd::lib_msun_src::b0079s3 {

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
gammaf_r(float x, int *signgamp)
{
	return ::lgammaf_r(x,signgamp);
}

} // namespace pbsd::lib_msun_src::b0079s3
