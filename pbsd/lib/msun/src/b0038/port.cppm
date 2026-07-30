/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
 * Copyright (c) 2026 Jesús Blázquez <jesuscblazquez@gmail.com>
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
 * PBSD batch b0038: C++23 port of
 *   lib/msun/src/s_fminimuml.c
 *   lib/msun/src/s_fmaximuml.c
 *   lib/msun/src/s_fminimumf.c
 *   lib/msun/src/s_fmaximumf.c
 */

module;

#include <math.h>

export module pbsd.lib.msun.src.b0038;

#if !defined(__amd64__) && !defined(__x86_64__) && !defined(__i386__)
#error "batch b0038: unsupported long double format"
#endif

namespace pbsd::lib_msun_src::b0038::detail {

/* From <machine/_fpmath.h> (amd64/i386). */
union IEEEl2bits {
	long double	e;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:32;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
		unsigned int	junk	:16;
	} bits;
	struct {
		unsigned long long	man	:64;
		unsigned int		expsign	:16;
		unsigned int		junk	:16;
	} xbits;
};

inline constexpr unsigned int LDBL_NBIT = 0x80000000;

union IEEEf2bits {
	float	f;
	struct {
		unsigned int	man	:23;
		unsigned int	exp	:8;
		unsigned int	sign	:1;
	} bits;
};

} // namespace pbsd::lib_msun_src::b0038::detail

export namespace pbsd::lib_msun_src::b0038 {

long double
fminimuml(long double x, long double y)
{
	detail::IEEEl2bits u[2];

	u[0].e = x;
	u[0].bits.manh &= ~detail::LDBL_NBIT;
	u[1].e = y;
	u[1].bits.manh &= ~detail::LDBL_NBIT;

	/* Handle NaN according to ISO/IEC 60559. NaN argument -> NaN return */
	if (u[0].bits.exp == 32767 && (u[0].bits.manh | u[0].bits.manl) != 0 ||
	    u[1].bits.exp == 32767 && (u[1].bits.manh | u[1].bits.manl) != 0)
		return (NAN);

	/* Handle comparisons of signed zeroes. */
	if (u[0].bits.sign != u[1].bits.sign)
		return (u[1].bits.sign ? y : x);

	return (x < y ? x : y);
}

long double
fmaximuml(long double x, long double y)
{
	detail::IEEEl2bits u[2];

	u[0].e = x;
	u[0].bits.manh &= ~detail::LDBL_NBIT;
	u[1].e = y;
	u[1].bits.manh &= ~detail::LDBL_NBIT;

	/* Handle NaN according to ISO/IEC 60559. NaN argument -> NaN return */
	if ((u[0].bits.exp == 32767 && (u[0].bits.manh | u[0].bits.manl) != 0) ||
	    (u[1].bits.exp == 32767 && (u[1].bits.manh | u[1].bits.manl) != 0))
		return (NAN);

	/* Handle comparisons of signed zeroes. */
	if (u[0].bits.sign != u[1].bits.sign)
		return (u[0].bits.sign ? y : x);

	return (x > y ? x : y);
}

float
fminimumf(float x, float y)
{
	detail::IEEEf2bits u[2];

	u[0].f = x;
	u[1].f = y;

	/* Handle NaN according to ISO/IEC 60559. NaN argument -> NaN return */
	if (u[0].bits.exp == 255 && u[0].bits.man != 0 ||
	    u[1].bits.exp == 255 && u[1].bits.man != 0)
		return (NAN);

	/* Handle comparisons of signed zeroes. */
	if (u[0].bits.sign != u[1].bits.sign)
		return (u[u[1].bits.sign].f);

	return (x < y ? x : y);
}

float
fmaximumf(float x, float y)
{
	detail::IEEEf2bits u[2];

	u[0].f = x;
	u[1].f = y;

	/* Handle NaN according to ISO/IEC 60559. NaN argument -> NaN return */
	if ((u[0].bits.exp == 255 && u[0].bits.man != 0) ||
	    (u[1].bits.exp == 255 && u[1].bits.man != 0))
		return (NAN);

	/* Handle comparisons of signed zeroes. */
	if (u[0].bits.sign != u[1].bits.sign)
		return (u[u[0].bits.sign].f);

	return (x > y ? x : y);
}

} // namespace pbsd::lib_msun_src::b0038
