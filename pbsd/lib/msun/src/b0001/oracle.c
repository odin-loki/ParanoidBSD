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
 * Reference oracle for PBSD batch b0001.  Original HardenedBSD sources
 * concatenated; functions carry a ref_ prefix, bodies are unmodified.
 *
 *   lib/msun/src/s_signgam.c
 *   lib/msun/src/s_lrintf.c
 *   lib/msun/src/s_llrint.c
 *   lib/msun/src/s_llrintf.c
 *
 * s_lrintf.c, s_llrint.c and s_llrintf.c are macro instantiations of the
 * s_lrint.c template; the preprocessor output of each instantiation is
 * reproduced verbatim below.
 */

#include <fenv.h>
#include <math.h>

/*
 * ---- lib/msun/src/s_signgam.c ----
 *
 * #include "math.h"
 * #include "math_private.h"
 */
int signgam = 0;

/*
 * ---- lib/msun/src/s_lrintf.c ----
 *
 * #define type		float
 * #define	roundit		rintf
 * #define dtype		long
 * #define	fn		lrintf
 *
 * #include "s_lrint.c"
 */

/*
 * C99 says we should not raise a spurious inexact exception when an
 * invalid exception is raised.  Unfortunately, the set of inputs
 * that overflows depends on the rounding mode when 'dtype' has more
 * significant bits than 'type'.  Hence, we bend over backwards for the
 * sake of correctness; an MD implementation could be more efficient.
 */
long
ref_lrintf(float x)
{
	fenv_t env;
	long d;

	feholdexcept(&env);
	d = (long)rintf(x);
	if (fetestexcept(FE_INVALID))
		feclearexcept(FE_INEXACT);
	feupdateenv(&env);
	return (d);
}

/*
 * ---- lib/msun/src/s_llrint.c ----
 *
 * #define type		double
 * #define	roundit		rint
 * #define dtype		long long
 * #define	fn		llrint
 *
 * #include "s_lrint.c"
 */

/*
 * C99 says we should not raise a spurious inexact exception when an
 * invalid exception is raised.  Unfortunately, the set of inputs
 * that overflows depends on the rounding mode when 'dtype' has more
 * significant bits than 'type'.  Hence, we bend over backwards for the
 * sake of correctness; an MD implementation could be more efficient.
 */
long long
ref_llrint(double x)
{
	fenv_t env;
	long long d;

	feholdexcept(&env);
	d = (long long)rint(x);
	if (fetestexcept(FE_INVALID))
		feclearexcept(FE_INEXACT);
	feupdateenv(&env);
	return (d);
}

/*
 * ---- lib/msun/src/s_llrintf.c ----
 *
 * #define type		float
 * #define	roundit		rintf
 * #define dtype		long long
 * #define	fn		llrintf
 *
 * #include "s_lrint.c"
 */

/*
 * C99 says we should not raise a spurious inexact exception when an
 * invalid exception is raised.  Unfortunately, the set of inputs
 * that overflows depends on the rounding mode when 'dtype' has more
 * significant bits than 'type'.  Hence, we bend over backwards for the
 * sake of correctness; an MD implementation could be more efficient.
 */
long long
ref_llrintf(float x)
{
	fenv_t env;
	long long d;

	feholdexcept(&env);
	d = (long long)rintf(x);
	if (fetestexcept(FE_INVALID))
		feclearexcept(FE_INEXACT);
	feupdateenv(&env);
	return (d);
}
