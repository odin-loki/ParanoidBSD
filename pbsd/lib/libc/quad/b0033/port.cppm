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
 * PBSD port of batch b0033.  The four source files
 *
 *	lib/libc/quad/iordi3.c
 *	lib/libc/quad/negdi2.c
 *	lib/libc/quad/notdi2.c
 *	lib/libc/quad/floatunsdidf.c
 *
 * each carry the identical copyright notice reproduced above, as does the
 * private header lib/libc/quad/quad.h whose declarations are reproduced
 * below.
 */

module;

#include <climits>
#include <cstdint>

export module pbsd.lib.libc.quad.b0033;

export namespace pbsd::lib_libc_quad::b0033 {

/*
 * <sys/types.h> spellings the quad library operates on.
 */
typedef long long quad_t;
typedef unsigned long long u_quad_t;

} // namespace pbsd::lib_libc_quad::b0033

namespace pbsd::lib_libc_quad::b0033 {

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
 * This library can provide 128-bit arithmetic on a machine with 128-bit
 * quads and 64-bit longs, for instance, or 96-bit arithmetic on machines
 * with 48-bit longs.
 *
 * The second assumption is a hard precondition of every routine below:
 * `union uu' is only a valid decomposition of a quad while the two halves
 * exactly tile it.  It is honoured here by spelling the halfword type
 * explicitly at 32 bits, which is what `long' is on the machines this
 * directory is compiled for (i386 and friends).  Widening the halfword to
 * a native LP64 `long' would leave ul[1] outside the quad and thus
 * uninitialised, which is not a behaviour worth reproducing.
 */
typedef std::int32_t quad_long;   /* the `long'  quad.h assumes */
typedef std::uint32_t quad_u_long; /* the `u_long' quad.h assumes */

/*
 * Depending on the desired operation, we view a `long long' (aka quad_t) in
 * one or more of the following formats.
 */
union uu {
	quad_t	q;		/* as a (signed) quad */
	quad_t	uq;		/* as an unsigned quad */
	quad_long	sl[2];	/* as two signed longs */
	quad_u_long	ul[2];	/* as two unsigned longs */
};

/*
 * Define high and low longwords.
 */
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
inline constexpr int H = 0;		/* _QUAD_HIGHWORD */
inline constexpr int L = 1;		/* _QUAD_LOWWORD */
#else
inline constexpr int H = 1;		/* _QUAD_HIGHWORD */
inline constexpr int L = 0;		/* _QUAD_LOWWORD */
#endif

/*
 * Total number of bits in a quad_t and in the pieces that make it up.
 * These are used for shifting, and also below for halfword extraction
 * and assembly.
 */
inline constexpr int QUAD_BITS = sizeof(quad_t) * CHAR_BIT;
inline constexpr int LONG_BITS = sizeof(quad_long) * CHAR_BIT;
inline constexpr int HALF_BITS = sizeof(quad_long) * CHAR_BIT / 2;

} // namespace pbsd::lib_libc_quad::b0033

export namespace pbsd::lib_libc_quad::b0033 {

/*
 * Return a | b, in quad.
 */
quad_t
__iordi3(quad_t a, quad_t b)
{
	union uu aa, bb;

	aa.q = a;
	bb.q = b;
	aa.ul[0] |= bb.ul[0];
	aa.ul[1] |= bb.ul[1];
	return (aa.q);
}

/*
 * Return -a (or, equivalently, 0 - a), in quad.  See subdi3.c.
 */
quad_t
__negdi2(quad_t a)
{
	union uu aa, res;

	aa.q = a;
	res.ul[L] = -aa.ul[L];
	res.ul[H] = -aa.ul[H] - (res.ul[L] > 0);
	return (res.q);
}

/*
 * Return ~a.  For some reason gcc calls this `one's complement' rather
 * than `not'.
 */
quad_t
__one_cmpldi2(quad_t a)
{
	union uu aa;

	aa.q = a;
	aa.ul[0] = ~aa.ul[0];
	aa.ul[1] = ~aa.ul[1];
	return (aa.q);
}

/*
 * Convert (unsigned) quad to double.
 * This is exactly like floatdidf.c except that negatives never occur.
 */
double
__floatunsdidf(u_quad_t x)
{
	double d;
	union uu u;

	u.uq = x;
	d = (double)u.ul[H] * ((1L << (LONG_BITS - 2)) * 4.0);
	d += u.ul[L];
	return (d);
}

} // namespace pbsd::lib_libc_quad::b0033
