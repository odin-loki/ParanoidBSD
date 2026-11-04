/*
 * oracle.c -- the reference specification for batch b0057.
 *
 * The original HardenedBSD sources are concatenated below, each function
 * renamed with a ref_ prefix.  No function body has been altered.
 */

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

#include <complex.h>

/*
 * s_conj.c includes "math_private.h", which is where FreeBSD picks up CMPLX
 * when the C library headers do not provide it.  math_private.h is not part
 * of this batch, so the one macro that s_conj.c actually needs from it is
 * supplied here.  C11 <complex.h> normally defines CMPLX already.
 */
#ifndef CMPLX
#define	CMPLX(x, y)	((double complex){ (double)(x), (double)(y) })
#endif

/* ------------------------------------------------------------------ */
/* lib/msun/src/s_crealf.c						*/
/* ------------------------------------------------------------------ */

float
ref_crealf(float complex z)
{
	return z;
}

/* ------------------------------------------------------------------ */
/* lib/msun/src/s_creal.c						*/
/* ------------------------------------------------------------------ */

double
ref_creal(double complex z)
{
	return z;
}

/* ------------------------------------------------------------------ */
/* lib/msun/src/s_creall.c						*/
/* ------------------------------------------------------------------ */

long double
ref_creall(long double complex z)
{
	return z;
}

/* ------------------------------------------------------------------ */
/* lib/msun/src/s_conj.c						*/
/* ------------------------------------------------------------------ */

double complex
ref_conj(double complex z)
{

	return (CMPLX(creal(z), -cimag(z)));
}
