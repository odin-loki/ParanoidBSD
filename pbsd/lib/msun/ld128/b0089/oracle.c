/*
 * Reference oracle for PBSD batch b0089.
 *
 * Original HardenedBSD lib/msun/ld128 sources concatenated; every external
 * function renamed with a ref_ prefix.  Bodies are otherwise unmodified.
 * Per-section macros rename file-local statics so the concatenated unit links.
 */

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef __always_inline
#define __always_inline __inline__
#endif

union IEEEl2bits {
	long double e;
	struct {
		unsigned long manl;
		unsigned long manh;
		unsigned int exp;
		unsigned int sign;
	} bits;
	struct {
		unsigned long manl;
		unsigned long manh;
		unsigned int expsign;
	} xbits;
};

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
typedef union {
	long double value;
	struct {
		u_int32_t mswhi;
		u_int32_t mswlo;
		u_int32_t lswhi;
		u_int32_t lswlo;
	} parts32;
} ieee_quad_shape_type;
#else
typedef union {
	long double value;
	struct {
		u_int32_t lswlo;
		u_int32_t lswhi;
		u_int32_t mswlo;
		u_int32_t mswhi;
	} parts32;
} ieee_quad_shape_type;
#endif

#define EXTRACT_LDBL128_WORDS(ix0, ix1, ix2, d) do { \
	union IEEEl2bits ew_u; \
	ew_u.e = (d); \
	(ix0) = ew_u.xbits.expsign; \
	(ix1) = ew_u.xbits.manh; \
	(ix2) = ew_u.xbits.manl; \
} while (0)

#define INSERT_LDBL128_WORDS(d, ix0, ix1, ix2) do { \
	union IEEEl2bits iw_u; \
	iw_u.xbits.expsign = (ix0); \
	iw_u.xbits.manh = (ix1); \
	iw_u.xbits.manl = (ix2); \
	(d) = iw_u.e; \
} while (0)

#define SET_LDBL_EXPSIGN(d, v) do { \
	union IEEEl2bits se_u; \
	se_u.e = (d); \
	se_u.xbits.expsign = (v); \
	(d) = se_u.e; \
} while (0)

#define ENTERI()
#define RETURNF(v) return (v)
#define RETURNI(x) return (x)
#define nan_mix(x, y) (((x) + 0.0L) + (y))

#define _2sumF(a, b) do { \
	__typeof__(a) __w; \
	__w = (a) + (b); \
	(b) = ((a) - __w) + (b); \
	(a) = __w; \
} while (0)

#define _3sumF(a, b, c) do { \
	__typeof__(a) __tmp; \
	__tmp = (c); \
	_2sumF(__tmp, (a)); \
	(b) += (a); \
	(a) = __tmp; \
} while (0)

#define STRUCT_RETURN
#define RETURN1(rp, v) do { \
	(rp)->hi = (v); \
	(rp)->lo_set = 0; \
	return; \
} while (0)

#define RETURN2(rp, h, l) do { \
	(rp)->hi = (h); \
	(rp)->lo = (l); \
	(rp)->lo_set = 1; \
	return; \
} while (0)

#define RETURNSPI(rp) do { \
	if (!(rp)->lo_set) \
		RETURNI((rp)->hi); \
	RETURNI((rp)->hi + (rp)->lo); \
} while (0)

#define NO_UTAB
#define NO_UTABL

/* ===== e_powl.c ===== */
#undef G
#undef F_hi
#undef F_lo
#undef ln2_hi
#undef ln2_lo
#undef E
#undef H
#define bp __b0089_powl_bp
#define dp_h __b0089_powl_dp_h
#define dp_l __b0089_powl_dp_l
#define zero __b0089_powl_zero
#define one __b0089_powl_one
#define two __b0089_powl_two
#define two113 __b0089_powl_two113
#define huge __b0089_powl_huge
#define tiny __b0089_powl_tiny
#define LN __b0089_powl_LN
#define LD __b0089_powl_LD
#define PN __b0089_powl_PN
#define PD __b0089_powl_PD
#define lg2 __b0089_powl_lg2
#define lg2_h __b0089_powl_lg2_h
#define lg2_l __b0089_powl_lg2_l
#define ovt __b0089_powl_ovt
#define cp __b0089_powl_cp
#define cp_h __b0089_powl_cp_h
#define cp_l __b0089_powl_cp_l
/*-
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
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* powl(x,y) return x**y
 *
 *		      n
 * Method:  Let x =  2   * (1+f)
 *	1. Compute and return log2(x) in two pieces:
 *		log2(x) = w1 + w2,
 *	   where w1 has 113-53 = 60 bit trailing zeros.
 *	2. Perform y*log2(x) = n+y' by simulating multi-precision
 *	   arithmetic, where |y'|<=0.5.
 *	3. Return x**y = 2**n*exp(y'*log2)
 *
 * Special cases:
 *	1.  (anything) ** 0  is 1
 *	2.  (anything) ** 1  is itself
 *	3.  (anything) ** NAN is NAN
 *	4.  NAN ** (anything except 0) is NAN
 *	5.  +-(|x| > 1) **  +INF is +INF
 *	6.  +-(|x| > 1) **  -INF is +0
 *	7.  +-(|x| < 1) **  +INF is +0
 *	8.  +-(|x| < 1) **  -INF is +INF
 *	9.  +-1         ** +-INF is NAN
 *	10. +0 ** (+anything except 0, NAN)               is +0
 *	11. -0 ** (+anything except 0, NAN, odd integer)  is +0
 *	12. +0 ** (-anything except 0, NAN)               is +INF
 *	13. -0 ** (-anything except 0, NAN, odd integer)  is +INF
 *	14. -0 ** (odd integer) = -( +0 ** (odd integer) )
 *	15. +INF ** (+anything except 0,NAN) is +INF
 *	16. +INF ** (-anything except 0,NAN) is +0
 *	17. -INF ** (anything)  = -0 ** (-anything)
 *	18. (-anything) ** (integer) is (-1)**(integer)*(+anything**integer)
 *	19. (-anything except 0 and inf) ** (non-integer) is NAN
 *
 */

static const long double bp[] = {
  1.0L,
  1.5L,
};

/* log_2(1.5) */
static const long double dp_h[] = {
  0.0,
  5.8496250072115607565592654282227158546448E-1L
};

/* Low part of log_2(1.5) */
static const long double dp_l[] = {
  0.0,
  1.0579781240112554492329533686862998106046E-16L
};

static const long double zero = 0.0L,
  one = 1.0L,
  two = 2.0L,
  two113 = 1.0384593717069655257060992658440192E34L,
  huge = 1.0e3000L,
  tiny = 1.0e-3000L;

/* 3/2 log x = 3 z + z^3 + z^3 (z^2 R(z^2))
   z = (x-1)/(x+1)
   1 <= x <= 1.25
   Peak relative error 2.3e-37 */
static const long double LN[] =
{
 -3.0779177200290054398792536829702930623200E1L,
  6.5135778082209159921251824580292116201640E1L,
 -4.6312921812152436921591152809994014413540E1L,
  1.2510208195629420304615674658258363295208E1L,
 -9.9266909031921425609179910128531667336670E-1L
};
static const long double LD[] =
{
 -5.129862866715009066465422805058933131960E1L,
  1.452015077564081884387441590064272782044E2L,
 -1.524043275549860505277434040464085593165E2L,
  7.236063513651544224319663428634139768808E1L,
 -1.494198912340228235853027849917095580053E1L
  /* 1.0E0 */
};

/* exp(x) = 1 + x - x / (1 - 2 / (x - x^2 R(x^2)))
   0 <= x <= 0.5
   Peak relative error 5.7e-38  */
static const long double PN[] =
{
  5.081801691915377692446852383385968225675E8L,
  9.360895299872484512023336636427675327355E6L,
  4.213701282274196030811629773097579432957E4L,
  5.201006511142748908655720086041570288182E1L,
  9.088368420359444263703202925095675982530E-3L,
};
static const long double PD[] =
{
  3.049081015149226615468111430031590411682E9L,
  1.069833887183886839966085436512368982758E8L,
  8.259257717868875207333991924545445705394E5L,
  1.872583833284143212651746812884298360922E3L,
  /* 1.0E0 */
};

static const long double
  /* ln 2 */
  lg2 = 6.9314718055994530941723212145817656807550E-1L,
  lg2_h = 6.9314718055994528622676398299518041312695E-1L,
  lg2_l = 2.3190468138462996154948554638754786504121E-17L,
  ovt = 8.0085662595372944372e-0017L,
  /* 2/(3*log(2)) */
  cp = 9.6179669392597560490661645400126142495110E-1L,
  cp_h = 9.6179669392597555432899980587535537779331E-1L,
  cp_l = 5.0577616648125906047157785230014751039424E-17L;

long double
ref_powl(long double x, long double y)
{
  long double z, ax, z_h, z_l, p_h, p_l;
  long double yy1, t1, t2, r, s, t, u, v, w;
  long double s2, s_h, s_l, t_h, t_l;
  int32_t i, j, k, yisint, n;
  u_int32_t ix, iy;
  int32_t hx, hy;
  ieee_quad_shape_type o, p, q;

  p.value = x;
  hx = p.parts32.mswhi;
  ix = hx & 0x7fffffff;

  q.value = y;
  hy = q.parts32.mswhi;
  iy = hy & 0x7fffffff;


  /* y==zero: x**0 = 1 */
  if ((iy | q.parts32.mswlo | q.parts32.lswhi | q.parts32.lswlo) == 0)
    return one;

  /* 1.0**y = 1; -1.0**+-Inf = 1 */
  if (x == one)
    return one;
  if (x == -1.0L && iy == 0x7fff0000
      && (q.parts32.mswlo | q.parts32.lswhi | q.parts32.lswlo) == 0)
    return one;

  /* +-NaN return x+y */
  if ((ix > 0x7fff0000)
      || ((ix == 0x7fff0000)
	  && ((p.parts32.mswlo | p.parts32.lswhi | p.parts32.lswlo) != 0))
      || (iy > 0x7fff0000)
      || ((iy == 0x7fff0000)
	  && ((q.parts32.mswlo | q.parts32.lswhi | q.parts32.lswlo) != 0)))
    return nan_mix(x, y);

  /* determine if y is an odd int when x < 0
   * yisint = 0       ... y is not an integer
   * yisint = 1       ... y is an odd int
   * yisint = 2       ... y is an even int
   */
  yisint = 0;
  if (hx < 0)
    {
      if (iy >= 0x40700000)	/* 2^113 */
	yisint = 2;		/* even integer y */
      else if (iy >= 0x3fff0000)	/* 1.0 */
	{
	  if (floorl (y) == y)
	    {
	      z = 0.5 * y;
	      if (floorl (z) == z)
		yisint = 2;
	      else
		yisint = 1;
	    }
	}
    }

  /* special value of y */
  if ((q.parts32.mswlo | q.parts32.lswhi | q.parts32.lswlo) == 0)
    {
      if (iy == 0x7fff0000)	/* y is +-inf */
	{
	  if (((ix - 0x3fff0000) | p.parts32.mswlo | p.parts32.lswhi |
	    p.parts32.lswlo) == 0)
	    return y - y;	/* +-1**inf is NaN */
	  else if (ix >= 0x3fff0000)	/* (|x|>1)**+-inf = inf,0 */
	    return (hy >= 0) ? y : zero;
	  else			/* (|x|<1)**-,+inf = inf,0 */
	    return (hy < 0) ? -y : zero;
	}
      if (iy == 0x3fff0000)
	{			/* y is  +-1 */
	  if (hy < 0)
	    return one / x;
	  else
	    return x;
	}
      if (hy == 0x40000000)
	return x * x;		/* y is  2 */
      if (hy == 0x3ffe0000)
	{			/* y is  0.5 */
	  if (hx >= 0)		/* x >= +0 */
	    return sqrtl (x);
	}
    }

  ax = fabsl (x);
  /* special value of x */
  if ((p.parts32.mswlo | p.parts32.lswhi | p.parts32.lswlo) == 0)
    {
      if (ix == 0x7fff0000 || ix == 0 || ix == 0x3fff0000)
	{
	  z = ax;		/*x is +-0,+-inf,+-1 */
	  if (hy < 0)
	    z = one / z;	/* z = (1/|x|) */
	  if (hx < 0)
	    {
	      if (((ix - 0x3fff0000) | yisint) == 0)
		{
		  z = (z - z) / (z - z);	/* (-1)**non-int is NaN */
		}
	      else if (yisint == 1)
		z = -z;		/* (x<0)**odd = -(|x|**odd) */
	    }
	  return z;
	}
    }

  /* (x<0)**(non-int) is NaN */
  if (((((u_int32_t) hx >> 31) - 1) | yisint) == 0)
    return (x - x) / (x - x);

  /* |y| is huge.
     2^-16495 = 1/2 of smallest representable value.
     If (1 - 1/131072)^y underflows, y > 1.4986e9 */
  if (iy > 0x401d654b)
    {
      /* if (1 - 2^-113)^y underflows, y > 1.1873e38 */
      if (iy > 0x407d654b)
	{
	  if (ix <= 0x3ffeffff)
	    return (hy < 0) ? huge * huge : tiny * tiny;
	  if (ix >= 0x3fff0000)
	    return (hy > 0) ? huge * huge : tiny * tiny;
	}
      /* over/underflow if x is not close to one */
      if (ix < 0x3ffeffff)
	return (hy < 0) ? huge * huge : tiny * tiny;
      if (ix > 0x3fff0000)
	return (hy > 0) ? huge * huge : tiny * tiny;
    }

  n = 0;
  /* take care subnormal number */
  if (ix < 0x00010000)
    {
      ax *= two113;
      n -= 113;
      o.value = ax;
      ix = o.parts32.mswhi;
    }
  n += ((ix) >> 16) - 0x3fff;
  j = ix & 0x0000ffff;
  /* determine interval */
  ix = j | 0x3fff0000;		/* normalize ix */
  if (j <= 0x3988)
    k = 0;			/* |x|<sqrt(3/2) */
  else if (j < 0xbb67)
    k = 1;			/* |x|<sqrt(3)   */
  else
    {
      k = 0;
      n += 1;
      ix -= 0x00010000;
    }

  o.value = ax;
  o.parts32.mswhi = ix;
  ax = o.value;

  /* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
  u = ax - bp[k];		/* bp[0]=1.0, bp[1]=1.5 */
  v = one / (ax + bp[k]);
  s = u * v;
  s_h = s;

  o.value = s_h;
  o.parts32.lswlo = 0;
  o.parts32.lswhi &= 0xf8000000;
  s_h = o.value;
  /* t_h=ax+bp[k] High */
  t_h = ax + bp[k];
  o.value = t_h;
  o.parts32.lswlo = 0;
  o.parts32.lswhi &= 0xf8000000;
  t_h = o.value;
  t_l = ax - (t_h - bp[k]);
  s_l = v * ((u - s_h * t_h) - s_h * t_l);
  /* compute log(ax) */
  s2 = s * s;
  u = LN[0] + s2 * (LN[1] + s2 * (LN[2] + s2 * (LN[3] + s2 * LN[4])));
  v = LD[0] + s2 * (LD[1] + s2 * (LD[2] + s2 * (LD[3] + s2 * (LD[4] + s2))));
  r = s2 * s2 * u / v;
  r += s_l * (s_h + s);
  s2 = s_h * s_h;
  t_h = 3.0 + s2 + r;
  o.value = t_h;
  o.parts32.lswlo = 0;
  o.parts32.lswhi &= 0xf8000000;
  t_h = o.value;
  t_l = r - ((t_h - 3.0) - s2);
  /* u+v = s*(1+...) */
  u = s_h * t_h;
  v = s_l * t_h + t_l * s;
  /* 2/(3log2)*(s+...) */
  p_h = u + v;
  o.value = p_h;
  o.parts32.lswlo = 0;
  o.parts32.lswhi &= 0xf8000000;
  p_h = o.value;
  p_l = v - (p_h - u);
  z_h = cp_h * p_h;		/* cp_h+cp_l = 2/(3*log2) */
  z_l = cp_l * p_h + p_l * cp + dp_l[k];
  /* log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l */
  t = (long double) n;
  t1 = (((z_h + z_l) + dp_h[k]) + t);
  o.value = t1;
  o.parts32.lswlo = 0;
  o.parts32.lswhi &= 0xf8000000;
  t1 = o.value;
  t2 = z_l - (((t1 - t) - dp_h[k]) - z_h);

  /* s (sign of result -ve**odd) = -1 else = 1 */
  s = one;
  if (((((u_int32_t) hx >> 31) - 1) | (yisint - 1)) == 0)
    s = -one;			/* (-ve)**(odd int) */

  /* split up y into yy1+y2 and compute (yy1+y2)*(t1+t2) */
  yy1 = y;
  o.value = yy1;
  o.parts32.lswlo = 0;
  o.parts32.lswhi &= 0xf8000000;
  yy1 = o.value;
  p_l = (y - yy1) * t1 + y * t2;
  p_h = yy1 * t1;
  z = p_l + p_h;
  o.value = z;
  j = o.parts32.mswhi;
  if (j >= 0x400d0000) /* z >= 16384 */
    {
      /* if z > 16384 */
      if (((j - 0x400d0000) | o.parts32.mswlo | o.parts32.lswhi |
	o.parts32.lswlo) != 0)
	return s * huge * huge;	/* overflow */
      else
	{
	  if (p_l + ovt > z - p_h)
	    return s * huge * huge;	/* overflow */
	}
    }
  else if ((j & 0x7fffffff) >= 0x400d01b9)	/* z <= -16495 */
    {
      /* z < -16495 */
      if (((j - 0xc00d01bc) | o.parts32.mswlo | o.parts32.lswhi |
	o.parts32.lswlo)
	  != 0)
	return s * tiny * tiny;	/* underflow */
      else
	{
	  if (p_l <= z - p_h)
	    return s * tiny * tiny;	/* underflow */
	}
    }
  /* compute 2**(p_h+p_l) */
  i = j & 0x7fffffff;
  k = (i >> 16) - 0x3fff;
  n = 0;
  if (i > 0x3ffe0000)
    {				/* if |z| > 0.5, set n = [z+0.5] */
      n = floorl (z + 0.5L);
      t = n;
      p_h -= t;
    }
  t = p_l + p_h;
  o.value = t;
  o.parts32.lswlo = 0;
  o.parts32.lswhi &= 0xf8000000;
  t = o.value;
  u = t * lg2_h;
  v = (p_l - (t - p_h)) * lg2 + t * lg2_l;
  z = u + v;
  w = v - (z - u);
  /*  exp(z) */
  t = z * z;
  u = PN[0] + t * (PN[1] + t * (PN[2] + t * (PN[3] + t * PN[4])));
  v = PD[0] + t * (PD[1] + t * (PD[2] + t * (PD[3] + t)));
  t1 = z - t * u / v;
  r = (z * t1) / (t1 - two) - (w + z * w);
  z = one - (r - z);
  o.value = z;
  j = o.parts32.mswhi;
  j += (n << 16);
  if ((j >> 16) <= 0)
    z = scalbnl (z, n);	/* subnormal output */
  else
    {
      o.parts32.mswhi = j;
      z = o.value;
    }
  return s * z;
}

