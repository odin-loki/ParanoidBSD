/*
 * PBSD batch b0093s4 -- reference oracle.
 *
 * This file is the SPECIFICATION for the port in port.cppm.  It contains the
 * original HardenedBSD C source of
 *
 *	lib/msun/ld80/s_tanpil.c
 *
 * with every function of the batch renamed with a "ref_" prefix.  Function
 * bodies are UNMODIFIED.
 *
 * In addition it carries the support material that the batch source pulls in
 * from headers and from other translation units of libm, because neither is
 * available when this file is compiled standalone:
 *
 *	- union IEEEl2bits			(lib/libc/amd64/_fpmath.h)
 *	- EXTRACT_LDBL80_WORDS, INSERT_LDBL80_WORDS,
 *	  ENTERI, RETURNI, RETURNF, _2sumF, FFLOORL80
 *						(lib/msun/src/math_private.h)
 *	- __kernel_tanl				(lib/msun/ld80/k_tanl.c)
 *
 * __kernel_tanl is deliberately NOT renamed: it is not part of this batch, it
 * is a shared dependency, and both the oracle and the port must call the very
 * same object code for the differential test to isolate the ported code.
 */

#include <stdint.h>
#include <math.h>

/*
 * ---------------------------------------------------------------------------
 * lib/libc/amd64/_fpmath.h
 * ---------------------------------------------------------------------------
 */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002, 2003 David Schultz <das@FreeBSD.ORG>
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

/*
 * ---------------------------------------------------------------------------
 * lib/msun/src/math_private.h  (the !__i386__ variants)
 * ---------------------------------------------------------------------------
 */

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

/*
 * ---------------------------------------------------------------------------
 * lib/msun/ld80/k_tanl.c  -- shared dependency, NOT part of this batch.
 * ---------------------------------------------------------------------------
 */

/*
 * ====================================================
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * ld80 version of k_tan.c.  See ../src/k_tan.c for most comments.
 */

/*
 * Domain [-0.67434, 0.67434], range ~[-2.25e-22, 1.921e-22]
 * |tan(x)/x - t(x)| < 2**-71.9
 *
 * See k_cosl.c for more details about the polynomial.
 */
#if defined(__amd64__) || defined(__i386__)
/* Long double constants are slow on these arches, and broken on i386. */
static const volatile double
T3hi =  0.33333333333333331,		/*  0x15555555555555.0p-54 */
T3lo =  1.8350121769317163e-17,		/*  0x15280000000000.0p-108 */
T5hi =  0.13333333333333336,		/*  0x11111111111112.0p-55 */
T5lo =  1.3051083651294260e-17,		/*  0x1e180000000000.0p-109 */
T7hi =  0.053968253968250494,		/*  0x1ba1ba1ba1b827.0p-57 */
T7lo =  3.1509625637859973e-18,		/*  0x1d100000000000.0p-111 */
pio4_hi =  0.78539816339744828,		/*  0x1921fb54442d18.0p-53 */
pio4_lo =  3.0628711372715500e-17,	/*  0x11a80000000000.0p-107 */
pio4lo_hi = -1.2541394031670831e-20,	/* -0x1d9cceba3f91f2.0p-119 */
pio4lo_lo =  6.1493048227390915e-37;	/*  0x1a280000000000.0p-173 */
#define	T3	((long double)T3hi + T3lo)
#define	T5	((long double)T5hi + T5lo)
#define	T7	((long double)T7hi + T7lo)
#define	pio4	((long double)pio4_hi + pio4_lo)
#define	pio4lo	((long double)pio4lo_hi + pio4lo_lo)
#else
static const long double
T3 =   0.333333333333333333180L,	/*  0xaaaaaaaaaaaaaaa5.0p-65 */
T5 =   0.133333333333333372290L,	/*  0x88888888888893c3.0p-66 */
T7 =   0.0539682539682504975744L,	/*  0xdd0dd0dd0dc13ba2.0p-68 */
pio4 = 0.785398163397448309628L,	/*  0xc90fdaa22168c235.0p-64 */
pio4lo = -1.25413940316708300586e-20L;	/* -0xece675d1fc8f8cbb.0p-130 */
#endif

static const double
T9  =  0.021869488536312216,		/*  0x1664f4882cc1c2.0p-58 */
T11 =  0.0088632355256619590,		/*  0x1226e355c17612.0p-59 */
T13 =  0.0035921281113786528,		/*  0x1d6d3d185d7ff8.0p-61 */
T15 =  0.0014558334756312418,		/*  0x17da354aa3f96b.0p-62 */
T17 =  0.00059003538700862256,		/*  0x13559358685b83.0p-63 */
T19 =  0.00023907843576635544,		/*  0x1f56242026b5be.0p-65 */
T21 =  0.000097154625656538905,		/*  0x1977efc26806f4.0p-66 */
T23 =  0.000038440165747303162,		/*  0x14275a09b3ceac.0p-67 */
T25 =  0.000018082171885432524,		/*  0x12f5e563e5487e.0p-68 */
T27 =  0.0000024196006108814377,	/*  0x144c0d80cc6896.0p-71 */
T29 =  0.0000078293456938132840,	/*  0x106b59141a6cb3.0p-69 */
T31 = -0.0000032609076735050182,	/* -0x1b5abef3ba4b59.0p-71 */
T33 =  0.0000023261313142559411;	/*  0x13835436c0c87f.0p-71 */

