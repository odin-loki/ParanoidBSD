// PBSD batch b0276 -- C++23 module port of lib/msun/src.
//
// Faithful port of:
//	lib/msun/src/s_tan.c
//	lib/msun/src/s_tanh.c
//	lib/msun/src/s_nextafter.c
//
// Behaviour is preserved exactly, including integer signedness, evaluation
// order and every rounding-visible operation.  Nothing has been "improved".

module;

#include <bit>
#include <cmath>
#include <cfloat>
#include <cstdint>

export module pbsd.lib.msun.src.b0276;

namespace pbsd::lib_msun_src::b0276 {

extern "C" double __kernel_tan(double x, double y, int iy);
extern "C" int __ieee754_rem_pio2(double x, double *y);

namespace detail {

using u_int32_t = unsigned int;

inline void
get_high_word(int32_t &i, double d)
{
	i = static_cast<int32_t>(
	    static_cast<uint32_t>(std::bit_cast<uint64_t>(d) >> 32));
}

inline void
extract_words(int32_t &ix0, u_int32_t &ix1, double d)
{
	const uint64_t bits = std::bit_cast<uint64_t>(d);
	ix0 = static_cast<int32_t>(static_cast<uint32_t>(bits >> 32));
	ix1 = static_cast<u_int32_t>(static_cast<uint32_t>(bits));
}

inline void
insert_words(double &d, uint32_t ix0, uint32_t ix1)
{
	d = std::bit_cast<double>((static_cast<uint64_t>(ix0) << 32) |
	    static_cast<uint64_t>(ix1));
}

}  // namespace detail

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

/* tan(x)
 * Return tangent function of x.
 *
 * kernel function:
 *	__kernel_tan		... tangent function on [-pi/4,pi/4]
 *	__ieee754_rem_pio2	... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the sin, cos and tan respectively on
 *	[-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *	in [-pi/4 , +pi/4], and let n = k mod 4.
 *	We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *	    0	       S	   C		 T
 *	    1	       C	  -S		-1/T
 *	    2	      -S	  -C		 T
 *	    3	      -C	   S		-1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *	TRIG(x) returns trig(x) nearly rounded
 */

export double
tan(double x)
{
	double y[2], z = 0.0;
	int32_t n, ix;

	/* High word of x. */
	detail::get_high_word(ix, x);

	/* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if (ix <= 0x3fe921fb) {
		if (ix < 0x3e400000)			/* x < 2**-27 */
			if ((int)x == 0) return x;	/* generate inexact */
		return __kernel_tan(x, z, 1);
	}

	/* tan(Inf or NaN) is NaN */
	else if (ix >= 0x7ff00000) return x - x;	/* NaN */

	/* argument reduction needed */
	else {
		n = __ieee754_rem_pio2(x, y);
		return __kernel_tan(y[0], y[1], 1 - ((n & 1) << 1));
							/*   1 -- n even
							    -1 -- n odd */
	}
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

/* Tanh(x)
 * Return the Hyperbolic Tangent of x
 *
 * Method :
 *				       x    -x
 *				      e  - e
 *	0. tanh(x) is defined to be -----------
 *				       x    -x
 *				      e  + e
 *	1. reduce x to non-negative by tanh(-x) = -tanh(x).
 *	2.  0      <= x <  2**-28 : tanh(x) := x with inexact if x != 0
 *					        -t
 *	    2**-28 <= x <  1      : tanh(x) := -----; t = expm1(-2x)
 *					       t + 2
 *						     2
 *	    1      <= x <  22     : tanh(x) := 1 - -----; t = expm1(2x)
 *						   t + 2
 *	    22     <= x <= INF    : tanh(x) := 1.
 *
 * Special cases:
 *	tanh(NaN) is NaN;
 *	only tanh(0)=0 is exact for finite argument.
 */

static const volatile double tiny = 1.0e-300;
static const double one = 1.0, two = 2.0, huge = 1.0e300;

export double
tanh(double x)
{
	double t, z;
	int32_t jx, ix;

	detail::get_high_word(jx, x);
	ix = jx & 0x7fffffff;

	/* x is INF or NaN */
	if (ix >= 0x7ff00000) {
		if (jx >= 0) return one / x + one;  /* tanh(+-inf)=+-1 */
		else	     return one / x - one;  /* tanh(NaN) = NaN */
	}

	/* |x| < 22 */
	if (ix < 0x40360000) {		/* |x|<22 */
		if (ix < 0x3e300000) {	/* |x|<2**-28 */
			if (huge + x > one) return x;
					/* tanh(tiny) = tiny with inexact */
		}
		if (ix >= 0x3ff00000) {	/* |x|>=1  */
			t = ::expm1(two * ::fabs(x));
			z = one - two / (t + two);
		} else {
			t = ::expm1(-two * ::fabs(x));
			z = -t / (t + two);
		}
	/* |x| >= 22, return +-1 */
	} else {
		z = one - tiny;		/* raise inexact flag */
	}
	return (jx >= 0) ? z : -z;
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

/* IEEE functions
 *	nextafter(x,y)
 *	return the next machine floating-point number of x in the
 *	direction toward y.
 *   Special cases:
 */

export double
nextafter(double x, double y)
{
	volatile double t;
	int32_t hx, hy, ix, iy;
	detail::u_int32_t lx, ly;

	detail::extract_words(hx, lx, x);
	detail::extract_words(hy, ly, y);
	ix = hx & 0x7fffffff;		/* |x| */
	iy = hy & 0x7fffffff;		/* |y| */

	if (((ix >= 0x7ff00000) && ((ix - 0x7ff00000) | lx) != 0) ||
							    /* x is nan */
	    ((iy >= 0x7ff00000) && ((iy - 0x7ff00000) | ly) != 0))
							    /* y is nan */
		return x + y;
	if (x == y) return y;		/* x=y, return y */
	if ((ix | lx) == 0) {			/* x == 0 */
		detail::insert_words(x, hy & 0x80000000, 1);
						/* return +-minsubnormal */
		t = x * x;
		if (t == x) return t; else return x;
						/* raise underflow flag */
	}
	if (hx >= 0) {				/* x > 0 */
		if (hx > hy || ((hx == hy) && (lx > ly))) {
						/* x > y, x -= ulp */
			if (lx == 0) hx -= 1;
			lx -= 1;
		} else {			/* x < y, x += ulp */
			lx += 1;
			if (lx == 0) hx += 1;
		}
	} else {				/* x < 0 */
		if (hy >= 0 || hx > hy || ((hx == hy) && (lx > ly))) {
						/* x < y, x -= ulp */
			if (lx == 0) hx -= 1;
			lx -= 1;
		} else {			/* x > y, x += ulp */
			lx += 1;
			if (lx == 0) hx += 1;
		}
	}
	hy = hx & 0x7ff00000;
	if (hy >= 0x7ff00000) return x + x;	/* overflow  */
	if (hy < 0x00100000) {		/* underflow */
		t = x * x;
		if (t != x) {		/* raise underflow flag */
			detail::insert_words(y, hx, lx);
			return y;
		}
	}
	detail::insert_words(x, hx, lx);
	return x;
}

}  // namespace pbsd::lib_msun_src::b0276
