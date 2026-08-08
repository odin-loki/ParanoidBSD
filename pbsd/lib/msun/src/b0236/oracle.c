/*
 * Oracle for batch b0236: the original HardenedBSD sources, concatenated,
 * with every function renamed with a ref_ prefix.  Function bodies are
 * unmodified.
 */

#include <complex.h>
#include <float.h>
#include <math.h>

#ifndef __weak_reference
#define __weak_reference(sym, alias)
#endif
#ifndef ref___weak_reference
#define ref___weak_reference(sym, alias) __weak_reference(sym, alias)
#endif

/* ---------------------------------------------------------------------- */
/* hbsd/src/lib/msun/src/w_dremf.c                                         */
/* ---------------------------------------------------------------------- */

/*
 * dremf() wrapper for remainderf().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

float
ref_dremf(float x, float y)
{
	return remainderf(x, y);
}

/* ---------------------------------------------------------------------- */
/* hbsd/src/lib/msun/src/w_cabsf.c                                         */
/* ---------------------------------------------------------------------- */

/*
 * cabsf() wrapper for hypotf().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

float
ref_cabsf(float complex z)
{

	return hypotf(crealf(z), cimagf(z));
}

/* ---------------------------------------------------------------------- */
/* hbsd/src/lib/msun/src/w_cabsl.c                                         */
/* ---------------------------------------------------------------------- */

/*
 * cabs() wrapper for hypot().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 *
 * Modified by Steven G. Kargl for the long double type.
 */

long double
ref_cabsl(long double complex z)
{
	return hypotl(creall(z), cimagl(z));
}

/* ---------------------------------------------------------------------- */
/* hbsd/src/lib/msun/src/w_cabs.c                                          */
/* ---------------------------------------------------------------------- */

/*
 * cabs() wrapper for hypot().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

double
ref_cabs(double complex z)
{
	return hypot(creal(z), cimag(z));
}

#if LDBL_MANT_DIG == 53
ref___weak_reference(ref_cabs, cabsl);
#endif
