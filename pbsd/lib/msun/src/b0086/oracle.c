/*
 * Reference oracle for PBSD batch b0086.
 *
 * The original HardenedBSD lib/msun/src sources, concatenated, with every
 * function renamed with a "ref_" prefix.  Function bodies are unmodified.
 *
 * The only additions are the feature-test macro required to expose
 * lgammal_r()/signgam on this host, and the word-access macros that
 * lib/msun/src/math_private.h would otherwise supply.
 */

#define _GNU_SOURCE

#include <endian.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>

/* from lib/msun/src/math_private.h */
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

#define GET_HIGH_WORD(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)

#define SET_HIGH_WORD(d,v)					\
do {								\
  ieee_double_shape_type sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)

/* ------------------------------------------------------------------ */
/* lib/msun/src/e_lgammal.c                                           */
/* ------------------------------------------------------------------ */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

extern int signgam;

long double
ref_lgammal(long double x)
{
	return lgammal_r(x,&signgam);
}

/* ------------------------------------------------------------------ */
/* lib/msun/src/s_fabs.c                                              */
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

/*
 * fabs(x) returns the absolute value of x.
 */

double
ref_fabs(double x)
{
	u_int32_t high;
	GET_HIGH_WORD(high,x);
	SET_HIGH_WORD(x,high&0x7fffffff);
        return x;
}

/* ------------------------------------------------------------------ */
/* lib/msun/src/s_finite.c                                            */
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

/*
 * finite(x) returns 1 is x is finite, else 0;
 * no branching!
 */

	int ref_finite(double x)
{
	int32_t hx;
	GET_HIGH_WORD(hx,x);
	return (int)((u_int32_t)((hx&0x7fffffff)-0x7ff00000)>>31);
}
