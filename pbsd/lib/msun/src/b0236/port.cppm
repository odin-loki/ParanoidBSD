module;

#include <float.h>
#include <math.h>

#ifndef __weak_reference
#define __weak_reference(sym, alias)
#endif

export module pbsd.lib.msun.src.b0236;

export namespace pbsd::lib_msun_src::b0236 {

typedef __complex__ float float_complex_t;
typedef __complex__ double double_complex_t;
typedef __complex__ long double long_double_complex_t;

extern "C" float crealf(float_complex_t);
extern "C" float cimagf(float_complex_t);
extern "C" double creal(double_complex_t);
extern "C" double cimag(double_complex_t);
extern "C" long double creall(long_double_complex_t);
extern "C" long double cimagl(long_double_complex_t);

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
cabsf(float_complex_t z)
{

	return (::hypotf(crealf(z), cimagf(z)));
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
cabsl(long_double_complex_t z)
{
	return (::hypotl(creall(z), cimagl(z)));
}

/*
 * cabs() wrapper for hypot().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

double
cabs(double_complex_t z)
{
	return (::hypot(creal(z), cimag(z)));
}

#if LDBL_MANT_DIG == 53
__weak_reference(cabs, cabsl);
#endif

} // namespace pbsd::lib_msun_src::b0236
