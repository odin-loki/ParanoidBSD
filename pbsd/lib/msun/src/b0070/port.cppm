/*-
 * PBSD batch b0070 -- faithful C++23 port of:
 *   hbsd/src/lib/msun/src/s_conjl.c
 *   hbsd/src/lib/msun/src/s_cimagf.c
 *   hbsd/src/lib/msun/src/s_cimag.c
 *   hbsd/src/lib/msun/src/s_cargl.c
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

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005-2008 David Schultz <das@FreeBSD.ORG>
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

module;

#include <math.h>

export module pbsd.lib.msun.src.b0070;

/*
 * <complex.h> declares nothing in C++ mode and there is no __builtin_complex
 * in g++, so the C spellings used by the originals are provided here with the
 * same semantics: creall and cimagl are component reads, CMPLXL builds a
 * value from two components without any intervening arithmetic.
 */
namespace pbsd::lib_msun_src::b0070::detail {

using float_complex_t = __complex__ float;
using double_complex_t = __complex__ double;
using long_double_complex_t = __complex__ long double;

/* math_private.h */
typedef union {
	float_complex_t f;
	float a[2];
} float_complex;

typedef union {
	double_complex_t f;
	double a[2];
} double_complex;

#define	REALPART(z)	((z).a[0])
#define	IMAGPART(z)	((z).a[1])

inline long double
creall_(long_double_complex_t z)
{

	return (__real__ z);
}

inline long double
cimagl_(long_double_complex_t z)
{

	return (__imag__ z);
}

inline long_double_complex_t
CMPLXL_(long double x, long double y)
{
	long_double_complex_t z;

	__real__ z = x;
	__imag__ z = y;
	return (z);
}

} /* namespace pbsd::lib_msun_src::b0070::detail */

export namespace pbsd::lib_msun_src::b0070 {

using detail::double_complex_t;
using detail::float_complex_t;
using detail::long_double_complex_t;

/* s_conjl.c */
long_double_complex_t
conjl(long_double_complex_t z)
{

	return (detail::CMPLXL_(detail::creall_(z), -detail::cimagl_(z)));
}

/* s_cimagf.c */
float
cimagf(float_complex_t z)
{
	const detail::float_complex z1 = { .f = z };

	return (IMAGPART(z1));
}

/* s_cimag.c */
double
cimag(double_complex_t z)
{
	const detail::double_complex z1 = { .f = z };

	return (IMAGPART(z1));
}

/* s_cargl.c */
long double
cargl(long_double_complex_t z)
{

	return (atan2l(detail::cimagl_(z), detail::creall_(z)));
}

} /* namespace pbsd::lib_msun_src::b0070 */
