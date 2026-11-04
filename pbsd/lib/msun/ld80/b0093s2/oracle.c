/*
 * PBSD batch b0093s2 -- reference oracle.
 *
 * The original HardenedBSD C source lib/msun/ld80/s_cospil.c concatenated
 * verbatim.  Every function is renamed with a "ref_" prefix.  The renaming
 * is done with the preprocessor so that no function body is modified in any
 * way: only the token that names the function is substituted, the body text
 * is untouched.
 *
 * Support code that s_cospil.c requires but that does not live in it (union
 * IEEEl2bits from lib/libc/amd64/_fpmath.h, macros from
 * lib/msun/src/math_private.h, and the kernels of lib/msun/ld80/k_sinl.c and
 * k_cosl.c) is reproduced verbatim first.  Only defines were added; no
 * function body was changed.
 */

#include <float.h>
#include <math.h>
#include <stdint.h>

#ifndef LONG_BIT
#define	LONG_BIT	(8 * (int)sizeof(long))
#endif

/* ------------------------------------------------------------------ */
/* from lib/libc/amd64/_fpmath.h					    */
/* ------------------------------------------------------------------ */

union IEEEl2bits {
	long double	e;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:32;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
		unsigned int	junkl	:16;
		unsigned int	junkh	:32;
	} bits;
	struct {
		unsigned long	man	:64;
		unsigned int	expsign	:16;
		unsigned long	junk	:48;
	} xbits;
};

#define	LDBL_NBIT	0x80000000
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)

#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32

/* ------------------------------------------------------------------ */
/* from lib/msun/src/math_private.h				    */
/* ------------------------------------------------------------------ */

typedef union {
	float value;
	uint32_t word;
} ieee_float_shape_type;

#define SET_FLOAT_WORD(d,i)					\
do {								\
  ieee_float_shape_type sf_u;					\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)

#define	EXTRACT_LDBL80_WORDS(ix0,ix1,d)				\
do {								\
  union IEEEl2bits ew_u;					\
  ew_u.e = (d);							\
  (ix0) = ew_u.xbits.expsign;					\
  (ix1) = ew_u.xbits.man;					\
} while (0)

#define	INSERT_LDBL80_WORDS(d,ix0,ix1)				\
do {								\
  union IEEEl2bits iw_u;					\
  iw_u.xbits.expsign = (ix0);					\
  iw_u.xbits.man = (ix1);					\
  (d) = iw_u.e;							\
} while (0)

#define	SET_LDBL_EXPSIGN(d,v)					\
do {								\
  union IEEEl2bits se_u;					\
  se_u.e = (d);							\
  se_u.xbits.expsign = (v);					\
  (d) = se_u.e;							\
} while (0)

/* Support switching the mode to FP_PE if necessary; not __i386__ here. */
#define	ENTERI()
#define	ENTERIT(x)
#define	RETURNI(x)	RETURNF(x)
#define	ENTERV()
#define	RETURNV()	return

/* Default return statement if hack*_t() is not used. */
#define      RETURNF(v)      return (v)

#define	_2sumF(a, b) do {	\
	__typeof(a) __w;	\
				\
	__w = (a) + (b);	\
	(b) = ((a) - __w) + (b); \
	(a) = __w;		\
} while (0)

#define	FFLOORL80(x, j0, ix, lx) do {			\
	j0 = ix - 0x3fff + 1;				\
	if ((j0) < 32) {				\
		(lx) = ((lx) >> 32) << 32;		\
		(lx) &= ~((((lx) << 32)-1) >> (j0));	\
	} else {					\
		uint64_t _m;				\
		_m = (uint64_t)-1 >> (j0);		\
		if ((lx) & _m) (lx) &= ~_m;		\
	}						\
	INSERT_LDBL80_WORDS((x), (ix), (lx));		\
} while (0)

/* ------------------------------------------------------------------ */
/* from lib/msun/ld80/k_sinl.c					    */
/* ------------------------------------------------------------------ */

static const double
half =  0.5;

#if defined(__amd64__) || defined(__i386__)
/* Long double constants are slow on these arches, and broken on i386. */
static const volatile double
S1hi = -0.16666666666666666,		/* -0x15555555555555.0p-55 */
S1lo = -9.2563760475949941e-18;		/* -0x15580000000000.0p-109 */
#define	S1	((long double)S1hi + S1lo)
#else
static const long double
S1 = -0.166666666666666666671L;		/* -0xaaaaaaaaaaaaaaab.0p-66 */
#endif

static const double
S2 =  0.0083333333333333332,		/*  0x11111111111111.0p-59 */
S3 = -0.00019841269841269427,		/* -0x1a01a01a019f81.0p-65 */
S4 =  0.0000027557319223597490,		/*  0x171de3a55560f7.0p-71 */
S5 = -0.000000025052108218074604,	/* -0x1ae64564f16cad.0p-78 */
S6 =  1.6059006598854211e-10,		/*  0x161242b90243b5.0p-85 */
S7 = -7.6429779983024564e-13,		/* -0x1ae42ebd1b2e00.0p-93 */
S8 =  2.6174587166648325e-15;		/*  0x179372ea0b3f64.0p-101 */

long double
__kernel_sinl(long double x, long double y, int iy)
{
	long double z,r,v;

	z	=  x*x;
	v	=  z*x;
	r	=  S2+z*(S3+z*(S4+z*(S5+z*(S6+z*(S7+z*S8)))));
	if(iy==0) return x+v*(S1+z*r);
	else      return x-((z*(half*y-v*r)-y)-v*S1);
}

/* ------------------------------------------------------------------ */
/* from lib/msun/ld80/k_cosl.c					    */
/* ------------------------------------------------------------------ */

