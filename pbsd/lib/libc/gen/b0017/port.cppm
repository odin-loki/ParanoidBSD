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

module;

#include <cstdint>

export module pbsd.lib.libc.gen.b0017;

export namespace pbsd::lib_libc_gen::b0017 {

/*
 * Transliterated from lib/libc/gen/rand48.h.  The macros become constexpr
 * functions; argument conversion to uint48 reproduces the casts the macros
 * performed, so the arithmetic (and its wrapping) is unchanged.
 */

using uint48 = std::uint64_t;

inline constexpr unsigned int RAND48_SEED_0 = 0x330e;
inline constexpr unsigned int RAND48_SEED_1 = 0xabcd;
inline constexpr unsigned int RAND48_SEED_2 = 0x1234;
inline constexpr unsigned int RAND48_MULT_0 = 0xe66d;
inline constexpr unsigned int RAND48_MULT_1 = 0xdeec;
inline constexpr unsigned int RAND48_MULT_2 = 0x0005;
inline constexpr unsigned int RAND48_ADD = 0x000b;

constexpr uint48
TOUINT48(uint48 x, uint48 y, uint48 z) noexcept
{
	return ((uint48)(x) + (((uint48)(y)) << 16) + (((uint48)(z)) << 32));
}

inline constexpr uint48 RAND48_SEED =
    TOUINT48(RAND48_SEED_0, RAND48_SEED_1, RAND48_SEED_2);
inline constexpr uint48 RAND48_MULT =
    TOUINT48(RAND48_MULT_0, RAND48_MULT_1, RAND48_MULT_2);

/* from lib/libc/gen/_rand48.c */
uint48 _rand48_seed = RAND48_SEED;
uint48 _rand48_mult = RAND48_MULT;
uint48 _rand48_add = RAND48_ADD;

void
LOADRAND48(uint48 &l, const unsigned short *x) noexcept
{
	(l) = TOUINT48((x)[0], (x)[1], (x)[2]);
}

void
STORERAND48(uint48 l, unsigned short *x) noexcept
{
	(x)[0] = (unsigned short)(l);
	(x)[1] = (unsigned short)((l) >> 16);
	(x)[2] = (unsigned short)((l) >> 32);
}

void
_DORAND48(uint48 &l) noexcept
{
	(l) = (l) * _rand48_mult + _rand48_add;
}

/* lib/libc/gen/mrand48.c */
long
mrand48(void)
{
	_DORAND48(_rand48_seed);
	return ((int)((_rand48_seed >> 16) & 0xffffffff));
}

/* lib/libc/gen/lcong48.c */
void
lcong48(unsigned short p[7])
{
	LOADRAND48(_rand48_seed, &p[0]);
	LOADRAND48(_rand48_mult, &p[3]);
	_rand48_add = p[6];
}

/* lib/libc/gen/srand48.c */
void
srand48(long seed)
{
	_rand48_seed = TOUINT48(RAND48_SEED_0, (unsigned short)seed,
	    (unsigned short)(seed >> 16));
	_rand48_mult = RAND48_MULT;
	_rand48_add = RAND48_ADD;
}

/* lib/libc/gen/seed48.c */
unsigned short *
seed48(unsigned short xseed[3])
{
	static unsigned short sseed[3];

	STORERAND48(_rand48_seed, sseed);
	LOADRAND48(_rand48_seed, xseed);
	_rand48_mult = RAND48_MULT;
	_rand48_add = RAND48_ADD;
	return (sseed);
}

} /* namespace pbsd::lib_libc_gen::b0017 */