#undef bp
#undef dp_h
#undef dp_l
#undef zero
#undef one
#undef two
#undef two113
#undef huge
#undef tiny
#undef LN
#undef LD
#undef PN
#undef PD
#undef lg2
#undef lg2_h
#undef lg2_l
#undef ovt
#undef cp
#undef cp_h
#undef cp_l

/* ===== s_exp2l.c ===== */
#undef G
#undef F_hi
#undef F_lo
#undef ln2_hi
#undef ln2_lo
#undef E
#undef H
#define huge __b0089_exp2l_huge
#define twom10000 __b0089_exp2l_twom10000
#define P1 __b0089_exp2l_P1
#define P2 __b0089_exp2l_P2
#define P3 __b0089_exp2l_P3
#define P4 __b0089_exp2l_P4
#define P5 __b0089_exp2l_P5
#define P6 __b0089_exp2l_P6
#define P7 __b0089_exp2l_P7
#define P8 __b0089_exp2l_P8
#define P9 __b0089_exp2l_P9
#define P10 __b0089_exp2l_P10
#define redux __b0089_exp2l_redux
#define tbl __b0089_exp2l_tbl
#define eps __b0089_exp2l_eps
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005-2008 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define	TBLBITS	7
#define	TBLSIZE	(1 << TBLBITS)

#define	BIAS	(LDBL_MAX_EXP - 1)
#define	EXPMASK	(BIAS + LDBL_MAX_EXP)

static volatile long double
    huge      = 0x1p10000L,
    twom10000 = 0x1p-10000L;

static const long double
    P1        = 0x1.62e42fefa39ef35793c7673007e6p-1L,
    P2	      = 0x1.ebfbdff82c58ea86f16b06ec9736p-3L,
    P3        = 0x1.c6b08d704a0bf8b33a762bad3459p-5L,
    P4        = 0x1.3b2ab6fba4e7729ccbbe0b4f3fc2p-7L,
    P5        = 0x1.5d87fe78a67311071dee13fd11d9p-10L,
    P6        = 0x1.430912f86c7876f4b663b23c5fe5p-13L;

static const double
    P7        = 0x1.ffcbfc588b041p-17,
    P8        = 0x1.62c0223a5c7c7p-20,
    P9        = 0x1.b52541ff59713p-24,
    P10       = 0x1.e4cf56a391e22p-28,
    redux     = 0x1.8p112 / TBLSIZE;

static const long double tbl[TBLSIZE] = {
	0x1.6a09e667f3bcc908b2fb1366dfeap-1L,
	0x1.6c012750bdabeed76a99800f4edep-1L,
	0x1.6dfb23c651a2ef220e2cbe1bc0d4p-1L,
	0x1.6ff7df9519483cf87e1b4f3e1e98p-1L,
	0x1.71f75e8ec5f73dd2370f2ef0b148p-1L,
	0x1.73f9a48a58173bd5c9a4e68ab074p-1L,
	0x1.75feb564267c8bf6e9aa33a489a8p-1L,
	0x1.780694fde5d3f619ae02808592a4p-1L,
	0x1.7a11473eb0186d7d51023f6ccb1ap-1L,
	0x1.7c1ed0130c1327c49334459378dep-1L,
	0x1.7e2f336cf4e62105d02ba1579756p-1L,
	0x1.80427543e1a11b60de67649a3842p-1L,
	0x1.82589994cce128acf88afab34928p-1L,
	0x1.8471a4623c7acce52f6b97c6444cp-1L,
	0x1.868d99b4492ec80e41d90ac2556ap-1L,
	0x1.88ac7d98a669966530bcdf2d4cc0p-1L,
	0x1.8ace5422aa0db5ba7c55a192c648p-1L,
	0x1.8cf3216b5448bef2aa1cd161c57ap-1L,
	0x1.8f1ae991577362b982745c72eddap-1L,
	0x1.9145b0b91ffc588a61b469f6b6a0p-1L,
	0x1.93737b0cdc5e4f4501c3f2540ae8p-1L,
	0x1.95a44cbc8520ee9b483695a0e7fep-1L,
	0x1.97d829fde4e4f8b9e920f91e8eb6p-1L,
	0x1.9a0f170ca07b9ba3109b8c467844p-1L,
	0x1.9c49182a3f0901c7c46b071f28dep-1L,
	0x1.9e86319e323231824ca78e64c462p-1L,
	0x1.a0c667b5de564b29ada8b8cabbacp-1L,
	0x1.a309bec4a2d3358c171f770db1f4p-1L,
	0x1.a5503b23e255c8b424491caf88ccp-1L,
	0x1.a799e1330b3586f2dfb2b158f31ep-1L,
	0x1.a9e6b5579fdbf43eb243bdff53a2p-1L,
	0x1.ac36bbfd3f379c0db966a3126988p-1L,
	0x1.ae89f995ad3ad5e8734d17731c80p-1L,
	0x1.b0e07298db66590842acdfc6fb4ep-1L,
	0x1.b33a2b84f15faf6bfd0e7bd941b0p-1L,
	0x1.b59728de559398e3881111648738p-1L,
	0x1.b7f76f2fb5e46eaa7b081ab53ff6p-1L,
	0x1.ba5b030a10649840cb3c6af5b74cp-1L,
	0x1.bcc1e904bc1d2247ba0f45b3d06cp-1L,
	0x1.bf2c25bd71e088408d7025190cd0p-1L,
	0x1.c199bdd85529c2220cb12a0916bap-1L,
	0x1.c40ab5fffd07a6d14df820f17deap-1L,
	0x1.c67f12e57d14b4a2137fd20f2a26p-1L,
	0x1.c8f6d9406e7b511acbc48805c3f6p-1L,
	0x1.cb720dcef90691503cbd1e949d0ap-1L,
	0x1.cdf0b555dc3f9c44f8958fac4f12p-1L,
	0x1.d072d4a07897b8d0f22f21a13792p-1L,
	0x1.d2f87080d89f18ade123989ea50ep-1L,
	0x1.d5818dcfba48725da05aeb66dff8p-1L,
	0x1.d80e316c98397bb84f9d048807a0p-1L,
	0x1.da9e603db3285708c01a5b6d480cp-1L,
	0x1.dd321f301b4604b695de3c0630c0p-1L,
	0x1.dfc97337b9b5eb968cac39ed284cp-1L,
	0x1.e264614f5a128a12761fa17adc74p-1L,
	0x1.e502ee78b3ff6273d130153992d0p-1L,
	0x1.e7a51fbc74c834b548b2832378a4p-1L,
	0x1.ea4afa2a490d9858f73a18f5dab4p-1L,
	0x1.ecf482d8e67f08db0312fb949d50p-1L,
	0x1.efa1bee615a27771fd21a92dabb6p-1L,
	0x1.f252b376bba974e8696fc3638f24p-1L,
	0x1.f50765b6e4540674f84b762861a6p-1L,
	0x1.f7bfdad9cbe138913b4bfe72bd78p-1L,
	0x1.fa7c1819e90d82e90a7e74b26360p-1L,
	0x1.fd3c22b8f71f10975ba4b32bd006p-1L,
	0x1.0000000000000000000000000000p+0L,
	0x1.0163da9fb33356d84a66ae336e98p+0L,
	0x1.02c9a3e778060ee6f7caca4f7a18p+0L,
	0x1.04315e86e7f84bd738f9a20da442p+0L,
	0x1.059b0d31585743ae7c548eb68c6ap+0L,
	0x1.0706b29ddf6ddc6dc403a9d87b1ep+0L,
	0x1.0874518759bc808c35f25d942856p+0L,
	0x1.09e3ecac6f3834521e060c584d5cp+0L,
	0x1.0b5586cf9890f6298b92b7184200p+0L,
	0x1.0cc922b7247f7407b705b893dbdep+0L,
	0x1.0e3ec32d3d1a2020742e4f8af794p+0L,
	0x1.0fb66affed31af232091dd8a169ep+0L,
	0x1.11301d0125b50a4ebbf1aed9321cp+0L,
	0x1.12abdc06c31cbfb92bad324d6f84p+0L,
	0x1.1429aaea92ddfb34101943b2588ep+0L,
	0x1.15a98c8a58e512480d573dd562aep+0L,
	0x1.172b83c7d517adcdf7c8c50eb162p+0L,
	0x1.18af9388c8de9bbbf70b9a3c269cp+0L,
	0x1.1a35beb6fcb753cb698f692d2038p+0L,
	0x1.1bbe084045cd39ab1e72b442810ep+0L,
	0x1.1d4873168b9aa7805b8028990be8p+0L,
	0x1.1ed5022fcd91cb8819ff61121fbep+0L,
	0x1.2063b88628cd63b8eeb0295093f6p+0L,
	0x1.21f49917ddc962552fd29294bc20p+0L,
	0x1.2387a6e75623866c1fadb1c159c0p+0L,
	0x1.251ce4fb2a63f3582ab7de9e9562p+0L,
	0x1.26b4565e27cdd257a673281d3068p+0L,
	0x1.284dfe1f5638096cf15cf03c9fa0p+0L,
	0x1.29e9df51fdee12c25d15f5a25022p+0L,
	0x1.2b87fd0dad98ffddea46538fca24p+0L,
	0x1.2d285a6e4030b40091d536d0733ep+0L,
	0x1.2ecafa93e2f5611ca0f45d5239a4p+0L,
	0x1.306fe0a31b7152de8d5a463063bep+0L,
	0x1.32170fc4cd8313539cf1c3009330p+0L,
	0x1.33c08b26416ff4c9c8610d96680ep+0L,
	0x1.356c55f929ff0c94623476373be4p+0L,
	0x1.371a7373aa9caa7145502f45452ap+0L,
	0x1.38cae6d05d86585a9cb0d9bed530p+0L,
	0x1.3a7db34e59ff6ea1bc9299e0a1fep+0L,
	0x1.3c32dc313a8e484001f228b58cf0p+0L,
	0x1.3dea64c12342235b41223e13d7eep+0L,
	0x1.3fa4504ac801ba0bf701aa417b9cp+0L,
	0x1.4160a21f72e29f84325b8f3dbacap+0L,
	0x1.431f5d950a896dc704439410b628p+0L,
	0x1.44e086061892d03136f409df0724p+0L,
	0x1.46a41ed1d005772512f459229f0ap+0L,
	0x1.486a2b5c13cd013c1a3b69062f26p+0L,
	0x1.4a32af0d7d3de672d8bcf46f99b4p+0L,
	0x1.4bfdad5362a271d4397afec42e36p+0L,
	0x1.4dcb299fddd0d63b36ef1a9e19dep+0L,
	0x1.4f9b2769d2ca6ad33d8b69aa0b8cp+0L,
	0x1.516daa2cf6641c112f52c84d6066p+0L,
	0x1.5342b569d4f81df0a83c49d86bf4p+0L,
	0x1.551a4ca5d920ec52ec620243540cp+0L,
	0x1.56f4736b527da66ecb004764e61ep+0L,
	0x1.58d12d497c7fd252bc2b7343d554p+0L,
	0x1.5ab07dd48542958c93015191e9a8p+0L,
	0x1.5c9268a5946b701c4b1b81697ed4p+0L,
	0x1.5e76f15ad21486e9be4c20399d12p+0L,
	0x1.605e1b976dc08b076f592a487066p+0L,
	0x1.6247eb03a5584b1f0fa06fd2d9eap+0L,
	0x1.6434634ccc31fc76f8714c4ee122p+0L,
	0x1.66238825522249127d9e29b92ea2p+0L,
	0x1.68155d44ca973081c57227b9f69ep+0L,
};