long double
__kernel_tanl(long double x, long double y, int iy) {
	long double z, r, v, w, s;
	long double osign;
	int i;

	iy = (iy == 1 ? -1 : 1);	/* XXX recover original interface */
	osign = (x >= 0 ? 1.0 : -1.0);	/* XXX slow, probably wrong for -0 */
	if (fabsl(x) >= 0.67434) {
		if (x < 0) {
			x = -x;
			y = -y;
		}
		z = pio4 - x;
		w = pio4lo - y;
		x = z + w;
		y = 0.0;
		i = 1;
	} else
		i = 0;
	z = x * x;
	w = z * z;
	r = T5 + w * (T9 + w * (T13 + w * (T17 + w * (T21 +
	    w * (T25 + w * (T29 + w * T33))))));
	v = z * (T7 + w * (T11 + w * (T15 + w * (T19 + w * (T23 +
	    w * (T27 + w * T31))))));
	s = z * x;
	r = y + z * (s * (r + v) + y);
	r += T3 * s;
	w = x + r;
	if (i == 1) {
		v = (long double) iy;
		return osign *
			(v - 2.0 * (x - (w * w / (w + v) - r)));
	}
	if (iy == 1)
		return w;
	else {
		/*
		 * if allow error up to 2 ulp, simply return
		 * -1.0 / (x+r) here
		 */
		/* compute -1.0 / (x+r) accurately */
		long double a, t;
		z = w;
		z = z + 0x1p32 - 0x1p32;
		v = r - (z - x);	/* z+v = r+x */
		t = a = -1.0 / w;	/* a = -1.0/w */
		t = t + 0x1p32 - 0x1p32;
		s = 1.0 + t * z;
		return t + a * (s + t * v);
	}
}

/*
 * ---------------------------------------------------------------------------
 * lib/msun/ld80/s_tanpil.c  -- the batch.
 * ---------------------------------------------------------------------------
 */

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
 * See ../src/s_tanpi.c for implementation details.
 */

static const double
pi_hi =  3.1415926814079285e+00,	/* 0x400921fb 0x58000000 */
pi_lo = -2.7818135228334233e-08;	/* 0xbe5dde97 0x3dcb3b3a */

/*
 * "static inline" dropped so that the differential harness can reach this
 * function directly.  The body is unmodified.
 */
long double
ref___kernel_tanpil(long double x)
{
	long double hi, lo, t;

	if (x < 0.25) {
		hi = (float)x;
		lo = x - hi;
		lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
		hi *= pi_hi;
		_2sumF(hi, lo);
		t = __kernel_tanl(hi, lo, -1);
	} else if (x > 0.25) {
		x = 0.5 - x;
		hi = (float)x;
		lo = x - hi;
		lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
		hi *= pi_hi;
		_2sumF(hi, lo);
		t = - __kernel_tanl(hi, lo, 1);
	} else
		t = 1;

	return (t);
}

volatile static const double vzero = 0;

long double
ref_tanpil(long double x)
{
	long double ax, hi, lo, odd, t;
	uint64_t lx, m;
	uint32_t j0;
	uint16_t hx, ix;

	EXTRACT_LDBL80_WORDS(hx, lx, x);
	ix = hx & 0x7fff;
	INSERT_LDBL80_WORDS(ax, ix, lx);

	ENTERI();

	if (ix < 0x3fff) {			/* |x| < 1 */
		if (ix < 0x3ffe) {		/* |x| < 0.5 */
			if (ix < 0x3fdd) {	/* |x| < 0x1p-34 */
				if (x == 0)
					RETURNI(x);
				INSERT_LDBL80_WORDS(hi, hx,
				    lx & 0xffffffff00000000ull);
				hi *= 0x1p63L;
				lo = x * 0x1p63L - hi;
				t = (pi_lo + pi_hi) * lo + pi_lo * hi +
				    pi_hi * hi;
				RETURNI(t * 0x1p-63L);
			}
			t = ref___kernel_tanpil(ax);
		} else if (ax == 0.5)
			t = 1 / vzero;
		else
			t = -ref___kernel_tanpil(1 - ax);
		RETURNI((hx & 0x8000) ? -t : t);
	}

	if (ix < 0x403e) {			/* 1 <= |x| < 0x1p63 */
		FFLOORL80(x, j0, ix, lx);	/* Integer part of ax. */
		odd = (uint64_t)x & 1 ? -1 : 1;
		ax -= x;
		EXTRACT_LDBL80_WORDS(ix, lx, ax);

		if (ix < 0x3ffe)		/* |x| < 0.5 */
			t = ix == 0 ? copysignl(0, odd) : ref___kernel_tanpil(ax);
		else if (ax == 0.5L)
			t = odd / vzero;
		else
			t = -ref___kernel_tanpil(1 - ax);
		RETURNI((hx & 0x8000) ? -t : t);
	}

	/* x = +-inf or nan. */
	if (ix >= 0x7fff)
		RETURNI(vzero / vzero);

	/*
	 * For 0x1p63 <= |x| < 0x1p64 need to determine if x is an even
	 * or odd integer to set t = +0 or -0.
	 * For |x| >= 0x1p64, it is always an even integer, so t = 0.
	 */
	t = ix >= 0x403f ? 0 : (copysignl(0, (lx & 1) ? -1 : 1));
	RETURNI((hx & 0x8000) ? -t : t);
}
