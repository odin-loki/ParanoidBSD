// PBSD port of HardenedBSD lib/msun/src batch b0276.
//
// Sources ported in this module:
//   s_tan.c         -> tan
//   s_tanh.c        -> tanh
//   s_nextafter.c   -> nextafter
//
// Original copyright headers are reproduced verbatim above each ported
// function.  __kernel_tan, __ieee754_rem_pio2, and __kernel_rem_pio2 are
// linked from the original msun objects built by build.sh.

module;

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <endian.h>
#include <sys/types.h>

export module pbsd.lib.msun.src.b0276;

namespace pbsd::lib_msun_src::b0276 {

typedef union {
	double value;
	struct {
#if __BYTE_ORDER == __BIG_ENDIAN
		std::uint32_t msw;
		std::uint32_t lsw;
#else
		std::uint32_t lsw;
		std::uint32_t msw;
#endif
	} parts;
} ieee_double_shape_type;

#define GET_HIGH_WORD(i,d)				\
do {							\
	ieee_double_shape_type gh_u;			\
	gh_u.value = (d);				\
	(i) = gh_u.parts.msw;				\
} while (0)

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {							\
	ieee_double_shape_type ew_u;			\
	ew_u.value = (d);				\
	(ix0) = ew_u.parts.msw;				\
	(ix1) = ew_u.parts.lsw;				\
} while (0)

#define INSERT_WORDS(d,ix0,ix1)				\
do {							\
	ieee_double_shape_type iw_u;			\
	iw_u.parts.msw = (ix0);				\
	iw_u.parts.lsw = (ix1);				\
	(d) = iw_u.value;				\
} while (0)

extern "C" double __kernel_tan(double x, double y, int iy);
extern "C" int __ieee754_rem_pio2(double x, double *y);

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

export double
tan(double x)
{
	double y[2],z=0.0;
	int32_t n, ix;

    /* High word of x. */
	GET_HIGH_WORD(ix,x);

    /* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if(ix <= 0x3fe921fb) {
	    if(ix<0x3e400000)			/* x < 2**-27 */
		if((int)x==0) return x;		/* generate inexact */
	    return __kernel_tan(x,z,1);
	}

    /* tan(Inf or NaN) is NaN */
	else if (ix>=0x7ff00000) return x-x;		/* NaN */

    /* argument reduction needed */
	else {
	    n = __ieee754_rem_pio2(x,y);
	    return __kernel_tan(y[0],y[1],1-((n&1)<<1)); /*   1 -- n even
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

namespace tanh_statics {

static const volatile double tiny = 1.0e-300;
static const double one = 1.0, two = 2.0, huge = 1.0e300;

} // namespace tanh_statics

#define tiny tanh_statics::tiny
#define one tanh_statics::one
#define two tanh_statics::two
#define huge tanh_statics::huge

export double
tanh(double x)
{
	double t,z;
	int32_t jx,ix;

	GET_HIGH_WORD(jx,x);
	ix = jx&0x7fffffff;

    /* x is INF or NaN */
	if(ix>=0x7ff00000) {
	    if (jx>=0) return one/x+one;    /* tanh(+-inf)=+-1 */
	    else       return one/x-one;    /* tanh(NaN) = NaN */
	}

    /* |x| < 22 */
	if (ix < 0x40360000) {		/* |x|<22 */
	    if (ix<0x3e300000) {	/* |x|<2**-28 */
		if(huge+x>one) return x; /* tanh(tiny) = tiny with inexact */
	    }
	    if (ix>=0x3ff00000) {	/* |x|>=1  */
		t = ::expm1(two*::fabs(x));
		z = one - two/(t+two);
	    } else {
	        t = ::expm1(-two*::fabs(x));
	        z= -t/(t+two);
	    }
    /* |x| >= 22, return +-1 */
	} else {
	    z = one - tiny;		/* raise inexact flag */
	}
	return (jx>=0)? z: -z;
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

export double
nextafter(double x, double y)
{
	volatile double t;
	int32_t hx,hy,ix,iy;
	u_int32_t lx,ly;

	EXTRACT_WORDS(hx,lx,x);
	EXTRACT_WORDS(hy,ly,y);
	ix = hx&0x7fffffff;		/* |x| */
	iy = hy&0x7fffffff;		/* |y| */

	if(((ix>=0x7ff00000)&&((ix-0x7ff00000)|lx)!=0) ||   /* x is nan */
	   ((iy>=0x7ff00000)&&((iy-0x7ff00000)|ly)!=0))     /* y is nan */
	   return x+y;
	if(x==y) return y;		/* x=y, return y */
	if((ix|lx)==0) {			/* x == 0 */
	    INSERT_WORDS(x,hy&0x80000000,1);	/* return +-minsubnormal */
	    t = x*x;
	    if(t==x) return t; else return x;	/* raise underflow flag */
	}
	if(hx>=0) {				/* x > 0 */
	    if(hx>hy||((hx==hy)&&(lx>ly))) {	/* x > y, x -= ulp */
		if(lx==0) hx -= 1;
		lx -= 1;
	    } else {				/* x < y, x += ulp */
		lx += 1;
		if(lx==0) hx += 1;
	    }
	} else {				/* x < 0 */
	    if(hy>=0||hx>hy||((hx==hy)&&(lx>ly))){/* x < y, x -= ulp */
		if(lx==0) hx -= 1;
		lx -= 1;
	    } else {				/* x > y, x += ulp */
		lx += 1;
		if(lx==0) hx += 1;
	    }
	}
	hy = hx&0x7ff00000;
	if(hy>=0x7ff00000) return x+x;	/* overflow  */
	if(hy<0x00100000) {		/* underflow */
	    t = x*x;
	    if(t!=x) {		/* raise underflow flag */
	        INSERT_WORDS(y,hx,lx);
		return y;
	    }
	}
	INSERT_WORDS(x,hx,lx);
	return x;
}

} // namespace pbsd::lib_msun_src::b0276