static const double
one = 1.0;

#if defined(__amd64__) || defined(__i386__)
/* Long double constants are slow on these arches, and broken on i386. */
static const volatile double
C1hi = 0.041666666666666664,		/*  0x15555555555555.0p-57 */
C1lo = 2.2598839032744733e-18;		/*  0x14d80000000000.0p-111 */
#define	C1	((long double)C1hi + C1lo)
#else
static const long double
C1 =  0.0416666666666666666136L;	/*  0xaaaaaaaaaaaaaa9b.0p-68 */
#endif

static const double
C2 = -0.0013888888888888874,		/* -0x16c16c16c16c10.0p-62 */
C3 =  0.000024801587301571716,		/*  0x1a01a01a018e22.0p-68 */
C4 = -0.00000027557319215507120,	/* -0x127e4fb7602f22.0p-74 */
C5 =  0.0000000020876754400407278,	/*  0x11eed8caaeccf1.0p-81 */
C6 = -1.1470297442401303e-11,		/* -0x19393412bd1529.0p-89 */
C7 =  4.7383039476436467e-14;		/*  0x1aac9d9af5c43e.0p-97 */

long double
__kernel_cosl(long double x, long double y)
{
	long double hz,z,r,w;

	z  = x*x;
	r  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*(C6+z*C7))))));
	hz = 0.5*z;
	w  = one-hz;
	return w + (((one-w)-hz) + (z*r-x*y));
}

/* ================================================================== */
/* lib/msun/ld80/s_cospil.c					      */
/* ================================================================== */

#define	pi_hi			ref_cospil_pi_hi
#define	pi_lo			ref_cospil_pi_lo
#define	__kernel_cospil		ref_cospil_kernel_cospil
#define	__kernel_sinpil		ref_cospil_kernel_sinpil
#define	vzero			ref_cospil_vzero
#define	cospil			ref_cospil

/*-
 * Copyright (c) 2017, 2023 Steven G. Kargl
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

/*
 * See ../src/s_cospi.c for implementation details.
 */

static const double
pi_hi = 3.1415926814079285e+00,	/* 0x400921fb 0x58000000 */
pi_lo =-2.7818135228334233e-08;	/* 0xbe5dde97 0x3dcb3b3a */

/* ---- lib/msun/ld80/k_cospil.h ----
 * See ../src/k_cospi.c for implementation details.
 */

static inline long double
__kernel_cospil(long double x)
{
	long double hi, lo;

	hi = (float)x;
	lo = x - hi;
	lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
	hi *= pi_hi;
	_2sumF(hi, lo);
	return (__kernel_cosl(hi, lo));
}

/* ---- lib/msun/ld80/k_sinpil.h ----
 * See ../src/k_sinpi.c for implementation details.
 */

static inline long double
__kernel_sinpil(long double x)
{
	long double hi, lo;

	hi = (float)x;
	lo = x - hi;
	lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
	hi *= pi_hi;
	_2sumF(hi, lo);
	return (__kernel_sinl(hi, lo, 1));
}

volatile static const double vzero = 0;

long double
cospil(long double x)
{
	long double ax, c;
	uint64_t lx, m;
	uint32_t j0;
	uint16_t hx, ix;

	EXTRACT_LDBL80_WORDS(hx, lx, x);
	ix = hx & 0x7fff;
	INSERT_LDBL80_WORDS(ax, ix, lx);

	ENTERI();

	if (ix < 0x3fff) {			/* |x| < 1 */
		if (ix < 0x3ffd) {		/* |x| < 0.25 */
			if (ix < 0x3fdd) {	/* |x| < 0x1p-34 */
				if ((int)x == 0)
					RETURNI(1);
			}
			RETURNI(__kernel_cospil(ax));
		}

		if (ix < 0x3ffe)			/* |x| < 0.5 */
			c = __kernel_sinpil(0.5 - ax);
		else if (lx < 0xc000000000000000ull) {	/* |x| < 0.75 */
			if (ax == 0.5)
				RETURNI(0);
			c = -__kernel_sinpil(ax - 0.5);
		} else
			c = -__kernel_cospil(1 - ax);
		RETURNI(c);
	}

	if (ix < 0x403e) {			/* 1 <= |x| < 0x1p63 */
		FFLOORL80(x, j0, ix, lx);	/* Integer part of ax. */
		ax -= x;
		EXTRACT_LDBL80_WORDS(ix, lx, ax);

		if (ix < 0x3ffe) {			/* |x| < 0.5 */
			if (ix < 0x3ffd)		/* |x| < 0.25 */
				c = ix == 0 ? 1 : __kernel_cospil(ax);
			else
				c = __kernel_sinpil(0.5 - ax);

		} else {
			if (lx < 0xc000000000000000ull) { /* |x| < 0.75 */
				if (ax == 0.5)
					RETURNI(0);
				c = -__kernel_sinpil(ax - 0.5);
			} else
				c = -__kernel_cospil(1 - ax);
		}

		if (j0 > 40)
			x -= 0x1p40;
		if (j0 > 30)
			x -= 0x1p30;
		j0 = (uint32_t)x;

		RETURNI(j0 & 1 ? -c : c);
	}

	if (ix >= 0x7fff)
		RETURNI(vzero / vzero);

	/*
	 * For 0x1p63 <= |x| < 0x1p64 need to determine if x is an even
	 * or odd integer to return t = +1 or -1.
	 * For |x| >= 0x1p64, it is always an even integer, so t = 1.
	 */
	RETURNI(ix >= 0x403f ? 1 : ((lx & 1) ? -1 : 1));
}

#undef pi_hi
#undef pi_lo
#undef __kernel_cospil
#undef __kernel_sinpil
#undef vzero
#undef cospil
