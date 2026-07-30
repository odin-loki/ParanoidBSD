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

/*
 * C++23 module port of HardenedBSD
 *	lib/libc/gen/_rand48.c
 *	lib/libc/gen/nrand48.c
 *	lib/libc/gen/erand48.c
 *	lib/libc/gen/jrand48.c
 *
 * The private header lib/libc/gen/rand48.h and the union IEEEd2bits layout
 * from lib/libc/include/fpmath.h are reproduced here so that the function
 * bodies stay byte-for-byte what they were.  The 64-bit wrap-around of
 * _DORAND48, the truncation to 48 bits in STORERAND48, the ERAND48_END bit
 * fiddling and jrand48's (int) cast (which sign-extends into the returned
 * long) are all preserved exactly.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.gen.b0013;

#define	RAND48_SEED_0	(0x330e)
#define	RAND48_SEED_1	(0xabcd)
#define	RAND48_SEED_2	(0x1234)
#define	RAND48_MULT_0	(0xe66d)
#define	RAND48_MULT_1	(0xdeec)
#define	RAND48_MULT_2	(0x0005)
#define	RAND48_ADD	(0x000b)

#define	TOUINT48(x, y, z)						\
	((uint48)(x) + (((uint48)(y)) << 16) + (((uint48)(z)) << 32))

#define	RAND48_SEED	TOUINT48(RAND48_SEED_0, RAND48_SEED_1, RAND48_SEED_2)
#define	RAND48_MULT	TOUINT48(RAND48_MULT_0, RAND48_MULT_1, RAND48_MULT_2)

#define	LOADRAND48(l, x) do {						\
	(l) = TOUINT48((x)[0], (x)[1], (x)[2]);				\
} while (0)

#define	STORERAND48(l, x) do {						\
	(x)[0] = (unsigned short)(l);					\
	(x)[1] = (unsigned short)((l) >> 16);				\
	(x)[2] = (unsigned short)((l) >> 32);				\
} while (0)

#define	_DORAND48(l) do {						\
	(l) = (l) * _rand48_mult + _rand48_add;				\
} while (0)

#define	DORAND48(l, x) do {						\
	LOADRAND48(l, x);						\
	_DORAND48(l);							\
	STORERAND48(l, x);						\
} while (0)

#define	ERAND48_BEGIN							\
	union {								\
		union IEEEd2bits ieee;					\
		std::uint64_t u64;					\
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

export namespace pbsd::lib_libc_gen::b0013 {

using uint48 = std::uint64_t;

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
union IEEEd2bits {
	double	d;
	struct {
		unsigned int	sign	:1;
		unsigned int	exp	:11;
		unsigned int	manh	:20;
		unsigned int	manl	:32;
	} bits;
};
#else /* _LITTLE_ENDIAN */
union IEEEd2bits {
	double	d;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
	} bits;
};
#endif

/* lib/libc/gen/_rand48.c */

uint48 _rand48_seed = RAND48_SEED;
uint48 _rand48_mult = RAND48_MULT;
uint48 _rand48_add = RAND48_ADD;

/* lib/libc/gen/nrand48.c */

long
nrand48(unsigned short xseed[3])
{
	uint48 tmp;

	DORAND48(tmp, xseed);
	return ((tmp >> 17) & 0x7fffffff);
}

/* lib/libc/gen/erand48.c */

double
erand48(unsigned short xseed[3])
{
	uint48 tmp;

	ERAND48_BEGIN;
	DORAND48(tmp, xseed);
	ERAND48_END(tmp);
}

/* lib/libc/gen/jrand48.c */

long
jrand48(unsigned short xseed[3])
{
	uint48 tmp;

	DORAND48(tmp, xseed);
	return ((int)((tmp >> 16) & 0xffffffff));
}

} /* namespace pbsd::lib_libc_gen::b0013 */
