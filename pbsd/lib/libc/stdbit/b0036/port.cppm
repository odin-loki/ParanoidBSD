/*
 * PBSD port of HardenedBSD lib/libc/stdbit sources (batch b0036).
 *
 * Sources ported here, verbatim in behaviour:
 *   lib/libc/stdbit/stdc_trailing_ones.c
 *   lib/libc/stdbit/stdc_trailing_zeros.c
 *   lib/libc/stdbit/stdc_leading_ones.c
 */

module;

#include <climits>

/*
 * The *_WIDTH macros are C23/C++23 additions; provide them when the hosted
 * <climits> of this toolchain does not.  Values are identical to the ones the
 * original sources rely upon.
 */
#ifndef UCHAR_WIDTH
#define UCHAR_WIDTH CHAR_BIT
#endif
#ifndef USHRT_WIDTH
#define USHRT_WIDTH ((int)(sizeof(unsigned short) * CHAR_BIT))
#endif
#ifndef UINT_WIDTH
#define UINT_WIDTH ((int)(sizeof(unsigned int) * CHAR_BIT))
#endif
#ifndef ULONG_WIDTH
#define ULONG_WIDTH ((int)(sizeof(unsigned long) * CHAR_BIT))
#endif
#ifndef ULLONG_WIDTH
#define ULLONG_WIDTH ((int)(sizeof(unsigned long long) * CHAR_BIT))
#endif

export module pbsd.lib.libc.stdbit.b0036;

export namespace pbsd::lib_libc_stdbit::b0036 {

/*
 * ===========================================================================
 * lib/libc/stdbit/stdc_trailing_ones.c
 *
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 * ===========================================================================
 */

/* Avoid triggering undefined behavior if x == ~0. */
static_assert(UCHAR_WIDTH < UINT_WIDTH,
    "stdc_trailing_ones_uc needs UCHAR_WIDTH < UINT_WIDTH");

unsigned int
stdc_trailing_ones_uc(unsigned char x)
{
	return (__builtin_ctz(~x));
}

/* Avoid triggering undefined behavior if x == ~0. */
static_assert(USHRT_WIDTH < UINT_WIDTH,
    "stdc_trailing_ones_uc needs USHRT_WIDTH < UINT_WIDTH");

unsigned int
stdc_trailing_ones_us(unsigned short x)
{
	return (__builtin_ctz(~x));
}

unsigned int
stdc_trailing_ones_ui(unsigned int x)
{
	if (x == ~0U)
		return (UINT_WIDTH);

	return (__builtin_ctz(~x));
}

unsigned int
stdc_trailing_ones_ul(unsigned long x)
{
	if (x == ~0UL)
		return (ULONG_WIDTH);

	return (__builtin_ctzl(~x));
}

unsigned int
stdc_trailing_ones_ull(unsigned long long x)
{
	if (x == ~0ULL)
		return (ULLONG_WIDTH);

	return (__builtin_ctzll(~x));
}

/*
 * ===========================================================================
 * lib/libc/stdbit/stdc_trailing_zeros.c
 *
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 * ===========================================================================
 */

/* Ensure we do not shift 1U out of range. */
static_assert(UCHAR_WIDTH < UINT_WIDTH,
    "stdc_trailing_zeros_uc needs UCHAR_WIDTH < UINT_WIDTH");

unsigned int
stdc_trailing_zeros_uc(unsigned char x)
{
	return (__builtin_ctz(x | 1U << UCHAR_WIDTH));
}

/* Ensure we do not shift 1U out of range. */
static_assert(USHRT_WIDTH < UINT_WIDTH,
    "stdc_trailing_zeros_uc needs USHRT_WIDTH < UINT_WIDTH");

unsigned int
stdc_trailing_zeros_us(unsigned short x)
{
	return (__builtin_ctz(x | 1U << USHRT_WIDTH));
}

unsigned int
stdc_trailing_zeros_ui(unsigned int x)
{
	if (x == 0U)
		return (UINT_WIDTH);

	return (__builtin_ctz(x));
}

unsigned int
stdc_trailing_zeros_ul(unsigned long x)
{
	if (x == 0UL)
		return (ULONG_WIDTH);

	return (__builtin_ctzl(x));
}

unsigned int
stdc_trailing_zeros_ull(unsigned long long x)
{
	if (x == 0ULL)
		return (ULLONG_WIDTH);

	return (__builtin_ctzll(x));
}

/*
 * ===========================================================================
 * lib/libc/stdbit/stdc_leading_ones.c
 *
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 * ===========================================================================
 */

/* Avoid triggering undefined behavior if x == 0. */
static_assert(UCHAR_WIDTH < UINT_WIDTH,
    "stdc_leading_ones_uc needs UCHAR_WIDTH < UINT_WIDTH");

unsigned int
stdc_leading_ones_uc(unsigned char x)
{
	const int offset = UINT_WIDTH - UCHAR_WIDTH;

	return (__builtin_clz(~(x << offset)));
}

/* Avoid triggering undefined behavior if x == 0. */
static_assert(USHRT_WIDTH < UINT_WIDTH,
    "stdc_leading_ones_us needs USHRT_WIDTH < UINT_WIDTH");

unsigned int
stdc_leading_ones_us(unsigned short x)
{
	const int offset = UINT_WIDTH - USHRT_WIDTH;

	return (__builtin_clz(~(x << offset)));
}

unsigned int
stdc_leading_ones_ui(unsigned int x)
{
	if (x == ~0U)
		return (UINT_WIDTH);

	return (__builtin_clz(~x));
}

unsigned int
stdc_leading_ones_ul(unsigned long x)
{
	if (x == ~0UL)
		return (ULONG_WIDTH);

	return (__builtin_clzl(~x));
}

unsigned int
stdc_leading_ones_ull(unsigned long long x)
{
	if (x == ~0ULL)
		return (ULLONG_WIDTH);

	return (__builtin_clzll(~x));
}

} /* namespace pbsd::lib_libc_stdbit::b0036 */
