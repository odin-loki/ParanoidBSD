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
 * Reference oracle for batch b0033: the concatenated originals of
 *
 *	lib/libc/quad/iordi3.c
 *	lib/libc/quad/negdi2.c
 *	lib/libc/quad/notdi2.c
 *	lib/libc/quad/floatunsdidf.c
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
/* lib/libc/quad/iordi3.c                                             */
/* ------------------------------------------------------------------ */

/*
 * Return a | b, in quad.
 */
quad_t
ref___iordi3(quad_t a, quad_t b)
{
	union uu aa, bb;

	aa.q = a;
	bb.q = b;
	aa.ul[0] |= bb.ul[0];
	aa.ul[1] |= bb.ul[1];
	return (aa.q);
}

/* ------------------------------------------------------------------ */
/* lib/libc/quad/negdi2.c                                             */
/* ------------------------------------------------------------------ */

/*
 * Return -a (or, equivalently, 0 - a), in quad.  See subdi3.c.
 */
quad_t
ref___negdi2(quad_t a)
{
	union uu aa, res;

	aa.q = a;
	res.ul[L] = -aa.ul[L];
	res.ul[H] = -aa.ul[H] - (res.ul[L] > 0);
	return (res.q);
}

/* ------------------------------------------------------------------ */
/* lib/libc/quad/notdi2.c                                             */
/* ------------------------------------------------------------------ */

/*
 * Return ~a.  For some reason gcc calls this `one's complement' rather
 * than `not'.
 */
quad_t
ref___one_cmpldi2(quad_t a)
{
	union uu aa;

	aa.q = a;
	aa.ul[0] = ~aa.ul[0];
	aa.ul[1] = ~aa.ul[1];
	return (aa.q);
}

/* ------------------------------------------------------------------ */
/* lib/libc/quad/floatunsdidf.c                                       */
/* ------------------------------------------------------------------ */

/*
 * Convert (unsigned) quad to double.
 * This is exactly like floatdidf.c except that negatives never occur.
 */
double
ref___floatunsdidf(u_quad_t x)
{
	double d;
	union uu u;

	u.uq = x;
	d = (double)u.ul[H] * ((1L << (LONG_BITS - 2)) * 4.0);
	d += u.ul[L];
	return (d);
}
