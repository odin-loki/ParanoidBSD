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
 * Reference oracle for batch b0034: the concatenated originals of
 *
 *	lib/libc/quad/ucmpdi2.c
 *	lib/libc/quad/cmpdi2.c
 *	lib/libc/quad/subdi3.c
 *
 * each of which carries the identical copyright notice reproduced above,
 * as does the private header lib/libc/quad/quad.h that they all include.
 * The only edits are the `ref_' prefix on each function name and the
 * preamble below, which supplies the declarations "quad.h" would have
 * supplied.  No function body has been touched.
 */

#include <limits.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* lib/libc/quad/quad.h, and the <sys/types.h> names it relies on.    */
/* ------------------------------------------------------------------ */

typedef long long quad_t;
typedef unsigned long long u_quad_t;

/*
 * quad.h states as a standing assumption that "a quad variable is exactly
 * twice as long as `long'".  union uu is only a valid decomposition of a
 * quad while that holds, so the halfword type is spelled explicitly at 32
 * bits here -- which is what `long' is on the machines this directory is
 * compiled for.
 */
typedef int32_t quad_long;
typedef uint32_t quad_u_long;

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
#define	H		0	/* _QUAD_HIGHWORD */
#define	L		1	/* _QUAD_LOWWORD */
#else
#define	H		1	/* _QUAD_HIGHWORD */
#define	L		0	/* _QUAD_LOWWORD */
#endif

#define	QUAD_BITS	(sizeof(quad_t) * CHAR_BIT)
#define	LONG_BITS	(sizeof(quad_long) * CHAR_BIT)
#define	HALF_BITS	(sizeof(quad_long) * CHAR_BIT / 2)

#define	HHALF(x)	((x) >> HALF_BITS)
#define	LHALF(x)	((x) & ((1L << HALF_BITS) - 1))
#define	LHUP(x)		((x) << HALF_BITS)

/* ------------------------------------------------------------------ */
/* lib/libc/quad/ucmpdi2.c                                            */
/* ------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------ */
/* lib/libc/quad/cmpdi2.c                                             */
/* ------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------ */
/* lib/libc/quad/subdi3.c                                             */
/* ------------------------------------------------------------------ */

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