static const float eps[TBLSIZE] = {
	-0x1.5c50p-101,
	-0x1.5d00p-106,
	 0x1.8e90p-102,
	-0x1.5340p-103,
	 0x1.1bd0p-102,
	-0x1.4600p-105,
	-0x1.7a40p-104,
	 0x1.d590p-102,
	-0x1.d590p-101,
	 0x1.b100p-103,
	-0x1.0d80p-105,
	 0x1.6b00p-103,
	-0x1.9f00p-105,
	 0x1.c400p-103,
	 0x1.e120p-103,
	-0x1.c100p-104,
	-0x1.9d20p-103,
	 0x1.a800p-108,
	 0x1.4c00p-106,
	-0x1.9500p-106,
	 0x1.6900p-105,
	-0x1.29d0p-100,
	 0x1.4c60p-103,
	 0x1.13a0p-102,
	-0x1.5b60p-103,
	-0x1.1c40p-103,
	 0x1.db80p-102,
	 0x1.91a0p-102,
	 0x1.dc00p-105,
	 0x1.44c0p-104,
	 0x1.9710p-102,
	 0x1.8760p-103,
	-0x1.a720p-103,
	 0x1.ed20p-103,
	-0x1.49c0p-102,
	-0x1.e000p-111,
	 0x1.86a0p-103,
	 0x1.2b40p-103,
	-0x1.b400p-108,
	 0x1.1280p-99,
	-0x1.02d8p-102,
	-0x1.e3d0p-103,
	-0x1.b080p-105,
	-0x1.f100p-107,
	-0x1.16c0p-105,
	-0x1.1190p-103,
	-0x1.a7d2p-100,
	 0x1.3450p-103,
	-0x1.67c0p-105,
	 0x1.4b80p-104,
	-0x1.c4e0p-103,
	 0x1.6000p-108,
	-0x1.3f60p-105,
	 0x1.93f0p-104,
	 0x1.5fe0p-105,
	 0x1.6f80p-107,
	-0x1.7600p-106,
	 0x1.21e0p-106,
	-0x1.3a40p-106,
	-0x1.40c0p-104,
	-0x1.9860p-105,
	-0x1.5d40p-108,
	-0x1.1d70p-106,
	 0x1.2760p-105,
	 0x0.0000p+0,
	 0x1.21e2p-104,
	-0x1.9520p-108,
	-0x1.5720p-106,
	-0x1.4810p-106,
	-0x1.be00p-109,
	 0x1.0080p-105,
	-0x1.5780p-108,
	-0x1.d460p-105,
	-0x1.6140p-105,
	 0x1.4630p-104,
	 0x1.ad50p-103,
	 0x1.82e0p-105,
	 0x1.1d3cp-101,
	 0x1.6100p-107,
	 0x1.ec30p-104,
	 0x1.f200p-108,
	 0x1.0b40p-103,
	 0x1.3660p-102,
	 0x1.d9d0p-103,
	-0x1.02d0p-102,
	 0x1.b070p-103,
	 0x1.b9c0p-104,
	-0x1.01c0p-103,
	-0x1.dfe0p-103,
	 0x1.1b60p-104,
	-0x1.ae94p-101,
	-0x1.3340p-104,
	 0x1.b3d8p-102,
	-0x1.6e40p-105,
	-0x1.3670p-103,
	 0x1.c140p-104,
	 0x1.1840p-101,
	 0x1.1ab0p-102,
	-0x1.a400p-104,
	 0x1.1f00p-104,
	-0x1.7180p-103,
	 0x1.4ce0p-102,
	 0x1.9200p-107,
	-0x1.54c0p-103,
	 0x1.1b80p-105,
	-0x1.1828p-101,
	 0x1.5720p-102,
	-0x1.a060p-100,
	 0x1.9160p-102,
	 0x1.a280p-104,
	 0x1.3400p-107,
	 0x1.2b20p-102,
	 0x1.7800p-108,
	 0x1.cfd0p-101,
	 0x1.2ef0p-102,
	-0x1.2760p-99,
	 0x1.b380p-104,
	 0x1.0048p-101,
	-0x1.60b0p-102,
	 0x1.a1ccp-100,
	-0x1.a640p-104,
	-0x1.08a0p-101,
	 0x1.7e60p-102,
	 0x1.22c0p-103,
	-0x1.7200p-106,
	 0x1.f0f0p-102,
	 0x1.eb4ep-99,
	 0x1.c6e0p-103,
};

/*
 * exp2l(x): compute the base 2 exponential of x
 *
 * Accuracy: Peak error < 0.502 ulp.
 *
 * Method: (accurate tables)
 *
 *   Reduce x:
 *     x = 2**k + y, for integer k and |y| <= 1/2.
 *     Thus we have exp2(x) = 2**k * exp2(y).
 *
 *   Reduce y:
 *     y = i/TBLSIZE + z - eps[i] for integer i near y * TBLSIZE.
 *     Thus we have exp2(y) = exp2(i/TBLSIZE) * exp2(z - eps[i]),
 *     with |z - eps[i]| <= 2**-8 + 2**-98 for the table used.
 *
 *   We compute exp2(i/TBLSIZE) via table lookup and exp2(z - eps[i]) via
 *   a degree-10 minimax polynomial with maximum error under 2**-120.
 *   The values in exp2t[] and eps[] are chosen such that
 *   exp2t[i] = exp2(i/TBLSIZE + eps[i]), and eps[i] is a small offset such
 *   that exp2t[i] is accurate to 2**-122.
 *
 *   Note that the range of i is +-TBLSIZE/2, so we actually index the tables
 *   by i0 = i + TBLSIZE/2.
 *
 *   This method is due to Gal, with many details due to Gal and Bachelis:
 *
 *	Gal, S. and Bachelis, B.  An Accurate Elementary Mathematical Library
 *	for the IEEE Floating Point Standard.  TOMS 17(1), 26-46 (1991).
 */
long double
ref_exp2l(long double x)
{
	union IEEEl2bits u, v;
	long double r, t, twopk, twopkp10000, z;
	uint32_t hx, ix, i0;
	int k;

	u.e = x;

	/* Filter out exceptional cases. */
	hx = u.xbits.expsign;
	ix = hx & EXPMASK;
	if (ix >= BIAS + 14) {		/* |x| >= 16384 */
		if (ix == BIAS + LDBL_MAX_EXP) {
			if (u.xbits.manh != 0
			    || u.xbits.manl != 0
			    || (hx & 0x8000) == 0)
				return (x + x);	/* x is NaN or +Inf */
			else 
				return (0.0);	/* x is -Inf */
		}
		if (x >= 16384)
			return (huge * huge); /* overflow */
		if (x <= -16495)
			return (twom10000 * twom10000); /* underflow */
	} else if (ix <= BIAS - 115) {		/* |x| < 0x1p-115 */
		return (1.0 + x);
	}

	/*
	 * Reduce x, computing z, i0, and k. The low bits of x + redux
	 * contain the 16-bit integer part of the exponent (k) followed by
	 * TBLBITS fractional bits (i0). We use bit tricks to extract these
	 * as integers, then set z to the remainder.
	 *
	 * Example: Suppose x is 0xabc.123456p0 and TBLBITS is 8.
	 * Then the low-order word of x + redux is 0x000abc12,
	 * We split this into k = 0xabc and i0 = 0x12 (adjusted to
	 * index into the table), then we compute z = 0x0.003456p0.
	 *
	 * XXX If the exponent is negative, the computation of k depends on
	 *     '>>' doing sign extension.
	 */
	u.e = x + redux;
	i0 = (u.bits.manl & 0xffffffff) + TBLSIZE / 2;
	k = (int)i0 >> TBLBITS;
	i0 = i0 & (TBLSIZE - 1);
	u.e -= redux;
	z = x - u.e;
	v.xbits.manh = 0;
	v.xbits.manl = 0;
	if (k >= LDBL_MIN_EXP) {
		v.xbits.expsign = LDBL_MAX_EXP - 1 + k;
		twopk = v.e;
	} else {
		v.xbits.expsign = LDBL_MAX_EXP - 1 + k + 10000;
		twopkp10000 = v.e;
	}

	/* Compute r = exp2(y) = exp2t[i0] * p(z - eps[i]). */
	t = tbl[i0];		/* exp2t[i0] */
	z -= eps[i0];		/* eps[i0]   */
	r = t + t * z * (P1 + z * (P2 + z * (P3 + z * (P4 + z * (P5 + z * (P6
	    + z * (P7 + z * (P8 + z * (P9 + z * P10)))))))));

	/* Scale by 2**k. */
	if(k >= LDBL_MIN_EXP) {
		if (k == LDBL_MAX_EXP)
			return (r * 2.0 * 0x1p16383L);
		return (r * twopk);
	} else {
		return (r * twopkp10000 * twom10000);
	}
}

#undef huge
#undef twom10000
#undef P1
#undef P2
#undef P3
#undef P4
#undef P5
#undef P6
#undef P7
#undef P8
#undef P9
#undef P10
#undef redux
#undef tbl
#undef eps

/* ===== s_erfl.c ===== */
#undef G
#undef F_hi
#undef F_lo
#undef ln2_hi
#undef ln2_lo
#undef E
#undef H
#define tiny __b0089_erfl_tiny
#define half __b0089_erfl_half
#define one __b0089_erfl_one
#define two __b0089_erfl_two
#define efx __b0089_erfl_efx
#define efx8 __b0089_erfl_efx8
#define pp0 __b0089_erfl_pp0
#define pp1 __b0089_erfl_pp1
#define pp2 __b0089_erfl_pp2
#define pp3 __b0089_erfl_pp3
#define pp4 __b0089_erfl_pp4
#define pp5 __b0089_erfl_pp5
#define pp6 __b0089_erfl_pp6
#define pp7 __b0089_erfl_pp7
#define pp8 __b0089_erfl_pp8
#define pp9 __b0089_erfl_pp9
#define qq1 __b0089_erfl_qq1
#define qq2 __b0089_erfl_qq2
#define qq3 __b0089_erfl_qq3
#define qq4 __b0089_erfl_qq4
#define qq5 __b0089_erfl_qq5
#define qq6 __b0089_erfl_qq6
#define qq7 __b0089_erfl_qq7
#define qq8 __b0089_erfl_qq8
#define qq9 __b0089_erfl_qq9
#define erx __b0089_erfl_erx
#define pa0 __b0089_erfl_pa0
#define pa1 __b0089_erfl_pa1
#define pa2 __b0089_erfl_pa2
#define pa3 __b0089_erfl_pa3
#define pa4 __b0089_erfl_pa4
#define pa5 __b0089_erfl_pa5
#define pa6 __b0089_erfl_pa6
#define pa7 __b0089_erfl_pa7
#define pa8 __b0089_erfl_pa8
#define pa9 __b0089_erfl_pa9
#define pa10 __b0089_erfl_pa10
#define pa11 __b0089_erfl_pa11
#define qa1 __b0089_erfl_qa1
#define qa2 __b0089_erfl_qa2
#define qa3 __b0089_erfl_qa3
#define qa4 __b0089_erfl_qa4
#define qa5 __b0089_erfl_qa5
#define qa6 __b0089_erfl_qa6
#define qa7 __b0089_erfl_qa7
#define qa8 __b0089_erfl_qa8
#define qa9 __b0089_erfl_qa9
#define qa10 __b0089_erfl_qa10
#define qa11 __b0089_erfl_qa11
#define qa12 __b0089_erfl_qa12
#define ra0 __b0089_erfl_ra0
#define ra1 __b0089_erfl_ra1
#define ra2 __b0089_erfl_ra2
#define ra3 __b0089_erfl_ra3
#define ra4 __b0089_erfl_ra4
#define ra5 __b0089_erfl_ra5
#define ra6 __b0089_erfl_ra6
#define ra7 __b0089_erfl_ra7
#define ra8 __b0089_erfl_ra8
#define ra9 __b0089_erfl_ra9
#define ra10 __b0089_erfl_ra10
#define ra11 __b0089_erfl_ra11
#define ra12 __b0089_erfl_ra12
#define ra13 __b0089_erfl_ra13
#define ra14 __b0089_erfl_ra14
#define ra15 __b0089_erfl_ra15
#define ra16 __b0089_erfl_ra16
#define sa1 __b0089_erfl_sa1
#define sa2 __b0089_erfl_sa2
#define sa3 __b0089_erfl_sa3
#define sa4 __b0089_erfl_sa4
#define sa5 __b0089_erfl_sa5
#define sa6 __b0089_erfl_sa6
#define sa7 __b0089_erfl_sa7
#define sa8 __b0089_erfl_sa8
#define sa9 __b0089_erfl_sa9
#define sa10 __b0089_erfl_sa10
#define sa11 __b0089_erfl_sa11
#define sa12 __b0089_erfl_sa12
#define sa13 __b0089_erfl_sa13
#define sa14 __b0089_erfl_sa14
#define sa15 __b0089_erfl_sa15
#define sa16 __b0089_erfl_sa16
#define rb0 __b0089_erfl_rb0
#define rb1 __b0089_erfl_rb1
#define rb2 __b0089_erfl_rb2
#define rb3 __b0089_erfl_rb3
#define rb4 __b0089_erfl_rb4
#define rb5 __b0089_erfl_rb5
#define rb6 __b0089_erfl_rb6
#define rb7 __b0089_erfl_rb7
#define rb8 __b0089_erfl_rb8
#define rb9 __b0089_erfl_rb9
#define rb10 __b0089_erfl_rb10
#define rb11 __b0089_erfl_rb11
#define rb12 __b0089_erfl_rb12
#define rb13 __b0089_erfl_rb13
#define rb14 __b0089_erfl_rb14
#define sb1 __b0089_erfl_sb1
#define sb2 __b0089_erfl_sb2
#define sb3 __b0089_erfl_sb3
#define sb4 __b0089_erfl_sb4
#define sb5 __b0089_erfl_sb5
#define sb6 __b0089_erfl_sb6
#define sb7 __b0089_erfl_sb7
#define sb8 __b0089_erfl_sb8
#define sb9 __b0089_erfl_sb9
#define sb10 __b0089_erfl_sb10
#define sb11 __b0089_erfl_sb11
#define sb12 __b0089_erfl_sb12
#define sb13 __b0089_erfl_sb13
#define sb14 __b0089_erfl_sb14
#define rc0 __b0089_erfl_rc0
#define rc1 __b0089_erfl_rc1
#define rc2 __b0089_erfl_rc2
#define rc3 __b0089_erfl_rc3
#define rc4 __b0089_erfl_rc4
#define rc5 __b0089_erfl_rc5
#define rc6 __b0089_erfl_rc6
#define rc7 __b0089_erfl_rc7
#define rc8 __b0089_erfl_rc8
#define rc9 __b0089_erfl_rc9
#define sc1 __b0089_erfl_sc1
#define sc2 __b0089_erfl_sc2
#define sc3 __b0089_erfl_sc3
#define sc4 __b0089_erfl_sc4
#define sc5 __b0089_erfl_sc5
#define sc6 __b0089_erfl_sc6
#define sc7 __b0089_erfl_sc7
#define sc8 __b0089_erfl_sc8
#define sc9 __b0089_erfl_sc9
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
 * See s_erf.c for complete comments.
 *
 * Converted to long double by Steven G. Kargl.
 */
