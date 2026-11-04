/*
 * Reference oracle for batch b0050.
 *
 * Concatenation of:
 *   lib/libc/stdbit/stdc_leading_zeros.c
 *   lib/libc/stdbit/stdc_bit_ceil.c
 *
 * Every function carries a ref_ prefix; the bodies are unmodified.
 */

/*
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <limits.h>

/*
 * C11 <limits.h> predates the C23 *_WIDTH macros, so supply them when the
 * hosting libc does not.
 */
#ifndef UCHAR_WIDTH
#define	UCHAR_WIDTH	CHAR_BIT
#endif
#ifndef USHRT_WIDTH
#define	USHRT_WIDTH	(CHAR_BIT * (int)sizeof(unsigned short))
#endif
#ifndef UINT_WIDTH
#define	UINT_WIDTH	(CHAR_BIT * (int)sizeof(unsigned int))
#endif
#ifndef ULONG_WIDTH
#define	ULONG_WIDTH	(CHAR_BIT * (int)sizeof(unsigned long))
#endif
#ifndef LONG_BIT
#define	LONG_BIT	ULONG_WIDTH
#endif
#ifndef ULLONG_WIDTH
#define	ULLONG_WIDTH	(CHAR_BIT * (int)sizeof(unsigned long long))
#endif

/* ---- lib/libc/stdbit/stdc_leading_zeros.c ---- */

/* Offset must be greater than zero. */
static_assert(UCHAR_WIDTH < UINT_WIDTH,
    "stdc_leading_zeros_uc needs UCHAR_WIDTH < UINT_WIDTH");

unsigned int
ref_stdc_leading_zeros_uc(unsigned char x)
{
	const int offset = UINT_WIDTH - UCHAR_WIDTH;

	return (__builtin_clz((x << offset) + (1U << (offset - 1))));
}

/* Offset must be greater than zero. */
static_assert(USHRT_WIDTH < UINT_WIDTH,
    "stdc_leading_zeros_us needs USHRT_WIDTH < UINT_WIDTH");

unsigned int
ref_stdc_leading_zeros_us(unsigned short x)
{
	const int offset = UINT_WIDTH - USHRT_WIDTH;

	return (__builtin_clz((x << offset) + (1U << (offset - 1))));
}

unsigned int
ref_stdc_leading_zeros_ui(unsigned int x)
{
	if (x == 0)
		return (UINT_WIDTH);

	return (__builtin_clz(x));
}

unsigned int
ref_stdc_leading_zeros_ul(unsigned long x)
{
	if (x == 0)
		return (ULONG_WIDTH);

	return (__builtin_clzl(x));
}

unsigned int
ref_stdc_leading_zeros_ull(unsigned long long x)
{
	if (x == 0)
		return (ULLONG_WIDTH);

	return (__builtin_clzll(x));
}

/* ---- lib/libc/stdbit/stdc_bit_ceil.c ---- */

/*
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/* Ensure we don't shift 1U out of range. */
static_assert(UCHAR_WIDTH < UINT_WIDTH,
    "stdc_bit_ceil_uc needs UCHAR_WIDTH < UINT_WIDTH");

unsigned char
ref_stdc_bit_ceil_uc(unsigned char x)
{
	if (x <= 1)
		return (1);

	return (1U << (UINT_WIDTH - __builtin_clz(x - 1)));
}

/* Ensure we don't shift 1U out of range. */
static_assert(USHRT_WIDTH < UINT_WIDTH,
    "stdc_bit_ceil_us needs USHRT_WIDTH < UINT_WIDTH");

unsigned short
ref_stdc_bit_ceil_us(unsigned short x)
{
	if (x <= 1)
		return (1);

	return (1U << (UINT_WIDTH - __builtin_clz(x - 1)));
}

unsigned int
ref_stdc_bit_ceil_ui(unsigned int x)
{
	if (x <= 1)
		return (1);

	if (x > UINT_MAX/2 + 1)
		return (0);

	return (1U << (UINT_WIDTH - __builtin_clz(x - 1)));
}

unsigned long
ref_stdc_bit_ceil_ul(unsigned long x)
{
	if (x <= 1)
		return (1);

	if (x > ULONG_MAX/2 + 1)
		return (0);

	return (1UL << (ULONG_WIDTH - __builtin_clzl(x - 1)));
}

unsigned long long
ref_stdc_bit_ceil_ull(unsigned long long x)
{
	if (x <= 1)
		return (1);

	if (x > ULLONG_MAX/2 + 1)
		return (0);

	return (1ULL << (ULLONG_WIDTH - __builtin_clzll(x - 1)));
}
