/*
 * oracle.c -- reference implementation for PBSD batch b0303.
 *
 * The original HardenedBSD sources from lib/msun/src are concatenated below.
 * Every function has been renamed with a ref_ prefix; function bodies are
 * otherwise byte-for-byte unmodified.
 *
 * This file is the specification.  Do not modify any function body.
 */

#include <complex.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

union IEEEf2bits {
	float	f;
	struct {
		unsigned int	man	:23;
		unsigned int	exp	:8;
		unsigned int	sign	:1;
	} bits;
};

union IEEEd2bits {
	double	d;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
	} bits;
};

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
#else
#error "Unsupported long double format"
#endif

#define	GET_LDBL_EXPSIGN(i,d)					\
do {								\
	union IEEEl2bits ge_u;					\
	ge_u.e = (d);							\
	(i) = ge_u.xbits.expsign;					\
} while (0)

#define	SET_LDBL_EXPSIGN(d,v)					\
do {								\
	union IEEEl2bits se_u;					\
	se_u.e = (d);							\
	se_u.xbits.expsign = (v);					\
	(d) = se_u.e;							\
} while (0)

#define	ENTERI()
#define	RETURNI(x)	return (x)

/* ======================================================================== */
/* lib/msun/src/s_signbit.c                                                 */
/* ======================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 Mike Barcroft <mike@FreeBSD.org>
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

int
ref___signbit(double d)
{
	union IEEEd2bits u;

	u.d = d;
	return (u.bits.sign);
}

int
ref___signbitf(float f)
{
	union IEEEf2bits u;

	u.f = f;
	return (u.bits.sign);
}

int
ref___signbitl(long double e)
{
	union IEEEl2bits u;

	u.e = e;
	return (u.bits.sign);
}

/* ======================================================================== */
/* lib/msun/src/e_atanhl.c                                                  */
/* ======================================================================== */

/* from: FreeBSD: head/lib/msun/src/e_atanh.c 176451 2008-02-22 02:30:36Z das */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

/*
 * See e_atanh.c for complete comments.
 *
 * Converted to long double by David Schultz <das@FreeBSD.ORG> and
 * Bruce D. Evans.
 */

/* EXP_TINY is the threshold below which we use atanh(x) ~= x. */
#if LDBL_MANT_DIG == 64
#define	EXP_TINY	-34
#elif LDBL_MANT_DIG == 113
#define	EXP_TINY	-58
#else
#error "Unsupported long double format"
#endif

#if LDBL_MAX_EXP != 0x4000
/* We also require the usual expsign encoding. */
#error "Unsupported long double format"
#endif

#define	BIAS	(LDBL_MAX_EXP - 1)

static const double one = 1.0, huge = 1e300;
static const double zero = 0.0;

long double
ref_atanhl(long double x)
{
	long double t;
	uint16_t hx, ix;

	ENTERI();
	GET_LDBL_EXPSIGN(hx, x);
	ix = hx & 0x7fff;
	if (ix >= 0x3fff)		/* |x| >= 1, or NaN or misnormal */
	    RETURNI(fabsl(x) == 1 ? x / zero : (x - x) / (x - x));
	if (ix < BIAS + EXP_TINY && (huge + x) > zero)
	    RETURNI(x);			/* x is tiny */
	SET_LDBL_EXPSIGN(x, ix);
	if (ix < 0x3ffe) {		/* |x| < 0.5, or misnormal */
	    t = x+x;
	    t = 0.5*log1pl(t+t*x/(one-x));
	} else 
	    t = 0.5*log1pl((x+x)/(one-x));
	RETURNI((hx & 0x8000) == 0 ? t : -t);
}

/* ======================================================================== */
/* lib/msun/src/s_cproj.c                                                   */
/* ======================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008 David Schultz <das@FreeBSD.ORG>
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

double complex
ref_cproj(double complex z)
{

	if (!isinf(creal(z)) && !isinf(cimag(z)))
		return (z);
	else
		return (CMPLX(INFINITY, copysign(0.0, cimag(z))));
}
