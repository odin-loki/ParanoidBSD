// PBSD port of HardenedBSD lib/msun/src batch b0267.
//
// Sources ported in this module:
//   s_copysignl.c -> copysignl
//   s_sincos.c    -> sincos
//
// Original copyright headers are reproduced verbatim above each ported
// function.  The ports are literal transcriptions.  __kernel_sincos comes from
// k_sincos.h; __ieee754_rem_pio2 and __kernel_rem_pio2 are linked from the
// original msun objects built by build.sh.

module;

#include <cmath>
#include <cstdint>

export module pbsd.lib.msun.src.b0267;

namespace pbsd::lib_msun_src::b0267 {

/* from lib/libc/amd64/_fpmath.h (via <fpmath.h>) */

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
		unsigned long long	man	:64;
		unsigned int		expsign	:16;
		unsigned long long	junk	:48;
	} xbits;
};

#include "k_sincos.h"

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 Stefan Farfeleder
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
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

export long double
copysignl(long double x, long double y)
{
	union IEEEl2bits ux, uy;

	ux.e = x;
	uy.e = y;
	ux.bits.sign = uy.bits.sign;
	return (ux.e);
}

/*-
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 * 
 * s_sin.c and s_cos.c merged by Steven G. Kargl.  Descriptions of the
 * algorithms are contained in the original files.
 */

export void
sincos(double x, double *sn, double *cs)
{
	double y[2];
	int32_t n, ix;

	/* High word of x. */
	GET_HIGH_WORD(ix, x);

	/* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if (ix <= 0x3fe921fb) {
		if (ix < 0x3e400000) {		/* |x| < 2**-27 */
			if ((int)x == 0) {	/* Generate inexact. */
				*sn = x;
				*cs = 1;
				return;
			}
		}
		__kernel_sincos(x, 0, 0, sn, cs);
		return;
	}

	/* If x = Inf or NaN, then sin(x) = NaN and cos(x) = NaN. */
	if (ix >= 0x7ff00000) {
		*sn = x - x;
		*cs = x - x;
		return;
	}

	/* Argument reduction. */
	n = __ieee754_rem_pio2(x, y);

	switch(n & 3) {
	case 0:
		__kernel_sincos(y[0], y[1], 1, sn, cs);
		break;
	case 1:
		__kernel_sincos(y[0], y[1], 1, cs, sn);
		*cs = -*cs;
		break;
	case 2:
		__kernel_sincos(y[0], y[1], 1, sn, cs);
		*sn = -*sn;
		*cs = -*cs;
		break;
	default:
		__kernel_sincos(y[0], y[1], 1, cs, sn);
		*sn = -*sn;
	}
}

} // namespace pbsd::lib_msun_src::b0267
