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

/*
 * Ported from HardenedBSD:
 *   lib/msun/src/s_crealf.c
 *   lib/msun/src/s_creal.c
 *   lib/msun/src/s_creall.c
 *   lib/msun/src/s_conj.c
 *
 * The C sources return a complex value from a function whose return type is
 * the corresponding real type; the implicit complex -> real conversion of C
 * yields the real part.  C++ has no such implicit conversion, so the real
 * part is named explicitly with __real__.  No other change is made.
 */

module;

export module pbsd.lib.msun.src.b0057;

export namespace pbsd::lib_msun_src::b0057 {

/* s_crealf.c */
float
crealf(float _Complex z)
{
	return __real__ z;
}

/* s_creal.c */
double
creal(double _Complex z)
{
	return __real__ z;
}

/* s_creall.c */
long double
creall(long double _Complex z)
{
	return __real__ z;
}

/*
 * s_conj.c
 *
 * CMPLX() composes a complex value out of two real parts without performing
 * any arithmetic on them, so the sign of a zero or of a NaN survives.  The
 * member-wise assignment below has exactly that property.  cimag() is not a
 * member of this batch; __imag__ is the same projection it performs.
 */
double _Complex
conj(double _Complex z)
{

	double _Complex r;
	__real__ r = creal(z);
	__imag__ r = -cimag_(z);
	return r;
}

}

namespace pbsd::lib_msun_src::b0057 {

double
cimag_(double _Complex z)
{
	return __imag__ z;
}

}
