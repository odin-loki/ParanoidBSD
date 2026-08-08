/*
 * oracle.c -- reference implementation for PBSD batch b0242.
 *
 * The original HardenedBSD sources from lib/msun/src are concatenated below.
 * Every function has been renamed with a "ref_" prefix; the function bodies
 * are otherwise byte-for-byte unmodified.
 *
 * This file is the specification.  Do not modify any function body.
 */

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

#ifndef FP_ILOGB0
#define FP_ILOGB0 (-__INT_MAX)
#endif
#ifndef FP_ILOGBNAN
#define FP_ILOGBNAN __INT_MAX
#endif

/* from lib/msun/src/math_private.h */
typedef union
{
  float value;
  unsigned int word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i,d)					\
do {								\
  ieee_float_shape_type gf_u;					\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

typedef union
{
  double value;
  struct
  {
#if __BYTE_ORDER == __BIG_ENDIAN
    u_int32_t msw;
    u_int32_t lsw;
#else
    u_int32_t lsw;
    u_int32_t msw;
#endif
  } parts;
} ieee_double_shape_type;

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  ieee_double_shape_type ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

static inline int32_t
subnormal_ilogb(int32_t hi, int32_t lo)
{
	int32_t j;
	uint32_t i;

	j = -1022;
	if (hi == 0) {
	    j -= 21;
	    i = (uint32_t)lo;
	} else
	    i = (uint32_t)hi << 11;

	for (; i < 0x7fffffff; i <<= 1) j -= 1;

	return (j);
}

static inline int32_t
subnormal_ilogbf(int32_t hx)
{
	int32_t j;
	uint32_t i;
	i = (uint32_t) hx << 8;
	for (j = -126; i < 0x7fffffff; i <<= 1) j -=1;

	return (j);
}

/* ------------------------------------------------------------------ */
/* s_ilogbf.c                                                         */
/* ------------------------------------------------------------------ */

/* s_ilogbf.c -- float version of s_ilogb.c.
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

int
ref_ilogbf(float x)
{
	int32_t hx,ix;

	GET_FLOAT_WORD(hx,x);
	hx &= 0x7fffffff;
	if(hx<0x00800000) {
	    if(hx==0)
		return FP_ILOGB0;
	    else			/* subnormal x */
		ix = subnormal_ilogbf(hx);
	    return ix;
	}
	else if (hx<0x7f800000) return (hx>>23)-127;
	else if (hx>0x7f800000) return FP_ILOGBNAN;
	else return INT_MAX;
}

/* ------------------------------------------------------------------ */
/* s_ilogb.c                                                          */
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

/* ilogb(double x)
 * return the binary exponent of non-zero x
 * ilogb(0) = FP_ILOGB0
 * ilogb(NaN) = FP_ILOGBNAN (no signal is raised)
 * ilogb(inf) = INT_MAX (no signal is raised)
 */

int
ref_ilogb(double x)
{
	int32_t hx, ix, lx;

	EXTRACT_WORDS(hx,lx,x);
	hx &= 0x7fffffff;
	if(hx<0x00100000) {
	    if((hx|lx)==0)
		return FP_ILOGB0;
	    else
		ix = subnormal_ilogb(hx, lx);
	    return ix;
	}
	else if (hx<0x7ff00000) return (hx>>20)-1023;
	else if (hx>0x7ff00000 || lx!=0) return FP_ILOGBNAN;
	else return INT_MAX;
}
