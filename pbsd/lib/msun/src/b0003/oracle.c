/*
 * Reference oracle for PBSD batch b0003.
 *
 * The original HardenedBSD lib/msun/src sources concatenated, with every
 * function renamed with a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * s_lrintl.c and s_llrintl.c consist solely of #defines plus #include
 * "s_lrint.c".  s_lroundf.c and s_lroundl.c consist solely of #defines plus
 * #include "s_lround.c".  Their preprocessed expansions are written out here.
 * The s_lround.c template's file-scope constants are given per-instantiation
 * names so the two copies can share one translation unit; INRANGE is #undef'd
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
/* s_lrintl.c:
 *	#define type		long double
 *	#define	roundit		rintl
 *	#define dtype		long
 *	#define	fn		lrintl
 *	#include "s_lrint.c"
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

long
ref_lrintl(long double x)
{
	fenv_t env;
	long d;

	feholdexcept(&env);
	d = (long)rintl(x);
	if (fetestexcept(FE_INVALID))
		feclearexcept(FE_INEXACT);
	feupdateenv(&env);
	return (d);
}

/* ---------------------------------------------------------------------- */
/* s_llrintl.c:
 *	#define type		long double
 *	#define	roundit		rintl
 *	#define dtype		long long
 *	#define	fn		llrintl
 *	#include "s_lrint.c"
 */

long long
ref_llrintl(long double x)
{
	fenv_t env;
	long long d;

	feholdexcept(&env);
	d = (long long)rintl(x);
	if (fetestexcept(FE_INVALID))
		feclearexcept(FE_INEXACT);
	feupdateenv(&env);
	return (d);
}

/* ---------------------------------------------------------------------- */
/* s_lroundf.c:
 *	#define type		float
 *	#define	roundit		roundf
 *	#define dtype		long
 *	#define	DTYPE_MIN	LONG_MIN
 *	#define	DTYPE_MAX	LONG_MAX
 *	#define	fn		lroundf
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

static const float type_min_lroundf = (float)LONG_MIN;
static const float type_max_lroundf = (float)LONG_MAX;
static const float dtype_min_lroundf = (float)LONG_MIN - 0.5;
static const float dtype_max_lroundf = (float)LONG_MAX + 0.5;
#define	INRANGE(x)	(dtype_max_lroundf - type_max_lroundf != 0.5 || \
			 ((x) > dtype_min_lroundf && (x) < dtype_max_lroundf))

long
ref_lroundf(float x)
{

	if (INRANGE(x)) {
		x = roundf(x);
		return ((long)x);
	} else {
		feraiseexcept(FE_INVALID);
		return (LONG_MAX);
	}
}

#undef INRANGE

/* ---------------------------------------------------------------------- */
/* s_lroundl.c:
 *	#define type		long double
 *	#define	roundit		roundl
 *	#define dtype		long
 *	#define	DTYPE_MIN	LONG_MIN
 *	#define	DTYPE_MAX	LONG_MAX
 *	#define	fn		lroundl
 *	#include "s_lround.c"
 */

static const long double type_min_lroundl = (long double)LONG_MIN;
static const long double type_max_lroundl = (long double)LONG_MAX;
static const long double dtype_min_lroundl = (long double)LONG_MIN - 0.5;
static const long double dtype_max_lroundl = (long double)LONG_MAX + 0.5;
#define	INRANGE(x)	(dtype_max_lroundl - type_max_lroundl != 0.5 || \
			 ((x) > dtype_min_lroundl && (x) < dtype_max_lroundl))

long
ref_lroundl(long double x)
{

	if (INRANGE(x)) {
		x = roundl(x);
		return ((long)x);
	} else {
		feraiseexcept(FE_INVALID);
		return (LONG_MAX);
	}
}

#undef INRANGE