/* XXX Prevent compilers from erroneously constant folding these: */
static const volatile long double tiny = 0x1p-10000L;

static const double
half= 0.5,
one = 1,
two = 2;
/*
 * In the domain [0, 2**-40], only the first term in the power series
 * expansion of erf(x) is used.  The magnitude of the first neglected
 * terms is less than 2**-120.
 */
static const long double
efx  =  1.28379167095512573896158903121545167e-01L,	/* 0xecbff6a7, 0x481dd788, 0xb64d21a8, 0xeb06fc3f */
efx8 =  1.02703333676410059116927122497236133e+00L,	/* 0xecbff6a7, 0x481dd788, 0xb64d21a8, 0xeb06ff3f */
/*
 * Domain [0, 0.84375], range ~[-1.919e-38, 1.919e-38]:
 * |(erf(x) - x)/x - pp(x)/qq(x)| < 2**-125.29
 */
pp0  =  1.28379167095512573896158903121545167e-01L,	/* 0x3ffc06eb, 0xa8214db6, 0x88d71d48, 0xa7f6bfec */
pp1  = -3.14931554396568573802046931159683404e-01L,	/* 0xbffd427d, 0x6ada7263, 0x547eb096, 0x95f37463 */
pp2  = -5.27514920282183487103576956956725309e-02L,	/* 0xbffab023, 0xe5a271e3, 0xb0e79b01, 0x2f7ac962 */
pp3  = -1.13202828509005281355609495523452713e-02L,	/* 0xbff872f1, 0x6a5023a1, 0xe08b3884, 0x326af20f */
pp4  = -9.18626155872522453865998391206048506e-04L,	/* 0xbff4e19f, 0xea5fb024, 0x43247a37, 0xe430b06c */
pp5  = -7.87518862406176274922506447157284230e-05L,	/* 0xbff14a4f, 0x31a85fe0, 0x7fff2204, 0x09c49b37 */
pp6  = -3.42357944472240436548115331090560881e-06L,	/* 0xbfeccb81, 0x4b43c336, 0xcd2eb6c2, 0x903f2d87 */
pp7  = -1.37317432573890412634717890726745428e-07L,	/* 0xbfe826e3, 0x0e915eb6, 0x42aee414, 0xf7e36805 */
pp8  = -2.71115170113861755855049008732113726e-09L,	/* 0xbfe2749e, 0x2b94fd00, 0xecb4d166, 0x0efb91f8 */
pp9  = -3.37925756196555959454018189718117864e-11L,	/* 0xbfdc293e, 0x1d9060cb, 0xd043204a, 0x314cd7f0 */
qq1  =  4.76672625471551170489978555182449450e-01L,	/* 0x3ffde81c, 0xde6531f0, 0x76803bee, 0x526e29e9 */
qq2  =  1.06713144672281502058807525850732240e-01L,	/* 0x3ffbb518, 0xd7a6bb74, 0xcd9bdd33, 0x7601eee5 */
qq3  =  1.47747613127513761102189201923147490e-02L,	/* 0x3ff8e423, 0xae527e18, 0xf12cb447, 0x723b4749 */
qq4  =  1.39939377672028671891148770908874816e-03L,	/* 0x3ff56ed7, 0xba055d84, 0xc21b45c4, 0x388d1812 */
qq5  =  9.44302939359455241271983309378738276e-05L,	/* 0x3ff18c11, 0xc18c99a4, 0x86d0fe09, 0x46387b4c */
qq6  =  4.56199342312522842161301671745365650e-06L,	/* 0x3fed3226, 0x73421d05, 0x08875300, 0x32fa1432 */
qq7  =  1.53019260483764773845294600092361197e-07L,	/* 0x3fe8489b, 0x3a63f627, 0x2b9ad2ce, 0x26516e57 */
qq8  =  3.25542691121324805094777901250005508e-09L,	/* 0x3fe2bf6c, 0x26d93a29, 0x9142be7c, 0x9f1dd043 */
qq9  =  3.37405581964478060434410167262684979e-11L;	/* 0x3fdc28c8, 0xfb8fa1be, 0x10e57eec, 0xaa19e49f */

static const long double
erx  =  8.42700792949714894142232424201210961e-01L,	/* 0x3ffeaf76, 0x7a741088, 0xb0000000, 0x00000000 */
/*
 * Domain [0.84375, 1.25], range ~[-2.521e-36, 2.523e-36]:
 * |(erf(x) - erx) - pa(x)/qa(x)| < 2**-120.15
 */
pa0  = -2.48010117891186017024438233323795897e-17L,	/* 0xbfc7c97f, 0x77812279, 0x6c877f22, 0xef4bfb2e */
pa1  =  4.15107497420594680894327969504526489e-01L,	/* 0x3ffda911, 0xf096fbc2, 0x55662005, 0x2337fa64 */
pa2  = -3.94180628087084846724448515851892609e-02L,	/* 0xbffa42e9, 0xab54528c, 0xad529da1, 0x6efc2af3 */
pa3  =  4.48897599625192107295954790681677462e-02L,	/* 0x3ffa6fbc, 0xa65edba1, 0x0e4cbcea, 0x73ef9a31 */
pa4  =  8.02069252143016600110972019232995528e-02L,	/* 0x3ffb4887, 0x0e8b548e, 0x3230b417, 0x11b553b3 */
pa5  = -1.02729816533435279443621120242391295e-02L,	/* 0xbff850a0, 0x041de3ee, 0xd5bca6c9, 0x4ef5f9f2 */
pa6  =  5.70777694530755634864821094419982095e-03L,	/* 0x3ff77610, 0x9b501e10, 0x4c978382, 0x742df68f */
pa7  =  1.22635150233075521018231779267077071e-03L,	/* 0x3ff5417b, 0x0e623682, 0x60327da0, 0x96b9219e */
pa8  =  5.36100234820204569428412542856666503e-04L,	/* 0x3ff41912, 0x27ceb4c1, 0x1d3298ec, 0x84ced627 */
pa9  = -1.97753571846365167177187858667583165e-04L,	/* 0xbff29eb8, 0x23f5bcf3, 0x15c83c46, 0xe4fda98b */
pa10 =  6.19333039900846970674794789568415105e-05L,	/* 0x3ff103c4, 0x60f88e46, 0xc0c9fb02, 0x13cc7fc1 */
pa11 = -5.40531400436645861492290270311751349e-06L,	/* 0xbfed6abe, 0x9665f8a8, 0xdd0ad3ba, 0xe5dc0ee3 */
qa1  =  9.05041313265490487793231810291907851e-01L,	/* 0x3ffecf61, 0x93340222, 0xe9930620, 0xc4e61168 */
qa2  =  6.79848064708886864767240880834868092e-01L,	/* 0x3ffe5c15, 0x0ba858dc, 0xf7900ae9, 0xfea1e09a */
qa3  =  4.04720609926471677581066689316516445e-01L,	/* 0x3ffd9e6f, 0x145e9b00, 0x6d8c1749, 0xd2928623 */
qa4  =  1.69183273898369996364661075664302225e-01L,	/* 0x3ffc5a7c, 0xc2a363c1, 0xd6c19097, 0xef9b4063 */
qa5  =  7.44476185988067992342479750486764248e-02L,	/* 0x3ffb30ef, 0xfc7259ef, 0x1bcbb089, 0x686dd62d */
qa6  =  2.02981172725892407200420389604788573e-02L,	/* 0x3ff94c90, 0x7976cb0e, 0x21e1d36b, 0x0f09ca2b */
qa7  =  6.94281866271607668268269403102277234e-03L,	/* 0x3ff7c701, 0x2b193250, 0xc5d46ecc, 0x374843d8 */
qa8  =  1.12952275469171559611651594706820034e-03L,	/* 0x3ff52818, 0xfd2a7c06, 0xd13e38fd, 0xda4b34f5 */
qa9  =  3.13736683241992737197226578597710179e-04L,	/* 0x3ff348fa, 0x0cb48d18, 0x051f849b, 0x135ccf74 */
qa10 =  1.17037675204033225470121134087771410e-05L,	/* 0x3fee88b6, 0x98f47704, 0xa5d8f8f2, 0xc6422e11 */
qa11 =  4.61312518293853991439362806880973592e-06L,	/* 0x3fed3594, 0xe31db94f, 0x3592b693, 0xed4386b4 */
qa12 = -1.02158572037456893687737553657431771e-06L;	/* 0xbfeb123a, 0xd60d9b1e, 0x1f6fdeb9, 0x7dc8410a */
/*
 * Domain [1.25,2.85715], range ~[-2.922e-37,2.922e-37]:
 * |log(x*erfc(x)) + x**2 + 0.5625 - ra(x)/sa(x)| < 2**-121.36
 */
static const long double
ra0  = -9.86494292470069009555706994426014461e-03L,	/* 0xbff84341, 0x239e8709, 0xe941b06a, 0xcb4b6ec5 */
ra1  = -1.13580436992565640457579040117568870e+00L,	/* 0xbfff22c4, 0x133f7c0d, 0x72d5e231, 0x2eb1ee3f */
ra2  = -4.89744330295291950661185707066921755e+01L,	/* 0xc00487cb, 0xa38b4fc2, 0xc136695b, 0xc1df8047 */
ra3  = -1.10766149300215937173768072715352140e+03L,	/* 0xc00914ea, 0x55e6beb3, 0xabc50e07, 0xb6e5664d */
ra4  = -1.49991031232170934967642795601952100e+04L,	/* 0xc00cd4b8, 0xd33243e6, 0xffbf6545, 0x3c57ef6e */
ra5  = -1.29805749738318462882524181556996692e+05L,	/* 0xc00ffb0d, 0xbfeed9b6, 0x5b2a3ff4, 0xe245bd3c */
ra6  = -7.42828497044940065828871976644647850e+05L,	/* 0xc0126ab5, 0x8fe7caca, 0x473352d9, 0xcd4e0c90 */
ra7  = -2.85637299581890734287995171242421106e+06L,	/* 0xc0145cad, 0xa7f76fe7, 0x3e358051, 0x1799f927 */
ra8  = -7.40674797129824999383748865571026084e+06L,	/* 0xc015c412, 0x6fe29c02, 0x298ad158, 0x7d24e45c */
ra9  = -1.28653420911930973914078724204151759e+07L,	/* 0xc016889e, 0x7c2eb0dc, 0x95d5863b, 0x0aa34dc3 */
ra10 = -1.47198163599330179552932489109452638e+07L,	/* 0xc016c136, 0x90b84923, 0xf9bcb497, 0x19bbd0f5 */
ra11 = -1.07812992258382800318665248311522624e+07L,	/* 0xc0164904, 0xe673a113, 0x35d7f079, 0xe13701f3 */
ra12 = -4.83545565681708642630419905537756076e+06L,	/* 0xc0152721, 0xfea094a8, 0x869eb39d, 0x413d6f13 */
ra13 = -1.23956521201673964822976917356685286e+06L,	/* 0xc0132ea0, 0xd3646baa, 0x2fe62b0d, 0xbae5ce85 */
ra14 = -1.62289333553652417591275333240371812e+05L,	/* 0xc0103cf8, 0xaab1e2d6, 0x4c25e014, 0x248d76ab */
ra15 = -8.82890392601176969729168894389833110e+03L,	/* 0xc00c13e7, 0x3b3d8f94, 0x6fbda6f6, 0xe7049a82 */
ra16 = -1.22591866337261720023681535568334619e+02L,	/* 0xc005ea5e, 0x12358891, 0xcfa712c5, 0x77f050d4 */
sa1  =  6.44508918884710829371852723353794047e+01L,	/* 0x400501cd, 0xb69a6c0f, 0x5716de14, 0x47161af6 */
sa2  =  1.76118475473171481523704824327358534e+03L,	/* 0x4009b84b, 0xd305829f, 0xc4c771b0, 0xbf1f7f9b */
sa3  =  2.69448346969488374857087646131950188e+04L,	/* 0x400da503, 0x56bacc05, 0x4fdba68d, 0x2cca27e6 */
sa4  =  2.56826633369941456778326497384543763e+05L,	/* 0x4010f59d, 0x51124428, 0x69c41de6, 0xbd0d5753 */
sa5  =  1.60647413092257206847700054645905859e+06L,	/* 0x40138834, 0xa2184244, 0x557a1bed, 0x68c9d556 */
sa6  =  6.76963075165099718574753447122393797e+06L,	/* 0x40159d2f, 0x7b01b0cc, 0x8bac9e95, 0x5d35d56e */
sa7  =  1.94295690905361884290986932493647741e+07L,	/* 0x40172878, 0xc1172d61, 0x3068501e, 0x2f3c71da */
sa8  =  3.79774781017759149060839255547073541e+07L,	/* 0x401821be, 0xc30d06fe, 0x410563d7, 0x032111fd */
sa9  =  5.00659831846029484248302236457727397e+07L,	/* 0x40187df9, 0x1f97a111, 0xc51d6ac2, 0x4b389793 */
sa10 =  4.36486287620506484276130525941972541e+07L,	/* 0x40184d03, 0x3a618ae0, 0x2a723357, 0xfa45c60a */
sa11 =  2.43779678791333894255510508253951934e+07L,	/* 0x401773fa, 0x6fe10ee2, 0xc467850d, 0xc6b7ff30 */
sa12 =  8.30732360384443202039372372212966542e+06L,	/* 0x4015fb09, 0xee6a5631, 0xdd98de7e, 0x8b00461a */
sa13 =  1.60160846942050515734192397495105693e+06L,	/* 0x40138704, 0x8782bf13, 0x5b8fb315, 0xa898abe5 */
sa14 =  1.54255505242533291014555153757001825e+05L,	/* 0x40102d47, 0xc0abc98e, 0x843c9490, 0xb4352440 */
sa15 =  5.87949220002375547561467275493888824e+03L,	/* 0x400b6f77, 0xe00d21d1, 0xec4d41e8, 0x2f8e1673 */
sa16 =  4.97272976346793193860385983372237710e+01L;	/* 0x40048dd1, 0x816c1b3f, 0x24f540a6, 0x4cfe03cc */
/*
 * Domain [2.85715,9], range ~[-7.886e-37,7.918e-37]:
 * |log(x*erfc(x)) + x**2 + 0.5625 - rb(x)/sb(x)| < 2**-120
 */
