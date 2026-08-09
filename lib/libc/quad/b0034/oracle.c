/*
 * oracle.c -- reference implementation for batch b0034.
 *
 * The original HardenedBSD sources
 *	lib/libc/quad/ucmpdi2.c
 *	lib/libc/quad/cmpdi2.c
 *	lib/libc/quad/subdi3.c
 * concatenated, with each function renamed with a `ref_' prefix.  The function
 * bodies are UNMODIFIED.  The only additions are the type/macro definitions
 * that lib/libc/quad/quad.h would otherwise supply, reproduced below so this
 * file is self-contained on a hosted C11 toolchain.
 */

#include <stdint.h>

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

/* ------------------------------------------------------------------------
 * lib/libc/quad/quad.h (the parts these three files use)
 * ------------------------------------------------------------------------ */

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
 * The machine model used here is the one HardenedBSD actually builds
 * lib/libc/quad for: a 64-bit quad_t made of two 32-bit longs.  The halves are
 * spelled with fixed-width types of the target's `long' width so the union
 * stays coherent no matter what the host's `long' happens to be.
 */
typedef int64_t		quad_t;
typedef uint64_t	u_quad_t;
typedef int32_t		quad_long;	/* the target's `long' */
typedef uint32_t	quad_u_long;	/* the target's `u_long' */

/*
 * Depending on the desired operation, we view a `long long' (aka quad_t) in
 * one or more of the following formats.
 */
union uu {
	quad_t	q;		/* as a (signed) quad */
	quad_t	uq;		/* as an unsigned quad */
	quad_long	sl[2];		/* as two signed longs */
	quad_u_long	ul[2];		/* as two unsigned longs */
};

/*
 * Define high and low longwords, as machine/endian.h does.
 */
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define	_QUAD_HIGHWORD	0
#define	_QUAD_LOWWORD	1
#else
#define	_QUAD_HIGHWORD	1
#define	_QUAD_LOWWORD	0
#endif

#define	H		_QUAD_HIGHWORD
#define	L		_QUAD_LOWWORD

/*
 * Total number of bits in a quad_t and in the pieces that make it up.
 */
#define	CHAR_BIT_	8
#define	QUAD_BITS	(sizeof(quad_t) * CHAR_BIT_)
#define	LONG_BITS	(sizeof(quad_long) * CHAR_BIT_)
#define	HALF_BITS	(sizeof(quad_long) * CHAR_BIT_ / 2)

int		ref___cmpdi2(quad_t a, quad_t b);
int		ref___ucmpdi2(u_quad_t a, u_quad_t b);
quad_t		ref___subdi3(quad_t a, quad_t b);

/* ------------------------------------------------------------------------
 * lib/libc/quad/ucmpdi2.c
 * ------------------------------------------------------------------------ */

/*
 * Return 0, 1, or 2 as a <, =, > b respectively.
 * Neither a nor b are considered signed.
 */
int
ref___ucmpdi2(u_quad_t a, u_quad_t b)
{
	union uu aa, bb;

	aa.uq = a;
	bb.uq = b;
	return (aa.ul[H] < bb.ul[H] ? 0 : aa.ul[H] > bb.ul[H] ? 2 :
	    aa.ul[L] < bb.ul[L] ? 0 : aa.ul[L] > bb.ul[L] ? 2 : 1);
}

/* ------------------------------------------------------------------------
 * lib/libc/quad/cmpdi2.c
 * ------------------------------------------------------------------------ */

/*
 * Return 0, 1, or 2 as a <, =, > b respectively.
 * Both a and b are considered signed---which means only the high word is
 * signed.
 */
int
ref___cmpdi2(quad_t a, quad_t b)
{
	union uu aa, bb;

	aa.q = a;
	bb.q = b;
	return (aa.sl[H] < bb.sl[H] ? 0 : aa.sl[H] > bb.sl[H] ? 2 :
	    aa.ul[L] < bb.ul[L] ? 0 : aa.ul[L] > bb.ul[L] ? 2 : 1);
}

/* ------------------------------------------------------------------------
 * lib/libc/quad/subdi3.c
 * ------------------------------------------------------------------------ */

/*
 * Subtract two quad values.  This is trivial since a one-bit carry
 * from a single u_long difference x-y occurs if and only if (x-y) > x.
 */
quad_t
ref___subdi3(quad_t a, quad_t b)
{
	union uu aa, bb, diff;

	aa.q = a;
	bb.q = b;
	diff.ul[L] = aa.ul[L] - bb.ul[L];
	diff.ul[H] = aa.ul[H] - bb.ul[H] - (diff.ul[L] > aa.ul[L]);
	return (diff.q);
}
