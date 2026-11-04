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
 * The IEEEd2bits definition below is taken verbatim from
 * lib/libc/include/fpmath.h, which rand48.h includes:
 *
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

module;

#include <cstdint>

export module pbsd.lib.libc.gen.b0015;

export namespace pbsd::lib_libc_gen::b0015 {

using uint48 = std::uint64_t;

inline constexpr unsigned int RAND48_SEED_0 = 0x330e;
inline constexpr unsigned int RAND48_SEED_1 = 0xabcd;
inline constexpr unsigned int RAND48_SEED_2 = 0x1234;
inline constexpr unsigned int RAND48_MULT_0 = 0xe66d;
inline constexpr unsigned int RAND48_MULT_1 = 0xdeec;
inline constexpr unsigned int RAND48_MULT_2 = 0x0005;
inline constexpr unsigned int RAND48_ADD = 0x000b;

constexpr uint48
touint48(unsigned int x, unsigned int y, unsigned int z) noexcept
{
	return static_cast<uint48>(x) + (static_cast<uint48>(y) << 16) +
	    (static_cast<uint48>(z) << 32);
}

inline constexpr uint48 RAND48_SEED =
    touint48(RAND48_SEED_0, RAND48_SEED_1, RAND48_SEED_2);
inline constexpr uint48 RAND48_MULT =
    touint48(RAND48_MULT_0, RAND48_MULT_1, RAND48_MULT_2);

/*
 * From lib/libc/gen/_rand48.c; the state _DORAND48 advances and the
 * multiplier/addend erand48-family callers may replace via lcong48().
 */
uint48 _rand48_seed = RAND48_SEED;
uint48 _rand48_mult = RAND48_MULT;
uint48 _rand48_add = RAND48_ADD;

union IEEEd2bits {
	double	d;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
#else /* _BIG_ENDIAN */
		unsigned int	sign	:1;
		unsigned int	exp	:11;
		unsigned int	manh	:20;
		unsigned int	manl	:32;
#endif
	} bits;
};

long
lrand48() noexcept
{
	_rand48_seed = _rand48_seed * _rand48_mult + _rand48_add;
	return static_cast<long>((_rand48_seed >> 17) & 0x7fffffff);
}

/*
 * Optimization for speed: assume doubles are IEEE 754 and use bit fiddling
 * rather than converting to double.  Specifically, clamp the result to 48 bits
 * and convert to a double in [0.0, 1.0) via division by 2^48.  Normalize by
 * shifting the most significant bit into the implicit one position and
 * adjusting the exponent accordingly.  The store to the exponent field
 * overwrites the implicit one.
 */
double
drand48() noexcept
{
	union {
		union IEEEd2bits ieee;
		std::uint64_t u64;
	} u;
	int s;

	_rand48_seed = _rand48_seed * _rand48_mult + _rand48_add;

	u.u64 = (_rand48_seed & 0xffffffffffffULL);
	if (u.u64 == 0)
		return (0.0);
	u.u64 <<= 5;
	for (s = 0; !(u.u64 & (1LL << 52)); s++, u.u64 <<= 1)
		;
	u.ieee.bits.exp = 1022 - s;
	return (u.ieee.d);
}

} /* namespace pbsd::lib_libc_gen::b0015 */