static const long double
rb0  = -9.86494292470008707171371994479162369e-3L, /* 0xbff84341, 0x239e86f4, 0x2f57e561, 0xf4469360 */
rb1  = -1.57047326624110727986326503729442830L,    /* 0xbfff920a, 0x8935bf73, 0x8803b894, 0x4656482d */
rb2  = -1.03228196364885474342132255440317065e2L,  /* 0xc0059ce9, 0xac4ed0ff, 0x2cff0ff7, 0x5e70d1ab */
rb3  = -3.74000570653418227179358710865224376e3L,  /* 0xc00ad380, 0x2ebf7835, 0xf6b07ed2, 0x861242f7 */
rb4  = -8.35435477739098044190860390632813956e4L,  /* 0xc00f4657, 0x8c3ae934, 0x3647d7b3, 0x80e76fb7 */
rb5  = -1.21398672055223642118716640216747152e6L,  /* 0xc0132862, 0x2b8761c8, 0x27d18c0f, 0x137c9463 */
rb6  = -1.17669175877248796101665344873273970e7L,  /* 0xc0166719, 0x0b2cea46, 0x81f14174, 0x11602ea5 */
rb7  = -7.66108006086998253606773064264599615e7L,  /* 0xc019243f, 0x3c26f4f0, 0x1cc05241, 0x3b953728 */
rb8  = -3.32547117558141845968704725353130804e8L,  /* 0xc01b3d24, 0x42d8ee26, 0x24ef6f3b, 0x604a8c65 */
rb9  = -9.41561252426350696802167711221739746e8L,  /* 0xc01cc0f8, 0xad23692a, 0x8ddb2310, 0xe9937145 */
rb10 = -1.67157110805390944549427329626281063e9L,  /* 0xc01d8e88, 0x9a903734, 0x09a55fa3, 0xd205c903 */
rb11 = -1.74339631004410841337645931421427373e9L,  /* 0xc01d9fa8, 0x77582d2a, 0xc183b8ab, 0x7e00cb05 */
rb12 = -9.57655233596934915727573141357471703e8L,  /* 0xc01cc8a5, 0x460cc685, 0xd0271fa0, 0x6a70e3da */
rb13 = -2.26320062731339353035254704082495066e8L,  /* 0xc01aafab, 0xd7d76721, 0xc9720e11, 0x6a8bd489 */
rb14 = -1.42777302996263256686002973851837039e7L,  /* 0xc016b3b8, 0xc499689f, 0x2b88d965, 0xc32414f9 */
sb1  =  1.08512869705594540211033733976348506e2L,  /* 0x4005b20d, 0x2db7528d, 0x00d20dcb, 0x858f6191 */
sb2  =  5.02757713761390460534494530537572834e3L,  /* 0x400b3a39, 0x3bf4a690, 0x3025d28d, 0xfd40a891 */
sb3  =  1.31019107205412870059331647078328430e5L,  /* 0x400fffcb, 0x1b71d05e, 0x3b28361d, 0x2a3c3690 */
sb4  =  2.13021555152296846166736757455018030e6L,  /* 0x40140409, 0x3c6984df, 0xc4491d7c, 0xb04aa08d */
sb5  =  2.26649105281820861953868568619768286e7L,  /* 0x401759d6, 0xce8736f0, 0xf28ad037, 0x2a901e0c */
sb6  =  1.61071939490875921812318684143076081e8L,  /* 0x401a3338, 0x686fb541, 0x6bd27d06, 0x4f95c9ac */
sb7  =  7.66895673844301852676056750497991966e8L,  /* 0x401c6daf, 0x31cec121, 0x54699126, 0x4bd9bf9e */
sb8  =  2.41884450436101936436023058196042526e9L,  /* 0x401e2059, 0x46b0b8d7, 0x87b64cbf, 0x78bc296d */
sb9  =  4.92403055884071695093305291535107666e9L,  /* 0x401f257e, 0xbe5ed739, 0x39e17346, 0xcadd2e55 */
sb10 =  6.18627786365587486459633615573786416e9L,  /* 0x401f70bb, 0x1be7a7e7, 0x6a45b5ae, 0x607c70f0 */
sb11 =  4.45898013426501378097430226324743199e9L,  /* 0x401f09c6, 0xa32643d7, 0xf1724620, 0x9ea46c32 */
sb12 =  1.63006115763329848117160344854224975e9L,  /* 0x401d84a3, 0x0996887f, 0x65a4f43b, 0x978c1d74 */
sb13 =  2.39216717012421697446304015847567721e8L,  /* 0x401ac845, 0x09a065c2, 0x30095da7, 0x9d72d6ae */
sb14 =  7.84837329009278694937250358810225609e6L;  /* 0x4015df06, 0xd5290e15, 0x63031fac, 0x4d9c894c */
/*
 * Domain [9,108], range ~[-5.324e-38,5.340e-38]:
 * |log(x*erfc(x)) + x**2 + 0.5625 - r(x)/s(x)| < 2**-124
 */
static const long double
rc0  = -9.86494292470008707171367567652935673e-3L, /* 0xbff84341, 0x239e86f4, 0x2f57e55b, 0x1aa10fd3 */
rc1  = -1.26229447747315096406518846411562266L,    /* 0xbfff4325, 0xbb1aab28, 0xda395cd9, 0xfb861c15 */
rc2  = -6.13742634438922591780742637728666162e1L,  /* 0xc004eafe, 0x7dd51cd8, 0x3c7c5928, 0x751e50cf */
rc3  = -1.50455835478908280402912854338421517e3L,  /* 0xc0097823, 0xbc15b9ab, 0x3d60745c, 0x523e80a5 */
rc4  = -2.04415631865861549920184039902945685e4L,  /* 0xc00d3f66, 0x40b3fc04, 0x5388f2ec, 0xb009e1f0 */
rc5  = -1.57625662981714582753490610560037638e5L,  /* 0xc01033dc, 0xd4dc95b6, 0xfd4da93b, 0xf355b4a9 */
rc6  = -6.73473451616752528402917538033283794e5L,  /* 0xc01248d8, 0x2e73a4f9, 0xcded49c5, 0xfa3bfeb7 */
rc7  = -1.47433165421387483167186683764364857e6L,  /* 0xc01367f1, 0xba77a8f7, 0xcfdd0dbb, 0x25d554b3 */
rc8  = -1.38811981807868828563794929997744139e6L,  /* 0xc01352e5, 0x7d16d9ad, 0xbbdcbf38, 0x38fbc5ea */
rc9  = -3.59659700530831825640766479698155060e5L,  /* 0xc0115f3a, 0xecd57f45, 0x21f8ad6c, 0x910a5958 */
sc1  =  7.72730753022908298637508998072635696e1L,  /* 0x40053517, 0xa10d52bc, 0xdabb55b6, 0xbd0328cd */
sc2  =  2.36825757341694050500333261769082182e3L,  /* 0x400a2808, 0x3e0a9b42, 0x82977842, 0x9c5de29e */
sc3  =  3.72210540173034735352888847134073099e4L,  /* 0x400e22ca, 0x1ba827ef, 0xac8390d7, 0x1fc39a41 */
sc4  =  3.24136032646418336712461033591393412e5L,  /* 0x40113c8a, 0x0216e100, 0xc59d1e44, 0xf0e68d9d */
sc5  =  1.57836135851134393802505823370009175e6L,  /* 0x40138157, 0x95bc7664, 0x17575961, 0xdbe58eeb */
sc6  =  4.12881981392063738026679089714182355e6L,  /* 0x4014f801, 0x9e82e8d2, 0xb8b3a70e, 0xfd84185d */
sc7  =  5.24438427289213488410596395361544142e6L,  /* 0x40154017, 0x81177109, 0x2aa6c3b0, 0x1f106625 */
sc8  =  2.59909544563616121735963429710382149e6L,  /* 0x40143d45, 0xbb90a9b1, 0x12bf9390, 0xa827a700 */
sc9  =  2.80930665169282501639651995082335693e5L;  /* 0x40111258, 0xaa92222e, 0xa97e3216, 0xa237fa6c */

long double
ref_erfl(long double x)
{
	long double ax,R,S,P,Q,s,y,z,r;
	uint64_t lx, llx;
	int32_t i;
	uint16_t hx;

	EXTRACT_LDBL128_WORDS(hx, lx, llx, x);

	if((hx & 0x7fff) == 0x7fff) {	/* erfl(nan)=nan */
		i = (hx>>15)<<1;
		return (1-i)+one/x;	/* erfl(+-inf)=+-1 */
	}

	ax = fabsl(x);
	if(ax < 0.84375) {
	    if(ax < 0x1p-40L) {
	        if(ax < 0x1p-16373L)	
		    return (8*x+efx8*x)/8;	/* avoid spurious underflow */
		return x + efx*x;
	    }
	    z = x*x;
	    r = pp0+z*(pp1+z*(pp2+z*(pp3+z*(pp4+z*(pp5+z*(pp6+z*(pp7+
		z*(pp8+z*pp9))))))));
	    s = one+z*(qq1+z*(qq2+z*(qq3+z*(qq4+z*(qq5+z*(qq6+z*(qq7+
		z*(qq8+z*qq9))))))));
	    y = r/s;
	    return x + x*y;
	}
	if(ax < 1.25) {
	    s = ax-one;
	    P = pa0+s*(pa1+s*(pa2+s*(pa3+s*(pa4+s*(pa5+s*(pa6+s*(pa7+
		s*(pa8+s*(pa9+s*(pa10+s*pa11))))))))));
	    Q = one+s*(qa1+s*(qa2+s*(qa3+s*(qa4+s*(qa5+s*(qa6+s*(qa7+
		s*(qa8+s*(qa9+s*(qa10+s*(qa11+s*qa12)))))))))));
	    if(x>=0) return (erx + P/Q); else return (-erx - P/Q);
	}
	if (ax >= 9) {			/* inf>|x|>= 9 */
	    if(x>=0) return (one-tiny); else return (tiny-one);
	}
	s = one/(ax*ax);
	if(ax < 2.85715) {	/* |x| < 2.85715 */
	    R=ra0+s*(ra1+s*(ra2+s*(ra3+s*(ra4+s*(ra5+s*(ra6+s*(ra7+
		s*(ra8+s*(ra9+s*(ra10+s*(ra11+s*(ra12+s*(ra13+s*(ra14+
		s*(ra15+s*ra16)))))))))))))));
	    S=one+s*(sa1+s*(sa2+s*(sa3+s*(sa4+s*(sa5+s*(sa6+s*(sa7+
		s*(sa8+s*(sa9+s*(sa10+s*(sa11+s*(sa12+s*(sa13+s*(sa14+
		s*(sa15+s*sa16)))))))))))))));
	} else {	/* |x| >= 2.85715 */
	    R=rb0+s*(rb1+s*(rb2+s*(rb3+s*(rb4+s*(rb5+s*(rb6+s*(rb7+
		s*(rb8+s*(rb9+s*(rb10+s*(rb11+s*(rb12+s*(rb13+
		s*rb14)))))))))))));
	    S=one+s*(sb1+s*(sb2+s*(sb3+s*(sb4+s*(sb5+s*(sb6+s*(sb7+
		s*(sb8+s*(sb9+s*(sb10+s*(sb11+s*(sb12+s*(sb13+
		s*sb14)))))))))))));
	}
	z = (float)ax;
	r = expl(-z*z-0.5625)*expl((z-ax)*(z+ax)+R/S);
	if(x>=0) return (one-r/ax); else return (r/ax-one);
}

long double
ref_erfcl(long double x)
{
	long double ax,R,S,P,Q,s,y,z,r;
	uint64_t lx, llx;
	uint16_t hx;

	EXTRACT_LDBL128_WORDS(hx, lx, llx, x);

	if((hx & 0x7fff) == 0x7fff) {	/* erfcl(nan)=nan */
					/* erfcl(+-inf)=0,2 */
	    return ((hx>>15)<<1)+one/x;
	}

	ax = fabsl(x);
	if(ax < 0.84375L) {
	    if(ax < 0x1p-34L)
		return one-x;
	    z = x*x;
	    r = pp0+z*(pp1+z*(pp2+z*(pp3+z*(pp4+z*(pp5+z*(pp6+z*(pp7+
		z*(pp8+z*pp9))))))));
	    s = one+z*(qq1+z*(qq2+z*(qq3+z*(qq4+z*(qq5+z*(qq6+z*(qq7+
		z*(qq8+z*qq9))))))));
	    y = r/s;
	    if(ax < 0.25L) {  	/* x<1/4 */
		return one-(x+x*y);
	    } else {
		r = x*y;
		r += (x-half);
	       return half - r;
	    }
	}
	if(ax < 1.25L) {
	    s = ax-one;
	    P = pa0+s*(pa1+s*(pa2+s*(pa3+s*(pa4+s*(pa5+s*(pa6+s*(pa7+
		    s*(pa8+s*(pa9+s*(pa10+s*pa11))))))))));
	    Q = one+s*(qa1+s*(qa2+s*(qa3+s*(qa4+s*(qa5+s*(qa6+s*(qa7+
		    s*(qa8+s*(qa9+s*(qa10+s*(qa11+s*qa12)))))))))));
	    if(x>=0) {
	        z  = one-erx; return z - P/Q;
	    } else {
		z = erx+P/Q; return one+z;
	    }
	}

	if(ax < 108) {			/* |x| < 108 */
 	    s = one/(ax*ax);
	    if(ax < 2.85715) {		/* |x| < 2.85715 */
	        R=ra0+s*(ra1+s*(ra2+s*(ra3+s*(ra4+s*(ra5+s*(ra6+s*(ra7+
		    s*(ra8+s*(ra9+s*(ra10+s*(ra11+s*(ra12+s*(ra13+s*(ra14+
		    s*(ra15+s*ra16)))))))))))))));
	        S=one+s*(sa1+s*(sa2+s*(sa3+s*(sa4+s*(sa5+s*(sa6+s*(sa7+
		    s*(sa8+s*(sa9+s*(sa10+s*(sa11+s*(sa12+s*(sa13+s*(sa14+
		    s*(sa15+s*sa16)))))))))))))));
	    } else if(ax < 9) {
		R=rb0+s*(rb1+s*(rb2+s*(rb3+s*(rb4+s*(rb5+s*(rb6+s*(rb7+
		    s*(rb8+s*(rb9+s*(rb10+s*(rb11+s*(rb12+s*(rb13+
		    s*rb14)))))))))))));
		S=one+s*(sb1+s*(sb2+s*(sb3+s*(sb4+s*(sb5+s*(sb6+s*(sb7+
		    s*(sb8+s*(sb9+s*(sb10+s*(sb11+s*(sb12+s*(sb13+
		    s*sb14)))))))))))));
	    } else {
		if(x < -9) return two-tiny;	/* x < -9 */
		R=rc0+s*(rc1+s*(rc2+s*(rc3+s*(rc4+s*(rc5+s*(rc6+s*(rc7+
		    s*(rc8+s*rc9))))))));
		S=one+s*(sc1+s*(sc2+s*(sc3+s*(sc4+s*(sc5+s*(sc6+s*(sc7+
		    s*(sc8+s*sc9))))))));
	    }
	    z = (float)ax;
	    r = expl(-z*z-0.5625)*expl((z-ax)*(z+ax)+R/S);
	    if(x>0) return r/ax; else return two-r/ax;
	} else {
	    if(x>0) return tiny*tiny; else return two-tiny;
	}
}

