module;

extern "C" {
#define complex _Complex
#include <complex.h>
#include <float.h>
#include <math.h>
}

#ifndef __weak_reference
#define __weak_reference(sym, alias)
#endif

export module pbsd.lib.msun.src.b0236;

export namespace pbsd::lib_msun_src::b0236 {

/*
 * dremf() wrapper for remainderf().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

float
dremf(float x, float y)
{
	return (::remainderf(x, y));
}

/*
 * cabsf() wrapper for hypotf().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

float
cabsf(float complex z)
{

	return (::hypotf(::crealf(z), ::cimagf(z)));
}

/*
 * cabs() wrapper for hypot().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 *
 * Modified by Steven G. Kargl for the long double type.
 */

long double
cabsl(long double complex z)
{
	return (::hypotl(::creall(z), ::cimagl(z)));
}

/*
 * cabs() wrapper for hypot().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

double
cabs(double complex z)
{
	return (::hypot(::creal(z), ::cimag(z)));
}

#if LDBL_MANT_DIG == 53
__weak_reference(cabs, cabsl);
#endif

} // namespace pbsd::lib_msun_src::b0236
