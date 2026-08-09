/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Port of:
 *	lib/libc/quad/floatdisf.c
 *	lib/libc/quad/ashrdi3.c
 *	lib/libc/quad/fixunsdfdi.c
 *	lib/libc/quad/fixunssfdi.c
 */

module;

#include <climits>
#include <cstdint>

export module pbsd.lib.libc.quad.b0042;

export namespace pbsd::lib_libc_quad::b0042 {

typedef long long quad_t;
typedef unsigned long long u_quad_t;

} // namespace pbsd::lib_libc_quad::b0042

namespace pbsd::lib_libc_quad::b0042 {

typedef std::int32_t quad_long;
typedef std::uint32_t quad_u_long;
typedef unsigned long u_long;

union uu {
	quad_t	q;
	quad_t	uq;
	quad_long	sl[2];
	quad_u_long	ul[2];
};

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
inline constexpr int H = 0;
inline constexpr int L = 1;
#else
inline constexpr int H = 1;
inline constexpr int L = 0;
#endif

inline constexpr int QUAD_BITS = sizeof(quad_t) * CHAR_BIT;
inline constexpr int LONG_BITS = sizeof(quad_long) * CHAR_BIT;
inline constexpr int HALF_BITS = sizeof(quad_long) * CHAR_BIT / 2;

typedef unsigned int qshift_t;

#define	ONE_FOURTH	(1L << (LONG_BITS - 2))
#define	ONE_HALF	(ONE_FOURTH * 2.0)
#define	ONE		(ONE_FOURTH * 4.0)

} // namespace pbsd::lib_libc_quad::b0042

export namespace pbsd::lib_libc_quad::b0042 {

/*
 * Convert (signed) quad to float.
 */
float
__floatdisf(quad_t x)
{
	float f;
	union uu u;
	int neg;

	/*
	 * Get an unsigned number first, by negating if necessary.
	 */
	if (x < 0)
		u.q = -x, neg = 1;
	else
		u.q = x, neg = 0;

	/*
	 * Now u.ul[H] has the factor of 2^32 (or whatever) and u.ul[L]
	 * has the units.  Ideally we could just set f, add LONG_BITS to
	 * its exponent, and then add the units, but this is portable
	 * code and does not know how to get at an exponent.  Machine-
	 * specific code may be able to do this more efficiently.
	 *
	 * Using double here may be excessive paranoia.
	 */
	f = (double)u.ul[H] * ((1L << (LONG_BITS - 2)) * 4.0);
	f += u.ul[L];

	return (neg ? -f : f);
}

/*
 * Shift a (signed) quad value right (arithmetic shift right).
 */
quad_t
__ashrdi3(quad_t a, qshift_t shift)
{
	union uu aa;

	aa.q = a;
	if (shift >= LONG_BITS) {
		quad_long s;

		/*
		 * Smear bits rightward using the machine's right-shift
		 * method, whether that is sign extension or zero fill,
		 * to get the `sign word' s.  Note that shifting by
		 * LONG_BITS is undefined, so we shift (LONG_BITS-1),
		 * then 1 more, to get our answer.
		 */
		s = (aa.sl[H] >> (LONG_BITS - 1)) >> 1;
		aa.ul[L] = shift >= QUAD_BITS ? s :
		    aa.sl[H] >> (shift - LONG_BITS);
		aa.ul[H] = s;
	} else if (shift > 0) {
		aa.ul[L] = (aa.ul[L] >> shift) |
		    (aa.ul[H] << (LONG_BITS - shift));
		aa.sl[H] >>= shift;
	}
	return (aa.q);
}

/*
 * Convert double to (unsigned) quad.
 * Not sure what to do with negative numbers---for now, anything out
 * of range becomes UQUAD_MAX.
 */
u_quad_t
__fixunsdfdi(double x)
{
	double toppart;
	union uu t;

	if (x < 0)
		return (~(u_quad_t)0);	/* ??? should be 0?  ERANGE??? */
#ifdef notdef				/* this falls afoul of a GCC bug */
	if (x >= UQUAD_MAX)
		return (UQUAD_MAX);
#else					/* so we wire in 2^64-1 instead */
	if (x >= 18446744073709551615.0)
		return (~(u_quad_t)0);
#endif
	/*
	 * Get the upper part of the result.  Note that the divide
	 * may round up; we want to avoid this if possible, so we
	 * subtract `1/2' first.
	 */
	toppart = (x - ONE_HALF) / ONE;
	/*
	 * Now build a u_quad_t out of the top part.  The difference
	 * between x and this is the bottom part (this may introduce
	 * a few fuzzy bits, but what the heck).  With any luck this
	 * difference will be nonnegative: x should wind up in the
	 * range [0..ULONG_MAX].  For paranoia, we assume [LONG_MIN..
	 * 2*ULONG_MAX] instead.
	 */
	t.ul[H] = (unsigned long)toppart;
	t.ul[L] = 0;
	x -= (double)t.uq;
	if (x < 0) {
		t.ul[H]--;
		x += (double)ULONG_MAX;
	}
	if (x > (double)ULONG_MAX) {
		t.ul[H]++;
		x -= (double)ULONG_MAX;
	}
	t.ul[L] = (u_long)x;
	return (t.uq);
}

/*
 * Convert float to (unsigned) quad.  We do most of our work in double,
 * out of sheer paranoia.
 *
 * Not sure what to do with negative numbers---for now, anything out
 * of range becomes UQUAD_MAX.
 *
 * N.B.: must use new ANSI syntax (sorry).
 */
u_quad_t
__fixunssfdi(float f)
{
	double x, toppart;
	union uu t;

	if (f < 0)
		return (~(u_quad_t)0);	/* ??? should be 0?  ERANGE??? */
#ifdef notdef				/* this falls afoul of a GCC bug */
	if (f >= UQUAD_MAX)
		return (UQUAD_MAX);
#else					/* so we wire in 2^64-1 instead */
	if (f >= 18446744073709551615.0)
		return (~(u_quad_t)0);
#endif
	x = f;
	/*
	 * Get the upper part of the result.  Note that the divide
	 * may round up; we want to avoid this if possible, so we
	 * subtract `1/2' first.
	 */
	toppart = (x - ONE_HALF) / ONE;
	/*
	 * Now build a u_quad_t out of the top part.  The difference
	 * between x and this is the bottom part (this may introduce
	 * a few fuzzy bits, but what the heck).  With any luck this
	 * difference will be nonnegative: x should wind up in the
	 * range [0..ULONG_MAX].  For paranoia, we assume [LONG_MIN..
	 * 2*ULONG_MAX] instead.
	 */
	t.ul[H] = (unsigned long)toppart;
	t.ul[L] = 0;
	x -= (double)t.uq;
	if (x < 0) {
		t.ul[H]--;
		x += (double)ULONG_MAX;
	}
	if (x > (double)ULONG_MAX) {
		t.ul[H]++;
		x -= (double)ULONG_MAX;
	}
	t.ul[L] = (u_long)x;
	return (t.uq);
}

} // namespace pbsd::lib_libc_quad::b0042