#undef tiny
#undef half
#undef one
#undef two
#undef efx
#undef efx8
#undef pp0
#undef pp1
#undef pp2
#undef pp3
#undef pp4
#undef pp5
#undef pp6
#undef pp7
#undef pp8
#undef pp9
#undef qq1
#undef qq2
#undef qq3
#undef qq4
#undef qq5
#undef qq6
#undef qq7
#undef qq8
#undef qq9
#undef erx
#undef pa0
#undef pa1
#undef pa2
#undef pa3
#undef pa4
#undef pa5
#undef pa6
#undef pa7
#undef pa8
#undef pa9
#undef pa10
#undef pa11
#undef qa1
#undef qa2
#undef qa3
#undef qa4
#undef qa5
#undef qa6
#undef qa7
#undef qa8
#undef qa9
#undef qa10
#undef qa11
#undef qa12
#undef ra0
#undef ra1
#undef ra2
#undef ra3
#undef ra4
#undef ra5
#undef ra6
#undef ra7
#undef ra8
#undef ra9
#undef ra10
#undef ra11
#undef ra12
#undef ra13
#undef ra14
#undef ra15
#undef ra16
#undef sa1
#undef sa2
#undef sa3
#undef sa4
#undef sa5
#undef sa6
#undef sa7
#undef sa8
#undef sa9
#undef sa10
#undef sa11
#undef sa12
#undef sa13
#undef sa14
#undef sa15
#undef sa16
#undef rb0
#undef rb1
#undef rb2
#undef rb3
#undef rb4
#undef rb5
#undef rb6
#undef rb7
#undef rb8
#undef rb9
#undef rb10
#undef rb11
#undef rb12
#undef rb13
#undef rb14
#undef sb1
#undef sb2
#undef sb3
#undef sb4
#undef sb5
#undef sb6
#undef sb7
#undef sb8
#undef sb9
#undef sb10
#undef sb11
#undef sb12
#undef sb13
#undef sb14
#undef rc0
#undef rc1
#undef rc2
#undef rc3
#undef rc4
#undef rc5
#undef rc6
#undef rc7
#undef rc8
#undef rc9
#undef sc1
#undef sc2
#undef sc3
#undef sc4
#undef sc5
#undef sc6
#undef sc7
#undef sc8
#undef sc9

/* ===== s_logl.c ===== */
#undef G
#undef F_hi
#undef F_lo
#undef ln2_hi
#undef ln2_lo
#undef E
#undef H
#define P2 __b0089_logl_P2
#define P3 __b0089_logl_P3
#define P4 __b0089_logl_P4
#define P5 __b0089_logl_P5
#define P6 __b0089_logl_P6
#define P7 __b0089_logl_P7
#define P8 __b0089_logl_P8
#define P9 __b0089_logl_P9
#define P10 __b0089_logl_P10
#define P11 __b0089_logl_P11
#define P12 __b0089_logl_P12
#define P13 __b0089_logl_P13
#define P14 __b0089_logl_P14
#define zero __b0089_logl_zero
#define T __b0089_logl_T
#define U __b0089_logl_U
#define invln10_hi __b0089_logl_invln10_hi
#define invln2_hi __b0089_logl_invln2_hi
#define invln10_lo __b0089_logl_invln10_lo
#define invln2_lo __b0089_logl_invln2_lo
#define invln10_lo_plus_hi __b0089_logl_invln10_lo_plus_hi
#define invln2_lo_plus_hi __b0089_logl_invln2_lo_plus_hi
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2007-2013 Bruce D. Evans
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Implementation of the natural logarithm of x for 128-bit format.
 *
 * First decompose x into its base 2 representation:
 *
 *    log(x) = log(X * 2**k), where X is in [1, 2)
 *           = log(X) + k * log(2).
 *
 * Let X = X_i + e, where X_i is the center of one of the intervals
 * [-1.0/256, 1.0/256), [1.0/256, 3.0/256), .... [2.0-1.0/256, 2.0+1.0/256)
 * and X is in this interval.  Then
 *
 *    log(X) = log(X_i + e)
 *           = log(X_i * (1 + e / X_i))
 *           = log(X_i) + log(1 + e / X_i).
 *
 * The values log(X_i) are tabulated below.  Let d = e / X_i and use
 *
 *    log(1 + d) = p(d)
 *
 * where p(d) = d - 0.5*d*d + ... is a special minimax polynomial of
 * suitably high degree.
 *
 * To get sufficiently small roundoff errors, k * log(2), log(X_i), and
 * sometimes (if |k| is not large) the first term in p(d) must be evaluated
 * and added up in extra precision.  Extra precision is not needed for the
 * rest of p(d).  In the worst case when k = 0 and log(X_i) is 0, the final
 * error is controlled mainly by the error in the second term in p(d).  The
 * error in this term itself is at most 0.5 ulps from the d*d operation in
 * it.  The error in this term relative to the first term is thus at most
 * 0.5 * |-0.5| * |d| < 1.0/1024 ulps.  We aim for an accumulated error of
 * at most twice this at the point of the final rounding step.  Thus the
 * final error should be at most 0.5 + 1.0/512 = 0.5020 ulps.  Exhaustive
 * testing of a float variant of this function showed a maximum final error
 * of 0.5008 ulps.  Non-exhaustive testing of a double variant of this
 * function showed a maximum final error of 0.5078 ulps (near 1+1.0/256).
 *
 * We made the maximum of |d| (and thus the total relative error and the
 * degree of p(d)) small by using a large number of intervals.  Using
 * centers of intervals instead of endpoints reduces this maximum by a
 * factor of 2 for a given number of intervals.  p(d) is special only
 * in beginning with the Taylor coefficients 0 + 1*d, which tends to happen
 * naturally.  The most accurate minimax polynomial of a given degree might
 * be different, but then we wouldn't want it since we would have to do
 * extra work to avoid roundoff error (especially for P0*d instead of d).
 */

#ifndef NO_STRUCT_RETURN
#define	STRUCT_RETURN
#endif
#if !defined(NO_UTAB) && !defined(NO_UTABL)
#define	USE_UTAB
#endif

/*
 * Domain [-0.005280, 0.004838], range ~[-1.1577e-37, 1.1582e-37]:
 * |log(1 + d)/d - p(d)| < 2**-122.7
 */
static const long double
P2 = -0.5L,
P3 =  3.33333333333333333333333333333233795e-1L,	/*  0x15555555555555555555555554d42.0p-114L */
P4 = -2.49999999999999999999999999941139296e-1L,	/* -0x1ffffffffffffffffffffffdab14e.0p-115L */
P5 =  2.00000000000000000000000085468039943e-1L,	/*  0x19999999999999999999a6d3567f4.0p-115L */
P6 = -1.66666666666666666666696142372698408e-1L,	/* -0x15555555555555555567267a58e13.0p-115L */
P7 =  1.42857142857142857119522943477166120e-1L,	/*  0x1249249249249248ed79a0ae434de.0p-115L */
P8 = -1.24999999999999994863289015033581301e-1L;	/* -0x1fffffffffffffa13e91765e46140.0p-116L */
/* Double precision gives ~ 53 + log2(P9 * max(|d|)**8) ~= 120 bits. */
static const double
P9 =  1.1111111111111401e-1,		/*  0x1c71c71c71c7ed.0p-56 */
P10 = -1.0000000000040135e-1,		/* -0x199999999a0a92.0p-56 */
P11 =  9.0909090728136258e-2,		/*  0x1745d173962111.0p-56 */
P12 = -8.3333318851855284e-2,		/* -0x1555551722c7a3.0p-56 */
P13 =  7.6928634666404178e-2,		/*  0x13b1985204a4ae.0p-56 */
P14 = -7.1626810078462499e-2;		/* -0x12562276cdc5d0.0p-56 */

static volatile const double zero = 0;

#define	INTERVALS	128
#define	LOG2_INTERVALS	7
#define	TSIZE		(INTERVALS + 1)
#define	G(i)		(T[(i)].G)
#define	F_hi(i)		(T[(i)].F_hi)
#define	F_lo(i)		(T[(i)].F_lo)
#define	ln2_hi		F_hi(TSIZE - 1)
#define	ln2_lo		F_lo(TSIZE - 1)
#define	E(i)		(U[(i)].E)
#define	H(i)		(U[(i)].H)

