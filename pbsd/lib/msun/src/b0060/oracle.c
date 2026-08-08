/*
 * Reference oracle for PBSD batch b0060.
 *
 * The original HardenedBSD lib/msun/src sources concatenated, with every
 * function renamed with a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * s_llround.c, s_llroundf.c and s_llroundl.c consist solely of #defines plus
 * #include "s_lround.c".  Their preprocessed expansions are written out here.
 * The s_lround.c template's file-scope constants are given per-instantiation
 * names so the three copies can share one translation unit; INRANGE is #undef'd
 * and redefined to match.  No function body is altered.
 *
 * <sys/limits.h> is a FreeBSD spelling of <limits.h>; the latter is used here
 * so the oracle builds on the host toolchain.
 */

#include <limits.h>
#include <fenv.h>
#include <math.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/* ---------------------------------------------------------------------- */
/* s_llround.c:
 *	#define type		double
 *	#define	roundit		round
 *	#define dtype		long long
 *	#define	DTYPE_MIN	LLONG_MIN
 *	#define	DTYPE_MAX	LLONG_MAX
 *	#define	fn		llround
 *	#include "s_lround.c"
 */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
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

static const double type_min_llround = (double)LLONG_MIN;
static const double type_max_llround = (double)LLONG_MAX;
static const double dtype_min_llround = (double)LLONG_MIN - 0.5;
static const double dtype_max_llround = (double)LLONG_MAX + 0.5;
#define	INRANGE(x)	(dtype_max_llround - type_max_llround != 0.5 || \
			 ((x) > dtype_min_llround && (x) < dtype_max_llround))

long long
ref_llround(double x)
{

	if (INRANGE(x)) {
		x = round(x);
		return ((long long)x);
	} else {
		feraiseexcept(FE_INVALID);
		return (LLONG_MAX);
	}
}

#undef INRANGE

/* ---------------------------------------------------------------------- */
/* s_llroundf.c:
 *	#define type		float
 *	#define	roundit		roundf
 *	#define dtype		long long
 *	#define	DTYPE_MIN	LLONG_MIN
 *	#define	DTYPE_MAX	LLONG_MAX
 *	#define	fn		llroundf
 *	#include "s_lround.c"
 */

static const float type_min_llroundf = (float)LLONG_MIN;
static const float type_max_llroundf = (float)LLONG_MAX;
static const float dtype_min_llroundf = (float)LLONG_MIN - 0.5;
static const float dtype_max_llroundf = (float)LLONG_MAX + 0.5;
#define	INRANGE(x)	(dtype_max_llroundf - type_max_llroundf != 0.5 || \
			 ((x) > dtype_min_llroundf && (x) < dtype_max_llroundf))

long long
ref_llroundf(float x)
{

	if (INRANGE(x)) {
		x = roundf(x);
		return ((long long)x);
	} else {
		feraiseexcept(FE_INVALID);
		return (LLONG_MAX);
	}
}

#undef INRANGE

/* ---------------------------------------------------------------------- */
/* s_llroundl.c:
 *	#define type		long double
 *	#define	roundit		roundl
 *	#define dtype		long long
 *	#define	DTYPE_MIN	LLONG_MIN
 *	#define	DTYPE_MAX	LLONG_MAX
 *	#define	fn		llroundl
 *	#include "s_lround.c"
 */

static const long double type_min_llroundl = (long double)LLONG_MIN;
static const long double type_max_llroundl = (long double)LLONG_MAX;
static const long double dtype_min_llroundl = (long double)LLONG_MIN - 0.5;
static const long double dtype_max_llroundl = (long double)LLONG_MAX + 0.5;
#define	INRANGE(x)	(dtype_max_llroundl - type_max_llroundl != 0.5 || \
			 ((x) > dtype_min_llroundl && (x) < dtype_max_llroundl))

long long
ref_llroundl(long double x)
{

	if (INRANGE(x)) {
		x = roundl(x);
		return ((long long)x);
	} else {
		feraiseexcept(FE_INVALID);
		return (LLONG_MAX);
	}
}

#undef INRANGE

/* ---------------------------------------------------------------------- */
/* w_drem.c */

/*
 * drem() wrapper for remainder().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

double
ref_drem(double x, double y)
{
	return remainder(x, y);
}
