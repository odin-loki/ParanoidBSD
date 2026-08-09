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

/*
 * PBSD port of:
 *   lib/msun/src/s_signgam.c
 *   lib/msun/src/s_lrintf.c   (s_lrint.c, type=float,  dtype=long)
 *   lib/msun/src/s_llrint.c   (s_lrint.c, type=double, dtype=long long)
 *   lib/msun/src/s_llrintf.c  (s_lrint.c, type=float,  dtype=long long)
 */

module;

#include <fenv.h>
#include <math.h>

export module pbsd.lib.msun.src.b0001;

export namespace pbsd::lib_msun_src::b0001 {

/*
 * s_signgam.c
 */
int signgam = 0;

/*
 * C99 says we should not raise a spurious inexact exception when an
 * invalid exception is raised.  Unfortunately, the set of inputs
 * that overflows depends on the rounding mode when 'dtype' has more
 * significant bits than 'type'.  Hence, we bend over backwards for the
 * sake of correctness; an MD implementation could be more efficient.
 */

/*
 * s_lrintf.c:
 *	type = float, roundit = rintf, dtype = long, fn = lrintf
 */
long
lrintf(float x)
{
	fenv_t env;
	long d;

	::feholdexcept(&env);
	d = (long)::rintf(x);
	if (::fetestexcept(FE_INVALID))
		::feclearexcept(FE_INEXACT);
	::feupdateenv(&env);
	return (d);
}

/*
 * s_llrint.c:
 *	type = double, roundit = rint, dtype = long long, fn = llrint
 */
long long
llrint(double x)
{
	fenv_t env;
	long long d;

	::feholdexcept(&env);
	d = (long long)::rint(x);
	if (::fetestexcept(FE_INVALID))
		::feclearexcept(FE_INEXACT);
	::feupdateenv(&env);
	return (d);
}

/*
 * s_llrintf.c:
 *	type = float, roundit = rintf, dtype = long long, fn = llrintf
 */
long long
llrintf(float x)
{
	fenv_t env;
	long long d;

	::feholdexcept(&env);
	d = (long long)::rintf(x);
	if (::fetestexcept(FE_INVALID))
		::feclearexcept(FE_INEXACT);
	::feupdateenv(&env);
	return (d);
}

} /* namespace pbsd::lib_msun_src::b0001 */