static const struct {
	float	G;			/* 1/(1 + i/128) rounded to 8/9 bits */
	float	F_hi;			/* log(1 / G_i) rounded (see below) */
	/* The compiler will insert 8 bytes of padding here. */
	long double F_lo;		/* next 113 bits for log(1 / G_i) */
} T[TSIZE] = {
	/*
	 * ln2_hi and each F_hi(i) are rounded to a number of bits that
	 * makes F_hi(i) + dk*ln2_hi exact for all i and all dk.
	 *
	 * The last entry (for X just below 2) is used to define ln2_hi
	 * and ln2_lo, to ensure that F_hi(i) and F_lo(i) cancel exactly
	 * with dk*ln2_hi and dk*ln2_lo, respectively, when dk = -1.
	 * This is needed for accuracy when x is just below 1.  (To avoid
	 * special cases, such x are "reduced" strangely to X just below
	 * 2 and dk = -1, and then the exact cancellation is needed
	 * because any the error from any non-exactness would be too
	 * large).
	 *
	 * The relevant range of dk is [-16445, 16383].  The maximum number
	 * of bits in F_hi(i) that works is very dependent on i but has
	 * a minimum of 93.  We only need about 12 bits in F_hi(i) for
	 * it to provide enough extra precision.
	 *
	 * We round F_hi(i) to 24 bits so that it can have type float,
	 * mainly to minimize the size of the table.  Using all 24 bits
	 * in a float for it automatically satisfies the above constraints.
	 */
     0x800000.0p-23,  0,               0,
     0xfe0000.0p-24,  0x8080ac.0p-30, -0x14ee431dae6674afa0c4bfe16e8fd.0p-144L,
     0xfc0000.0p-24,  0x8102b3.0p-29, -0x1db29ee2d83717be918e1119642ab.0p-144L,
     0xfa0000.0p-24,  0xc24929.0p-29,  0x1191957d173697cf302cc9476f561.0p-143L,
     0xf80000.0p-24,  0x820aec.0p-28,  0x13ce8888e02e78eba9b1113bc1c18.0p-142L,
     0xf60000.0p-24,  0xa33577.0p-28, -0x17a4382ce6eb7bfa509bec8da5f22.0p-142L,
     0xf48000.0p-24,  0xbc42cb.0p-28, -0x172a21161a107674986dcdca6709c.0p-143L,
     0xf30000.0p-24,  0xd57797.0p-28, -0x1e09de07cb958897a3ea46e84abb3.0p-142L,
     0xf10000.0p-24,  0xf7518e.0p-28,  0x1ae1eec1b036c484993c549c4bf40.0p-151L,
     0xef0000.0p-24,  0x8cb9df.0p-27, -0x1d7355325d560d9e9ab3d6ebab580.0p-141L,
     0xed8000.0p-24,  0x999ec0.0p-27, -0x1f9f02d256d5037108f4ec21e48cd.0p-142L,
     0xec0000.0p-24,  0xa6988b.0p-27, -0x16fc0a9d12c17a70f7a684c596b12.0p-143L,
     0xea0000.0p-24,  0xb80698.0p-27,  0x15d581c1e8da99ded322fb08b8462.0p-141L,
     0xe80000.0p-24,  0xc99af3.0p-27, -0x1535b3ba8f150ae09996d7bb4653e.0p-143L,
     0xe70000.0p-24,  0xd273b2.0p-27,  0x163786f5251aefe0ded34c8318f52.0p-145L,
     0xe50000.0p-24,  0xe442c0.0p-27,  0x1bc4b2368e32d56699c1799a244d4.0p-144L,
     0xe38000.0p-24,  0xf1b83f.0p-27,  0x1c6090f684e6766abceccab1d7174.0p-141L,
     0xe20000.0p-24,  0xff448a.0p-27, -0x1890aa69ac9f4215f93936b709efb.0p-142L,
     0xe08000.0p-24,  0x8673f6.0p-26,  0x1b9985194b6affd511b534b72a28e.0p-140L,
     0xdf0000.0p-24,  0x8d515c.0p-26, -0x1dc08d61c6ef1d9b2ef7e68680598.0p-143L,
     0xdd8000.0p-24,  0x943a9e.0p-26, -0x1f72a2dac729b3f46662238a9425a.0p-142L,
     0xdc0000.0p-24,  0x9b2fe6.0p-26, -0x1fd4dfd3a0afb9691aed4d5e3df94.0p-140L,
     0xda8000.0p-24,  0xa2315d.0p-26, -0x11b26121629c46c186384993e1c93.0p-142L,
     0xd90000.0p-24,  0xa93f2f.0p-26,  0x1286d633e8e5697dc6a402a56fce1.0p-141L,
     0xd78000.0p-24,  0xb05988.0p-26,  0x16128eba9367707ebfa540e45350c.0p-144L,
     0xd60000.0p-24,  0xb78094.0p-26,  0x16ead577390d31ef0f4c9d43f79b2.0p-140L,
     0xd50000.0p-24,  0xbc4c6c.0p-26,  0x151131ccf7c7b75e7d900b521c48d.0p-141L,
     0xd38000.0p-24,  0xc3890a.0p-26, -0x115e2cd714bd06508aeb00d2ae3e9.0p-140L,
     0xd20000.0p-24,  0xcad2d7.0p-26, -0x1847f406ebd3af80485c2f409633c.0p-142L,
     0xd10000.0p-24,  0xcfb620.0p-26,  0x1c2259904d686581799fbce0b5f19.0p-141L,
     0xcf8000.0p-24,  0xd71653.0p-26,  0x1ece57a8d5ae54f550444ecf8b995.0p-140L,
     0xce0000.0p-24,  0xde843a.0p-26, -0x1f109d4bc4595412b5d2517aaac13.0p-141L,
     0xcd0000.0p-24,  0xe37fde.0p-26,  0x1bc03dc271a74d3a85b5b43c0e727.0p-141L,
     0xcb8000.0p-24,  0xeb050c.0p-26, -0x1bf2badc0df841a71b79dd5645b46.0p-145L,
     0xca0000.0p-24,  0xf29878.0p-26, -0x18efededd89fbe0bcfbe6d6db9f66.0p-147L,
     0xc90000.0p-24,  0xf7ad6f.0p-26,  0x1373ff977baa6911c7bafcb4d84fb.0p-141L,
     0xc80000.0p-24,  0xfcc8e3.0p-26,  0x196766f2fb328337cc050c6d83b22.0p-140L,
     0xc68000.0p-24,  0x823f30.0p-25,  0x19bd076f7c434e5fcf1a212e2a91e.0p-139L,
     0xc58000.0p-24,  0x84d52c.0p-25, -0x1a327257af0f465e5ecab5f2a6f81.0p-139L,
     0xc40000.0p-24,  0x88bc74.0p-25,  0x113f23def19c5a0fe396f40f1dda9.0p-141L,
     0xc30000.0p-24,  0x8b5ae6.0p-25,  0x1759f6e6b37de945a049a962e66c6.0p-139L,
     0xc20000.0p-24,  0x8dfccb.0p-25,  0x1ad35ca6ed5147bdb6ddcaf59c425.0p-141L,
     0xc10000.0p-24,  0x90a22b.0p-25,  0x1a1d71a87deba46bae9827221dc98.0p-139L,
     0xbf8000.0p-24,  0x94a0d8.0p-25, -0x139e5210c2b730e28aba001a9b5e0.0p-140L,
     0xbe8000.0p-24,  0x974f16.0p-25, -0x18f6ebcff3ed72e23e13431adc4a5.0p-141L,
     0xbd8000.0p-24,  0x9a00f1.0p-25, -0x1aa268be39aab7148e8d80caa10b7.0p-139L,
     0xbc8000.0p-24,  0x9cb672.0p-25, -0x14c8815839c5663663d15faed7771.0p-139L,
     0xbb0000.0p-24,  0xa0cda1.0p-25,  0x1eaf46390dbb2438273918db7df5c.0p-141L,
     0xba0000.0p-24,  0xa38c6e.0p-25,  0x138e20d831f698298adddd7f32686.0p-141L,
     0xb90000.0p-24,  0xa64f05.0p-25, -0x1e8d3c41123615b147a5d47bc208f.0p-142L,
     0xb80000.0p-24,  0xa91570.0p-25,  0x1ce28f5f3840b263acb4351104631.0p-140L,
     0xb70000.0p-24,  0xabdfbb.0p-25, -0x186e5c0a42423457e22d8c650b355.0p-139L,
     0xb60000.0p-24,  0xaeadef.0p-25, -0x14d41a0b2a08a465dc513b13f567d.0p-143L,
     0xb50000.0p-24,  0xb18018.0p-25,  0x16755892770633947ffe651e7352f.0p-139L,
     0xb40000.0p-24,  0xb45642.0p-25, -0x16395ebe59b15228bfe8798d10ff0.0p-142L,
     0xb30000.0p-24,  0xb73077.0p-25,  0x1abc65c8595f088b61a335f5b688c.0p-140L,
     0xb20000.0p-24,  0xba0ec4.0p-25, -0x1273089d3dad88e7d353e9967d548.0p-139L,
     0xb10000.0p-24,  0xbcf133.0p-25,  0x10f9f67b1f4bbf45de06ecebfaf6d.0p-139L,
     0xb00000.0p-24,  0xbfd7d2.0p-25, -0x109fab904864092b34edda19a831e.0p-140L,
     0xaf0000.0p-24,  0xc2c2ac.0p-25, -0x1124680aa43333221d8a9b475a6ba.0p-139L,
     0xae8000.0p-24,  0xc439b3.0p-25, -0x1f360cc4710fbfe24b633f4e8d84d.0p-140L,
     0xad8000.0p-24,  0xc72afd.0p-25, -0x132d91f21d89c89c45003fc5d7807.0p-140L,
     0xac8000.0p-24,  0xca20a2.0p-25, -0x16bf9b4d1f8da8002f2449e174504.0p-139L,
     0xab8000.0p-24,  0xcd1aae.0p-25,  0x19deb5ce6a6a8717d5626e16acc7d.0p-141L,
     0xaa8000.0p-24,  0xd0192f.0p-25,  0x1a29fb48f7d3ca87dabf351aa41f4.0p-139L,
     0xaa0000.0p-24,  0xd19a20.0p-25,  0x1127d3c6457f9d79f51dcc73014c9.0p-141L,
     0xa90000.0p-24,  0xd49f6a.0p-25, -0x1ba930e486a0ac42d1bf9199188e7.0p-141L,
     0xa80000.0p-24,  0xd7a94b.0p-25, -0x1b6e645f31549dd1160bcc45c7e2c.0p-139L,
     0xa70000.0p-24,  0xdab7d0.0p-25,  0x1118a425494b610665377f15625b6.0p-140L,
     0xa68000.0p-24,  0xdc40d5.0p-25,  0x1966f24d29d3a2d1b2176010478be.0p-140L,
     0xa58000.0p-24,  0xdf566d.0p-25, -0x1d8e52eb2248f0c95dd83626d7333.0p-142L,
     0xa48000.0p-24,  0xe270ce.0p-25, -0x1ee370f96e6b67ccb006a5b9890ea.0p-140L,
     0xa40000.0p-24,  0xe3ffce.0p-25,  0x1d155324911f56db28da4d629d00a.0p-140L,
     0xa30000.0p-24,  0xe72179.0p-25, -0x1fe6e2f2f867d8f4d60c713346641.0p-140L,
     0xa20000.0p-24,  0xea4812.0p-25,  0x1b7be9add7f4d3b3d406b6cbf3ce5.0p-140L,
     0xa18000.0p-24,  0xebdd3d.0p-25,  0x1b3cfb3f7511dd73692609040ccc2.0p-139L,
     0xa08000.0p-24,  0xef0b5b.0p-25, -0x1220de1f7301901b8ad85c25afd09.0p-139L,
     0xa00000.0p-24,  0xf0a451.0p-25, -0x176364c9ac81cc8a4dfb804de6867.0p-140L,
     0x9f0000.0p-24,  0xf3da16.0p-25,  0x1eed6b9aafac8d42f78d3e65d3727.0p-141L,
     0x9e8000.0p-24,  0xf576e9.0p-25,  0x1d593218675af269647b783d88999.0p-139L,
     0x9d8000.0p-24,  0xf8b47c.0p-25, -0x13e8eb7da053e063714615f7cc91d.0p-144L,
     0x9d0000.0p-24,  0xfa553f.0p-25,  0x1c063259bcade02951686d5373aec.0p-139L,
     0x9c0000.0p-24,  0xfd9ac5.0p-25,  0x1ef491085fa3c1649349630531502.0p-139L,
     0x9b8000.0p-24,  0xff3f8c.0p-25,  0x1d607a7c2b8c5320619fb9433d841.0p-139L,
     0x9a8000.0p-24,  0x814697.0p-24, -0x12ad3817004f3f0bdff99f932b273.0p-138L,
     0x9a0000.0p-24,  0x821b06.0p-24, -0x189fc53117f9e54e78103a2bc1767.0p-141L,
     0x990000.0p-24,  0x83c5f8.0p-24,  0x14cf15a048907b7d7f47ddb45c5a3.0p-139L,
     0x988000.0p-24,  0x849c7d.0p-24,  0x1cbb1d35fb82873b04a9af1dd692c.0p-138L,
     0x978000.0p-24,  0x864ba6.0p-24,  0x1128639b814f9b9770d8cb6573540.0p-138L,
     0x970000.0p-24,  0x87244c.0p-24,  0x184733853300f002e836dfd47bd41.0p-139L,
     0x968000.0p-24,  0x87fdaa.0p-24,  0x109d23aef77dd5cd7cc94306fb3ff.0p-140L,
     0x958000.0p-24,  0x89b293.0p-24, -0x1a81ef367a59de2b41eeebd550702.0p-138L,
     0x950000.0p-24,  0x8a8e20.0p-24, -0x121ad3dbb2f45275c917a30df4ac9.0p-138L,
     0x948000.0p-24,  0x8b6a6a.0p-24, -0x1cfb981628af71a89df4e6df2e93b.0p-139L,
     0x938000.0p-24,  0x8d253a.0p-24, -0x1d21730ea76cfdec367828734cae5.0p-139L,
     0x930000.0p-24,  0x8e03c2.0p-24,  0x135cc00e566f76b87333891e0dec4.0p-138L,
     0x928000.0p-24,  0x8ee30d.0p-24, -0x10fcb5df257a263e3bf446c6e3f69.0p-140L,
     0x918000.0p-24,  0x90a3ee.0p-24, -0x16e171b15433d723a4c7380a448d8.0p-139L,
     0x910000.0p-24,  0x918587.0p-24, -0x1d050da07f3236f330972da2a7a87.0p-139L,
     0x908000.0p-24,  0x9267e7.0p-24,  0x1be03669a5268d21148c6002becd3.0p-139L,
     0x8f8000.0p-24,  0x942f04.0p-24,  0x10b28e0e26c336af90e00533323ba.0p-139L,
     0x8f0000.0p-24,  0x9513c3.0p-24,  0x1a1d820da57cf2f105a89060046aa.0p-138L,
     0x8e8000.0p-24,  0x95f950.0p-24, -0x19ef8f13ae3cf162409d8ea99d4c0.0p-139L,
     0x8e0000.0p-24,  0x96dfab.0p-24, -0x109e417a6e507b9dc10dac743ad7a.0p-138L,
     0x8d0000.0p-24,  0x98aed2.0p-24,  0x10d01a2c5b0e97c4990b23d9ac1f5.0p-139L,
     0x8c8000.0p-24,  0x9997a2.0p-24, -0x1d6a50d4b61ea74540bdd2aa99a42.0p-138L,
     0x8c0000.0p-24,  0x9a8145.0p-24,  0x1b3b190b83f9527e6aba8f2d783c1.0p-138L,
     0x8b8000.0p-24,  0x9b6bbf.0p-24,  0x13a69fad7e7abe7ba81c664c107e0.0p-138L,
     0x8b0000.0p-24,  0x9c5711.0p-24, -0x11cd12316f576aad348ae79867223.0p-138L,
     0x8a8000.0p-24,  0x9d433b.0p-24,  0x1c95c444b807a246726b304ccae56.0p-139L,
     0x898000.0p-24,  0x9f1e22.0p-24, -0x1b9c224ea698c2f9b47466d6123fe.0p-139L,
     0x890000.0p-24,  0xa00ce1.0p-24,  0x125ca93186cf0f38b4619a2483399.0p-141L,
     0x888000.0p-24,  0xa0fc80.0p-24, -0x1ee38a7bc228b3597043be78eaf49.0p-139L,
     0x880000.0p-24,  0xa1ed00.0p-24, -0x1a0db876613d204147dc69a07a649.0p-138L,
     0x878000.0p-24,  0xa2de62.0p-24,  0x193224e8516c008d3602a7b41c6e8.0p-139L,
     0x870000.0p-24,  0xa3d0a9.0p-24,  0x1fa28b4d2541aca7d5844606b2421.0p-139L,
     0x868000.0p-24,  0xa4c3d6.0p-24,  0x1c1b5760fb4571acbcfb03f16daf4.0p-138L,
     0x858000.0p-24,  0xa6acea.0p-24,  0x1fed5d0f65949c0a345ad743ae1ae.0p-140L,
     0x850000.0p-24,  0xa7a2d4.0p-24,  0x1ad270c9d749362382a7688479e24.0p-140L,
     0x848000.0p-24,  0xa899ab.0p-24,  0x199ff15ce532661ea9643a3a2d378.0p-139L,
     0x840000.0p-24,  0xa99171.0p-24,  0x1a19e15ccc45d257530a682b80490.0p-139L,
     0x838000.0p-24,  0xaa8a28.0p-24, -0x121a14ec532b35ba3e1f868fd0b5e.0p-140L,
     0x830000.0p-24,  0xab83d1.0p-24,  0x1aee319980bff3303dd481779df69.0p-139L,
     0x828000.0p-24,  0xac7e6f.0p-24, -0x18ffd9e3900345a85d2d86161742e.0p-140L,
     0x820000.0p-24,  0xad7a03.0p-24, -0x1e4db102ce29f79b026b64b42caa1.0p-140L,
     0x818000.0p-24,  0xae768f.0p-24,  0x17c35c55a04a82ab19f77652d977a.0p-141L,
     0x810000.0p-24,  0xaf7415.0p-24,  0x1448324047019b48d7b98c1cf7234.0p-138L,
     0x808000.0p-24,  0xb07298.0p-24, -0x1750ee3915a197e9c7359dd94152f.0p-138L,
     0x800000.0p-24,  0xb17218.0p-24, -0x105c610ca86c3898cff81a12a17e2.0p-141L,
};

#ifdef USE_UTAB
static const struct {
	float	H;			/* 1 + i/INTERVALS (exact) */
	float	E;			/* H(i) * G(i) - 1 (exact) */
} U[TSIZE] = {
	 0x800000.0p-23,  0,
	 0x810000.0p-23, -0x800000.0p-37,
	 0x820000.0p-23, -0x800000.0p-35,
	 0x830000.0p-23, -0x900000.0p-34,
	 0x840000.0p-23, -0x800000.0p-33,
	 0x850000.0p-23, -0xc80000.0p-33,
	 0x860000.0p-23, -0xa00000.0p-36,
	 0x870000.0p-23,  0x940000.0p-33,
	 0x880000.0p-23,  0x800000.0p-35,
	 0x890000.0p-23, -0xc80000.0p-34,
	 0x8a0000.0p-23,  0xe00000.0p-36,
	 0x8b0000.0p-23,  0x900000.0p-33,
	 0x8c0000.0p-23, -0x800000.0p-35,
	 0x8d0000.0p-23, -0xe00000.0p-33,
	 0x8e0000.0p-23,  0x880000.0p-33,
	 0x8f0000.0p-23, -0xa80000.0p-34,
	 0x900000.0p-23, -0x800000.0p-35,
	 0x910000.0p-23,  0x800000.0p-37,
	 0x920000.0p-23,  0x900000.0p-35,
	 0x930000.0p-23,  0xd00000.0p-35,
	 0x940000.0p-23,  0xe00000.0p-35,
	 0x950000.0p-23,  0xc00000.0p-35,
	 0x960000.0p-23,  0xe00000.0p-36,
	 0x970000.0p-23, -0x800000.0p-38,
	 0x980000.0p-23, -0xc00000.0p-35,
	 0x990000.0p-23, -0xd00000.0p-34,
	 0x9a0000.0p-23,  0x880000.0p-33,
	 0x9b0000.0p-23,  0xe80000.0p-35,
	 0x9c0000.0p-23, -0x800000.0p-35,
	 0x9d0000.0p-23,  0xb40000.0p-33,
	 0x9e0000.0p-23,  0x880000.0p-34,
	 0x9f0000.0p-23, -0xe00000.0p-35,
	 0xa00000.0p-23,  0x800000.0p-33,
	 0xa10000.0p-23, -0x900000.0p-36,
	 0xa20000.0p-23, -0xb00000.0p-33,
	 0xa30000.0p-23, -0xa00000.0p-36,
	 0xa40000.0p-23,  0x800000.0p-33,
	 0xa50000.0p-23, -0xf80000.0p-35,
	 0xa60000.0p-23,  0x880000.0p-34,
	 0xa70000.0p-23, -0x900000.0p-33,
	 0xa80000.0p-23, -0x800000.0p-35,
	 0xa90000.0p-23,  0x900000.0p-34,
	 0xaa0000.0p-23,  0xa80000.0p-33,
	 0xab0000.0p-23, -0xac0000.0p-34,
	 0xac0000.0p-23, -0x800000.0p-37,
	 0xad0000.0p-23,  0xf80000.0p-35,
	 0xae0000.0p-23,  0xf80000.0p-34,
	 0xaf0000.0p-23, -0xac0000.0p-33,
	 0xb00000.0p-23, -0x800000.0p-33,
	 0xb10000.0p-23, -0xb80000.0p-34,
	 0xb20000.0p-23, -0x800000.0p-34,
	 0xb30000.0p-23, -0xb00000.0p-35,
	 0xb40000.0p-23, -0x800000.0p-35,
	 0xb50000.0p-23, -0xe00000.0p-36,
	 0xb60000.0p-23, -0x800000.0p-35,
	 0xb70000.0p-23, -0xb00000.0p-35,
	 0xb80000.0p-23, -0x800000.0p-34,
	 0xb90000.0p-23, -0xb80000.0p-34,
	 0xba0000.0p-23, -0x800000.0p-33,
	 0xbb0000.0p-23, -0xac0000.0p-33,
	 0xbc0000.0p-23,  0x980000.0p-33,
	 0xbd0000.0p-23,  0xbc0000.0p-34,
	 0xbe0000.0p-23,  0xe00000.0p-36,
	 0xbf0000.0p-23, -0xb80000.0p-35,
	 0xc00000.0p-23, -0x800000.0p-33,
	 0xc10000.0p-23,  0xa80000.0p-33,
	 0xc20000.0p-23,  0x900000.0p-34,
	 0xc30000.0p-23, -0x800000.0p-35,
	 0xc40000.0p-23, -0x900000.0p-33,
	 0xc50000.0p-23,  0x820000.0p-33,
	 0xc60000.0p-23,  0x800000.0p-38,
	 0xc70000.0p-23, -0x820000.0p-33,
	 0xc80000.0p-23,  0x800000.0p-33,
	 0xc90000.0p-23, -0xa00000.0p-36,
	 0xca0000.0p-23, -0xb00000.0p-33,
	 0xcb0000.0p-23,  0x840000.0p-34,
	 0xcc0000.0p-23, -0xd00000.0p-34,
	 0xcd0000.0p-23,  0x800000.0p-33,
	 0xce0000.0p-23, -0xe00000.0p-35,
	 0xcf0000.0p-23,  0xa60000.0p-33,
	 0xd00000.0p-23, -0x800000.0p-35,
	 0xd10000.0p-23,  0xb40000.0p-33,
	 0xd20000.0p-23, -0x800000.0p-35,
	 0xd30000.0p-23,  0xaa0000.0p-33,
	 0xd40000.0p-23, -0xe00000.0p-35,
	 0xd50000.0p-23,  0x880000.0p-33,
	 0xd60000.0p-23, -0xd00000.0p-34,
	 0xd70000.0p-23,  0x9c0000.0p-34,
	 0xd80000.0p-23, -0xb00000.0p-33,
	 0xd90000.0p-23, -0x800000.0p-38,
	 0xda0000.0p-23,  0xa40000.0p-33,
	 0xdb0000.0p-23, -0xdc0000.0p-34,
	 0xdc0000.0p-23,  0xc00000.0p-35,
	 0xdd0000.0p-23,  0xca0000.0p-33,
	 0xde0000.0p-23, -0xb80000.0p-34,
	 0xdf0000.0p-23,  0xd00000.0p-35,
	 0xe00000.0p-23,  0xc00000.0p-33,
	 0xe10000.0p-23, -0xf40000.0p-34,
	 0xe20000.0p-23,  0x800000.0p-37,
	 0xe30000.0p-23,  0x860000.0p-33,
	 0xe40000.0p-23, -0xc80000.0p-33,
	 0xe50000.0p-23, -0xa80000.0p-34,
	 0xe60000.0p-23,  0xe00000.0p-36,
	 0xe70000.0p-23,  0x880000.0p-33,
	 0xe80000.0p-23, -0xe00000.0p-33,
	 0xe90000.0p-23, -0xfc0000.0p-34,
	 0xea0000.0p-23, -0x800000.0p-35,
	 0xeb0000.0p-23,  0xe80000.0p-35,
	 0xec0000.0p-23,  0x900000.0p-33,
	 0xed0000.0p-23,  0xe20000.0p-33,
	 0xee0000.0p-23, -0xac0000.0p-33,
	 0xef0000.0p-23, -0xc80000.0p-34,
	 0xf00000.0p-23, -0x800000.0p-35,
	 0xf10000.0p-23,  0x800000.0p-35,
	 0xf20000.0p-23,  0xb80000.0p-34,
	 0xf30000.0p-23,  0x940000.0p-33,
	 0xf40000.0p-23,  0xc80000.0p-33,
	 0xf50000.0p-23, -0xf20000.0p-33,
	 0xf60000.0p-23, -0xc80000.0p-33,
	 0xf70000.0p-23, -0xa20000.0p-33,
	 0xf80000.0p-23, -0x800000.0p-33,
	 0xf90000.0p-23, -0xc40000.0p-34,
	 0xfa0000.0p-23, -0x900000.0p-34,
	 0xfb0000.0p-23, -0xc80000.0p-35,
	 0xfc0000.0p-23, -0x800000.0p-35,
	 0xfd0000.0p-23, -0x900000.0p-36,
	 0xfe0000.0p-23, -0x800000.0p-37,
	 0xff0000.0p-23, -0x800000.0p-39,
	 0x800000.0p-22,  0,
};
#endif /* USE_UTAB */

