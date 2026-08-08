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
 *	lib/libc/quad/adddi3.c
 *	lib/libc/quad/ashldi3.c
 *	lib/libc/quad/lshldi3.c
 *	lib/libc/quad/floatdidf.c
 */

module;

#include <climits>
#include <cstdint>

export module pbsd.lib.libc.quad.b0040;

export namespace pbsd::lib_libc_quad::b0040 {

typedef long long quad_t;
typedef unsigned long long u_quad_t;

} // namespace pbsd::lib_libc_quad::b0040

namespace pbsd::lib_libc_quad::b0040 {

/*
 * Quad arithmetic.
 *
 * This library makes the following assumptions:
 *
 *  - The type long long (aka quad_t) exists.
 *
 *  - A quad variable is exactly twice as long as `long'.
 *
 *  - The machine's arithmetic is two's complement.
 *
 * The second assumption is a hard precondition of every routine below:
 * `union uu' is only a valid decomposition of a quad while the two halves
 * exactly tile it.  It is honoured here by spelling the halfword type
 * explicitly at 32 bits, which is what `long' is on the machines this
 * directory is compiled for (i386 and friends).  Widening the halfword to
 * a native LP64 `long' would leave ul[1] outside the quad and thus
 * uninitialised, which is not a behaviour worth reproducing.
 */
typedef std::int32_t quad_long;
typedef std::uint32_t quad_u_long;

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

} // namespace pbsd::lib_libc_quad::b0040

export namespace pbsd::lib_libc_quad::b0040 {

/*
 * Add two quads.  This is trivial since a one-bit carry from a single
 * u_long addition x+y occurs if and only if the sum x+y is less than
 * either x or y (the choice to compare with x or y is arbitrary).
 */
quad_t
__adddi3(quad_t a, quad_t b)
{
	union uu aa, bb, sum;

	aa.q = a;
	bb.q = b;
	sum.ul[L] = aa.ul[L] + bb.ul[L];
	sum.ul[H] = aa.ul[H] + bb.ul[H] + (sum.ul[L] < bb.ul[L]);
	return (sum.q);
}

/*
 * Shift a (signed) quad value left (arithmetic shift left).
 * This is the same as logical shift left!
 */
quad_t
__ashldi3(quad_t a, qshift_t shift)
{
	union uu aa;

	aa.q = a;
	if (shift >= LONG_BITS) {
		aa.ul[H] = shift >= QUAD_BITS ? 0 :
		    aa.ul[L] << (shift - LONG_BITS);
		aa.ul[L] = 0;
	} else if (shift > 0) {
		aa.ul[H] = (aa.ul[H] << shift) |
		    (aa.ul[L] >> (LONG_BITS - shift));
		aa.ul[L] <<= shift;
	}
	return (aa.q);
}

/*
 * Shift an (unsigned) quad value left (logical shift left).
 * This is the same as arithmetic shift left!
 */
quad_t
__lshldi3(quad_t a, qshift_t shift)
{
	union uu aa;

	aa.q = a;
	if (shift >= LONG_BITS) {
		aa.ul[H] = shift >= QUAD_BITS ? 0 :
		    aa.ul[L] << (shift - LONG_BITS);
		aa.ul[L] = 0;
	} else if (shift > 0) {
		aa.ul[H] = (aa.ul[H] << shift) |
		    (aa.ul[L] >> (LONG_BITS - shift));
		aa.ul[L] <<= shift;
	}
	return (aa.q);
}

/*
 * Convert (signed) quad to double.
 */
double
__floatdidf(quad_t x)
{
	double d;
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
	 * has the units.  Ideally we could just set d, add LONG_BITS to
	 * its exponent, and then add the units, but this is portable
	 * code and does not know how to get at an exponent.  Machine-
	 * specific code may be able to do this more efficiently.
	 */
	d = (double)u.ul[H] * ((1L << (LONG_BITS - 2)) * 4.0);
	d += u.ul[L];

	return (neg ? -d : d);
}

} // namespace pbsd::lib_libc_quad::b0040
