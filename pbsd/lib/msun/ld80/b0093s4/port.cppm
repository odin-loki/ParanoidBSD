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
 * PBSD port of lib/msun/ld80/s_tanpil.c to C++23.
 *
 * See ../src/s_tanpi.c for implementation details.
 *
 * __kernel_tanl (lib/msun/ld80/k_tanl.c) is not part of this batch; it is
 * resolved at link time exactly as it is in libm.
 */

module;

#include <cstdint>
#include <cmath>

export module pbsd.lib.msun.ld80.b0093s4;

extern "C" long double __kernel_tanl(long double x, long double y, int iy);

export namespace pbsd::lib_msun_ld80::b0093s4 {

long double __kernel_tanpil(long double x);
long double tanpil(long double x);

} /* export namespace pbsd::lib_msun_ld80::b0093s4 */

namespace pbsd::lib_msun_ld80::b0093s4 {

/* lib/libc/amd64/_fpmath.h */
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

/* lib/msun/src/math_private.h (the !__i386__ variants) */

#define	EXTRACT_LDBL80_WORDS(ix0,ix1,d)				\
do {								\
  IEEEl2bits ew_u;						\
  ew_u.e = (d);							\
  (ix0) = ew_u.xbits.expsign;					\
  (ix1) = ew_u.xbits.man;					\
} while (0)

#define	INSERT_LDBL80_WORDS(d,ix0,ix1)				\
do {								\
  IEEEl2bits iw_u;						\
  iw_u.xbits.expsign = (ix0);					\
  iw_u.xbits.man = (ix1);					\
  (d) = iw_u.e;							\
} while (0)

#define	ENTERI()
#define	RETURNI(x)	RETURNF(x)

/* Default return statement if hack*_t() is not used. */
#define      RETURNF(v)      return (v)

#define	_2sumF(a, b) do {	\
	decltype(a) __w;	\
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
		std::uint64_t _m;			\
		_m = (std::uint64_t)-1 >> (j0);		\
		if ((lx) & _m) (lx) &= ~_m;		\
	}						\
	INSERT_LDBL80_WORDS((x), (ix), (lx));		\
} while (0)

static const double
pi_hi =  3.1415926814079285e+00,	/* 0x400921fb 0x58000000 */
pi_lo = -2.7818135228334233e-08;	/* 0xbe5dde97 0x3dcb3b3a */

/*
 * "static inline" in the original.  Given module linkage here so that the
 * differential harness can exercise it directly; the body is unchanged.
 */
long double
__kernel_tanpil(long double x)
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
tanpil(long double x)
{
	long double ax, hi, lo, odd, t;
	std::uint64_t lx, m;
	std::uint32_t j0;
	std::uint16_t hx, ix;

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
			t = __kernel_tanpil(ax);
		} else if (ax == 0.5)
			t = 1 / vzero;
		else
			t = -__kernel_tanpil(1 - ax);
		RETURNI((hx & 0x8000) ? -t : t);
	}

	if (ix < 0x403e) {			/* 1 <= |x| < 0x1p63 */
		FFLOORL80(x, j0, ix, lx);	/* Integer part of ax. */
		odd = (std::uint64_t)x & 1 ? -1 : 1;
		ax -= x;
		EXTRACT_LDBL80_WORDS(ix, lx, ax);

		if (ix < 0x3ffe)		/* |x| < 0.5 */
			t = ix == 0 ? std::copysignl(0, odd) : __kernel_tanpil(ax);
		else if (ax == 0.5L)
			t = odd / vzero;
		else
			t = -__kernel_tanpil(1 - ax);
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
	t = ix >= 0x403f ? 0 : (std::copysignl(0, (lx & 1) ? -1 : 1));
	RETURNI((hx & 0x8000) ? -t : t);
}

} /* namespace pbsd::lib_msun_ld80::b0093s4 */