#ifdef STRUCT_RETURN
#define	RETURN1(rp, v) do {	\
	(rp)->hi = (v);		\
	(rp)->lo_set = 0;	\
	return;			\
} while (0)

#define	RETURN2(rp, h, l) do {	\
	(rp)->hi = (h);		\
	(rp)->lo = (l);		\
	(rp)->lo_set = 1;	\
	return;			\
} while (0)

struct ld {
	long double hi;
	long double lo;
	int	lo_set;
};
#else
#define	RETURN1(rp, v)	RETURNF(v)
#define	RETURN2(rp, h, l)	RETURNI((h) + (l))
#endif

#ifdef STRUCT_RETURN
static __always_inline void
k_logl(long double x, struct ld *rp)
#else
long double
ref_logl(long double x)
#endif
{
	long double d, val_hi, val_lo;
	double dd, dk;
	uint64_t lx, llx;
	int i, k;
	uint16_t hx;

	EXTRACT_LDBL128_WORDS(hx, lx, llx, x);
	k = -16383;
#if 0 /* Hard to do efficiently.  Don't do it until we support all modes. */
	if (x == 1)
		RETURN1(rp, 0);		/* log(1) = +0 in all rounding modes */
#endif
	if (hx == 0 || hx >= 0x8000) {	/* zero, negative or subnormal? */
		if (((hx & 0x7fff) | lx | llx) == 0)
			RETURN1(rp, -1 / zero);	/* log(+-0) = -Inf */
		if (hx != 0)
			/* log(neg or NaN) = qNaN: */
			RETURN1(rp, (x - x) / zero);
		x *= 0x1.0p113;		/* subnormal; scale up x */
		EXTRACT_LDBL128_WORDS(hx, lx, llx, x);
		k = -16383 - 113;
	} else if (hx >= 0x7fff)
		RETURN1(rp, x + x);	/* log(Inf or NaN) = Inf or qNaN */
#ifndef STRUCT_RETURN
	ENTERI();
#endif
	k += hx;
	dk = k;

	/* Scale x to be in [1, 2). */
	SET_LDBL_EXPSIGN(x, 0x3fff);

	/* 0 <= i <= INTERVALS: */
#define	L2I	(49 - LOG2_INTERVALS)
	i = (lx + (1LL << (L2I - 2))) >> (L2I - 1);

	/*
	 * -0.005280 < d < 0.004838.  In particular, the infinite-
	 * precision |d| is <= 2**-7.  Rounding of G(i) to 8 bits
	 * ensures that d is representable without extra precision for
	 * this bound on |d| (since when this calculation is expressed
	 * as x*G(i)-1, the multiplication needs as many extra bits as
	 * G(i) has and the subtraction cancels 8 bits).  But for
	 * most i (107 cases out of 129), the infinite-precision |d|
	 * is <= 2**-8.  G(i) is rounded to 9 bits for such i to give
	 * better accuracy (this works by improving the bound on |d|,
	 * which in turn allows rounding to 9 bits in more cases).
	 * This is only important when the original x is near 1 -- it
	 * lets us avoid using a special method to give the desired
	 * accuracy for such x.
	 */
	if (0)
		d = x * G(i) - 1;
	else {
#ifdef USE_UTAB
		d = (x - H(i)) * G(i) + E(i);
#else
		long double x_hi;
		double x_lo;

		/*
		 * Split x into x_hi + x_lo to calculate x*G(i)-1 exactly.
		 * G(i) has at most 9 bits, so the splitting point is not
		 * critical.
		 */
		INSERT_LDBL128_WORDS(x_hi, 0x3fff, lx,
		    llx & 0xffffffffff000000ULL);
		x_lo = x - x_hi;
		d = x_hi * G(i) - 1 + x_lo * G(i);
#endif
	}

	/*
	 * Our algorithm depends on exact cancellation of F_lo(i) and
	 * F_hi(i) with dk*ln_2_lo and dk*ln2_hi when k is -1 and i is
	 * at the end of the table.  This and other technical complications
	 * make it difficult to avoid the double scaling in (dk*ln2) *
	 * log(base) for base != e without losing more accuracy and/or
	 * efficiency than is gained.
	 */
	/*
	 * Use double precision operations wherever possible, since
	 * long double operations are emulated and were very slow on
	 * the old sparc64 and unknown on the newer aarch64 and riscv
	 * machines.  Also, don't try to improve parallelism by
	 * increasing the number of operations, since any parallelism
	 * on such machines is needed for the emulation.  Horner's
	 * method is good for this, and is also good for accuracy.
	 * Horner's method doesn't handle the `lo' term well, either
	 * for efficiency or accuracy.  However, for accuracy we
	 * evaluate d * d * P2 separately to take advantage of by P2
	 * being exact, and this gives a good place to sum the 'lo'
	 * term too.
	 */
	dd = (double)d;
	val_lo = d * d * d * (P3 +
	    d * (P4 + d * (P5 + d * (P6 + d * (P7 + d * (P8 +
	    dd * (P9 + dd * (P10 + dd * (P11 + dd * (P12 + dd * (P13 +
	    dd * P14))))))))))) + (F_lo(i) + dk * ln2_lo) + d * d * P2;
	val_hi = d;
	_3sumF(val_hi, val_lo, F_hi(i) + dk * ln2_hi);
	RETURN2(rp, val_hi, val_lo);
}

long double
ref_log1pl(long double x)
{
	long double d, d_hi, f_lo, val_hi, val_lo;
	long double f_hi, twopminusk;
	double d_lo, dd, dk;
	uint64_t lx, llx;
	int i, k;
	int16_t ax, hx;

	EXTRACT_LDBL128_WORDS(hx, lx, llx, x);
	if (hx < 0x3fff) {		/* x < 1, or x neg NaN */
		ax = hx & 0x7fff;
		if (ax >= 0x3fff) {	/* x <= -1, or x neg NaN */
			if (ax == 0x3fff && (lx | llx) == 0)
				RETURNF(-1 / zero);	/* log1p(-1) = -Inf */
			/* log1p(x < 1, or x NaN) = qNaN: */
			RETURNF((x - x) / (x - x));
		}
		if (ax <= 0x3f8d) {	/* |x| < 2**-113 */
			if ((int)x == 0)
				RETURNF(x);	/* x with inexact if x != 0 */
		}
		f_hi = 1;
		f_lo = x;
	} else if (hx >= 0x7fff) {	/* x +Inf or non-neg NaN */
		RETURNF(x + x);		/* log1p(Inf or NaN) = Inf or qNaN */
	} else if (hx < 0x40e1) {	/* 1 <= x < 2**226 */
		f_hi = x;
		f_lo = 1;
	} else {			/* 2**226 <= x < +Inf */
		f_hi = x;
		f_lo = 0;		/* avoid underflow of the P3 term */
	}
	ENTERI();
	x = f_hi + f_lo;
	f_lo = (f_hi - x) + f_lo;

	EXTRACT_LDBL128_WORDS(hx, lx, llx, x);
	k = -16383;

	k += hx;
	dk = k;

	SET_LDBL_EXPSIGN(x, 0x3fff);
	twopminusk = 1;
	SET_LDBL_EXPSIGN(twopminusk, 0x7ffe - (hx & 0x7fff));
	f_lo *= twopminusk;

	i = (lx + (1LL << (L2I - 2))) >> (L2I - 1);

	/*
	 * x*G(i)-1 (with a reduced x) can be represented exactly, as
	 * above, but now we need to evaluate the polynomial on d =
	 * (x+f_lo)*G(i)-1 and extra precision is needed for that.
	 * Since x+x_lo is a hi+lo decomposition and subtracting 1
	 * doesn't lose too many bits, an inexact calculation for
	 * f_lo*G(i) is good enough.
	 */
	if (0)
		d_hi = x * G(i) - 1;
	else {
#ifdef USE_UTAB
		d_hi = (x - H(i)) * G(i) + E(i);
#else
		long double x_hi;
		double x_lo;

		INSERT_LDBL128_WORDS(x_hi, 0x3fff, lx,
		    llx & 0xffffffffff000000ULL);
		x_lo = x - x_hi;
		d_hi = x_hi * G(i) - 1 + x_lo * G(i);
#endif
	}
	d_lo = f_lo * G(i);

	/*
	 * This is _2sumF(d_hi, d_lo) inlined.  The condition
	 * (d_hi == 0 || |d_hi| >= |d_lo|) for using _2sumF() is not
	 * always satisifed, so it is not clear that this works, but
	 * it works in practice.  It works even if it gives a wrong
	 * normalized d_lo, since |d_lo| > |d_hi| implies that i is
	 * nonzero and d is tiny, so the F(i) term dominates d_lo.
	 * In float precision:
	 * (By exhaustive testing, the worst case is d_hi = 0x1.bp-25.
	 * And if d is only a little tinier than that, we would have
	 * another underflow problem for the P3 term; this is also ruled
	 * out by exhaustive testing.)
	 */
	d = d_hi + d_lo;
	d_lo = d_hi - d + d_lo;
	d_hi = d;

	dd = (double)d;
	val_lo = d * d * d * (P3 +
	    d * (P4 + d * (P5 + d * (P6 + d * (P7 + d * (P8 +
	    dd * (P9 + dd * (P10 + dd * (P11 + dd * (P12 + dd * (P13 +
	    dd * P14))))))))))) + (F_lo(i) + dk * ln2_lo + d_lo) + d * d * P2;
	val_hi = d_hi;
	_3sumF(val_hi, val_lo, F_hi(i) + dk * ln2_hi);
	RETURNI(val_hi + val_lo);
}

#ifdef STRUCT_RETURN

long double
ref_logl(long double x)
{
	struct ld r;

	ENTERI();
	k_logl(x, &r);
	RETURNSPI(&r);
}

/*
 * 29+113 bit decompositions.  The bits are distributed so that the products
 * of the hi terms are exact in double precision.  The types are chosen so
 * that the products of the hi terms are done in at least double precision,
 * without any explicit conversions.  More natural choices would require a
 * slow long double precision multiplication.
 */
static const double
invln10_hi =  4.3429448176175356e-1,		/*  0x1bcb7b15000000.0p-54 */
invln2_hi =  1.4426950402557850e0;		/*  0x17154765000000.0p-52 */
static const long double
invln10_lo =  1.41498268538580090791605082294397000e-10L,	/*  0x137287195355baaafad33dc323ee3.0p-145L */
invln2_lo =  6.33178418956604368501892137426645911e-10L,	/*  0x15c17f0bbbe87fed0691d3e88eb57.0p-143L */
invln10_lo_plus_hi = invln10_lo + invln10_hi,
invln2_lo_plus_hi = invln2_lo + invln2_hi;

long double
ref_log10l(long double x)
{
	struct ld r;
	long double hi, lo;

	ENTERI();
	k_logl(x, &r);
	if (!r.lo_set)
		RETURNI(r.hi);
	_2sumF(r.hi, r.lo);
	hi = (float)r.hi;
	lo = r.lo + (r.hi - hi);
	RETURNI(invln10_hi * hi + (invln10_lo_plus_hi * lo + invln10_lo * hi));
}

long double
ref_log2l(long double x)
{
	struct ld r;
	long double hi, lo;

	ENTERI();
	k_logl(x, &r);
	if (!r.lo_set)
		RETURNI(r.hi);
	_2sumF(r.hi, r.lo);
	hi = (float)r.hi;
	lo = r.lo + (r.hi - hi);
	RETURNI(invln2_hi * hi + (invln2_lo_plus_hi * lo + invln2_lo * hi));
}

#endif /* STRUCT_RETURN */

#undef P2
#undef P3
#undef P4
#undef P5
#undef P6
#undef P7
#undef P8
#undef P9
#undef P10
#undef P11
#undef P12
#undef P13
#undef P14
#undef zero
#undef T
#undef U
#undef invln10_hi
#undef invln2_hi
#undef invln10_lo
#undef invln2_lo
#undef invln10_lo_plus_hi
#undef invln2_lo_plus_hi

