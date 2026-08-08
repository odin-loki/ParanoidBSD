module;

#include <math.h>
#include <cstdint>
#include <cfloat>

export module pbsd.lib.msun.ld128.b0088s4;

export namespace pbsd::lib_msun_ld128::b0088s4 {

union IEEEl2bits {
	long double e;
	struct {
		unsigned long long man : 64;
		unsigned int exp : 15;
		unsigned int sign : 1;
	} bits;
	struct {
		unsigned long long man : 64;
		unsigned int expsign : 16;
	} xbits;
};

#define _2sumF(a, b) do {	\
	__typeof__(a) __w;	\
	__w = (a) + (b);	\
	(b) = ((a) - __w) + (b); \
	(a) = __w;		\
} while (0)

#define FFLOORL128(x, ai, ar) do {			\
	union IEEEl2bits u;				\
	uint64_t m;					\
	int e;						\
	u.e = (x);					\
	e = u.bits.exp - 16383;				\
	if (e < 63) {					\
		m = (uint64_t)-1 >> (e + 1);		\
		u.bits.man &= ~m;			\
	}						\
	(ai) = u.e;					\
	(ar) = (x) - (ai);				\
} while (0)

/* k_tanl.c — dependency of __kernel_tanpil */
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
 * ld128 version of k_tan.c.  See ../src/k_tan.c for most comments.
 */

/*
 * Domain [-0.67434, 0.67434], range ~[-3.37e-36, 1.982e-37]
 * |tan(x)/x - t(x)| < 2**-117.8 (XXX should be ~1e-37)
 *
 * See ../ld80/k_cosl.c for more details about the polynomial.
 */
static const long double
T3 = 0x1.5555555555555555555555555553p-2L,
T5 = 0x1.1111111111111111111111111eb5p-3L,
T7 = 0x1.ba1ba1ba1ba1ba1ba1ba1b694cd6p-5L,
T9 = 0x1.664f4882c10f9f32d6bbe09d8bcdp-6L,
T11 = 0x1.226e355e6c23c8f5b4f5762322eep-7L,
T13 = 0x1.d6d3d0e157ddfb5fed8e84e27b37p-9L,
T15 = 0x1.7da36452b75e2b5fce9ee7c2c92ep-10L,
T17 = 0x1.355824803674477dfcf726649efep-11L,
T19 = 0x1.f57d7734d1656e0aceb716f614c2p-13L,
T21 = 0x1.967e18afcb180ed942dfdc518d6cp-14L,
T23 = 0x1.497d8eea21e95bc7e2aa79b9f2cdp-15L,
T25 = 0x1.0b132d39f055c81be49eff7afd50p-16L,
T27 = 0x1.b0f72d33eff7bfa2fbc1059d90b6p-18L,
T29 = 0x1.5ef2daf21d1113df38d0fbc00267p-19L,
T31 = 0x1.1c77d6eac0234988cdaa04c96626p-20L,
T33 = 0x1.cd2a5a292b180e0bdd701057dfe3p-22L,
T35 = 0x1.75c7357d0298c01a31d0a6f7d518p-23L,
T37 = 0x1.2f3190f4718a9a520f98f50081fcp-24L,
pio4 = 0x1.921fb54442d18469898cc51701b8p-1L,
pio4lo = 0x1.cd129024e088a67cc74020bbea60p-116L;

static const double
T39 =  0.000000028443389121318352,	/*  0x1e8a7592977938.0p-78 */
T41 =  0.000000011981013102001973,	/*  0x19baa1b1223219.0p-79 */
T43 =  0.0000000038303578044958070,	/*  0x107385dfb24529.0p-80 */
T45 =  0.0000000034664378216909893,	/*  0x1dc6c702a05262.0p-81 */
T47 = -0.0000000015090641701997785,	/* -0x19ecef3569ebb6.0p-82 */
T49 =  0.0000000029449552300483952,	/*  0x194c0668da786a.0p-81 */
T51 = -0.0000000022006995706097711,	/* -0x12e763b8845268.0p-81 */
T53 =  0.0000000015468200913196612,	/*  0x1a92fc98c29554.0p-82 */
T55 = -0.00000000061311613386849674,	/* -0x151106cbc779a9.0p-83 */
T57 =  1.4912469681508012e-10;		/*  0x147edbdba6f43a.0p-85 */

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
	    w * (T25 + w * (T29 + w * (T33 +
	    w * (T37 + w * (T41 + w * (T45 + w * (T49 + w * (T53 +
	    w * T57))))))))))));
	v = z * (T7 + w * (T11 + w * (T15 + w * (T19 + w * (T23 +
	    w * (T27 + w * (T31 + w * (T35 +
	    w * (T39 + w * (T43 + w * (T47 + w * (T51 + w * T55))))))))))));
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

/* s_tanpil.c */
/*-
 * Copyright (c) 2017-2023 Steven G. Kargl
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

/*
 * pi_hi contains the leading 56 bits of a 169 bit approximation for pi.
 */
static const long double
pi_hi = 3.14159265358979322702026593105983920e+00L,
pi_lo = 1.14423774522196636802434264184180742e-17L;

long double
__kernel_tanpil(long double x)
{
	long double hi, lo, t;

	if (x < 0.25) {
		hi = (double)x;
		lo = x - hi;
		lo = lo * (pi_lo + pi_hi) + hi * pi_lo;
		hi *= pi_hi;
		_2sumF(hi, lo);
		t = __kernel_tanl(hi, lo, -1);
	} else if (x > 0.25) {
		x = 0.5 - x;
		hi = (double)x;
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
tanpil(long double x)
{
	long double ai, ar, ax, hi, lo, t;
	double odd;

	ax = fabsl(x);

	if (ax < 1) {
		if (ax < 0.5) {
			if (ax < 0x1p-60) {
				if (x == 0)
					return (x);
				hi = (double)x;
				hi *= 0x1p113L;
				lo = x * 0x1p113L - hi;
				t = (pi_lo + pi_hi) * lo + pi_lo * lo +
				    pi_hi * hi;
				return (t * 0x1p-113L);
			}
			t = __kernel_tanpil(ax);
		} else if (ax == 0.5)
			t = 1 / vzero;
		else
			t = -__kernel_tanpil(1 - ax);
		return (x < 0 ? -t : t);
	}

	if (ax < 0x1p112) {
		/* Split ax = ai + ar with 0 <= ar < 1. */
		FFLOORL128(ax, ai, ar);
		odd = fmodl(ai, 2.L) == 0 ? 1 : -1;
		if (ar < 0.5)
			t = ar == 0 ? copysign(0., odd) : __kernel_tanpil(ar);
		else if (ar == 0.5)
			t = odd / vzero;
		else
			t = -__kernel_tanpil(1 - ar);
		return (x < 0 ? -t : t);
	}

	/* x = +-inf or nan. */
	if (isinf(x) || isnan(x))
		return (vzero / vzero);

	/*
	 * For 0x1p112 <= |x| < 0x1p113 need to determine if x is an even
	 * or odd integer to set t = +0 or -0.
	 * For |x| >= 0x1p113, it is always an even integer, so t = 0.
	 */
	t = fmodl(ax,2.L) == 0  ? 0 : copysign(0., -1.);
	return (copysignl(t, x));
}

} // export namespace
