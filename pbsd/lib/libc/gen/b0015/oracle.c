/*
 * Reference oracle for batch b0015.  The function bodies below are the
 * unmodified bodies of lib/libc/gen/lrand48.c and lib/libc/gen/drand48.c;
 * only the function names carry a ref_ prefix.  The supporting macros and
 * types are reproduced from lib/libc/gen/rand48.h, lib/libc/gen/_rand48.c
 * and lib/libc/include/fpmath.h so that this file is self-contained.
 */

/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 Mike Barcroft <mike@FreeBSD.org>
 * Copyright (c) 2002 David Schultz <das@FreeBSD.ORG>
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

#include <stdint.h>
#include <math.h>
#include <stdlib.h>

/* Missing defines: FreeBSD's <sys/endian.h> spellings. */
#ifndef _LITTLE_ENDIAN
#define	_LITTLE_ENDIAN	__ORDER_LITTLE_ENDIAN__
#endif
#ifndef _BIG_ENDIAN
#define	_BIG_ENDIAN	__ORDER_BIG_ENDIAN__
#endif
#ifndef _BYTE_ORDER
#define	_BYTE_ORDER	__BYTE_ORDER__
#endif

/* --- lib/libc/include/fpmath.h --- */

#ifndef _IEEE_WORD_ORDER
#define	_IEEE_WORD_ORDER	_BYTE_ORDER
#endif

#define	DBL_MANH_SIZE	20
#define	DBL_MANL_SIZE	32

union IEEEd2bits {
	double	d;
	struct {
#if _BYTE_ORDER == _LITTLE_ENDIAN
#if _IEEE_WORD_ORDER == _LITTLE_ENDIAN
		unsigned int	manl	:32;
#endif
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
#if _IEEE_WORD_ORDER == _BIG_ENDIAN
		unsigned int	manl	:32;
#endif
#else /* _BIG_ENDIAN */
		unsigned int	sign	:1;
		unsigned int	exp	:11;
		unsigned int	manh	:20;
		unsigned int	manl	:32;
#endif
	} bits;
};

/* --- lib/libc/gen/rand48.h --- */

#define	RAND48_SEED_0	(0x330e)
#define	RAND48_SEED_1	(0xabcd)
#define	RAND48_SEED_2	(0x1234)
#define	RAND48_MULT_0	(0xe66d)
#define	RAND48_MULT_1	(0xdeec)
#define	RAND48_MULT_2	(0x0005)
#define	RAND48_ADD	(0x000b)

typedef uint64_t uint48;

#define	TOUINT48(x, y, z)						\
	((uint48)(x) + (((uint48)(y)) << 16) + (((uint48)(z)) << 32))

#define	RAND48_SEED	TOUINT48(RAND48_SEED_0, RAND48_SEED_1, RAND48_SEED_2)
#define	RAND48_MULT	TOUINT48(RAND48_MULT_0, RAND48_MULT_1, RAND48_MULT_2)

#define	_DORAND48(l) do {						\
	(l) = (l) * _rand48_mult + _rand48_add;				\
} while (0)

#define	ERAND48_BEGIN							\
	union {								\
		union IEEEd2bits ieee;					\
		uint64_t u64;						\
	} u;								\
	int s

/*
 * Optimization for speed: assume doubles are IEEE 754 and use bit fiddling
 * rather than converting to double.  Specifically, clamp the result to 48 bits
 * and convert to a double in [0.0, 1.0) via division by 2^48.  Normalize by
 * shifting the most significant bit into the implicit one position and
 * adjusting the exponent accordingly.  The store to the exponent field
 * overwrites the implicit one.
 */
#define	ERAND48_END(x) do {						\
	u.u64 = ((x) & 0xffffffffffffULL);				\
	if (u.u64 == 0)							\
		return (0.0);						\
	u.u64 <<= 5;							\
	for (s = 0; !(u.u64 & (1LL << 52)); s++, u.u64 <<= 1)		\
		;							\
	u.ieee.bits.exp = 1022 - s;					\
	return (u.ieee.d);						\
} while (0)

/* --- lib/libc/gen/_rand48.c --- */

uint48 _rand48_seed = RAND48_SEED;
uint48 _rand48_mult = RAND48_MULT;
uint48 _rand48_add = RAND48_ADD;

/* --- lib/libc/gen/lrand48.c --- */

long
ref_lrand48(void)
{
	_DORAND48(_rand48_seed);
	return (_rand48_seed >> 17) & 0x7fffffff;
}

/* --- lib/libc/gen/drand48.c --- */

double
ref_drand48(void)
{
	ERAND48_BEGIN;
	_DORAND48(_rand48_seed);
	ERAND48_END(_rand48_seed);
}
