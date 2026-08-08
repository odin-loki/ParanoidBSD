/*
 * oracle.c -- reference implementation for PBSD batch b0136.
 *
 * The original HardenedBSD source from lib/msun/src/s_scalbnl.c is reproduced
 * below.  The function has been renamed with a "ref_" prefix; the function body
 * is otherwise byte-for-byte unmodified.  The only additions are the
 * declarations and macros that the original file obtained from "math.h",
 * "math_private.h" and "fpmath.h".
 *
 * This file is the specification.  Do not modify any function body.
 */

#include <math.h>
#include <float.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

#if LDBL_MANT_DIG == 64
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
#elif LDBL_MANT_DIG == 113
union IEEEl2bits {
	long double	e;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
	} bits;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	expsign	:16;
	} xbits;
};
#endif

/* ------------------------------------------------------------------ */
/* s_scalbnl.c							      */
/* ------------------------------------------------------------------ */

/*
 * Copyright (c) 2005-2020 Rich Felker, et al.
 *
 * SPDX-License-Identifier: MIT
 *
 * Please see https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT
 * for all contributors to musl.
 */
#include <math.h>
#include <float.h>
#include "math_private.h"
#include "fpmath.h"
/*
 * scalbnl (long double x, int n)
 * scalbnl(x,n) returns x* 2**n  computed by  exponent
 * manipulation rather than by actually performing an
 * exponentiation or a multiplication.
 */
#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
long double ref_scalbnl(long double x, int n)
{
	union IEEEl2bits u;

	if (n > 16383) {
		x *= 0x1p16383L;
		n -= 16383;
		if (n > 16383) {
			x *= 0x1p16383L;
			n -= 16383;
			if (n > 16383)
				n = 16383;
		}
	} else if (n < -16382) {
		x *= 0x1p-16382L * 0x1p113L;
		n += 16382 - 113;
		if (n < -16382) {
			x *= 0x1p-16382L * 0x1p113L;
			n += 16382 - 113;
			if (n < -16382)
				n = -16382;
		}
	}
	u.e = 1.0;
	u.xbits.expsign = 0x3fff + n;
	return x * u.e;
}
#endif
