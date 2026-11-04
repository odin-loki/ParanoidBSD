/*
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <limits.h>
#if defined(__has_include)
#if __has_include(<stdbit.h>)
#include <stdbit.h>
#endif
#endif

#ifndef LONG_BIT
#define LONG_BIT (CHAR_BIT * (int)sizeof(long))
#endif

/* ---- lib/libc/stdbit/stdc_count_ones.c ---- */

unsigned int
ref_stdc_count_ones_uc(unsigned char x)
{
	return (__builtin_popcount(x));
}

unsigned int
ref_stdc_count_ones_us(unsigned short x)
{
	return (__builtin_popcount(x));
}

unsigned int
ref_stdc_count_ones_ui(unsigned int x)
{
	return (__builtin_popcount(x));
}

unsigned int
ref_stdc_count_ones_ul(unsigned long x)
{
	return (__builtin_popcountl(x));
}

unsigned int
ref_stdc_count_ones_ull(unsigned long long x)
{
	return (__builtin_popcountll(x));
}

/* ---- lib/libc/stdbit/stdc_count_zeros.c ---- */

unsigned int
ref_stdc_count_zeros_uc(unsigned char x)
{
	return (__builtin_popcount(x ^ UCHAR_MAX));
}

unsigned int
ref_stdc_count_zeros_us(unsigned short x)
{
	return (__builtin_popcount(x ^ USHRT_MAX));
}

unsigned int
ref_stdc_count_zeros_ui(unsigned int x)
{
	return (__builtin_popcount(~x));
}

unsigned int
ref_stdc_count_zeros_ul(unsigned long x)
{
	return (__builtin_popcountl(~x));
}

unsigned int
ref_stdc_count_zeros_ull(unsigned long long x)
{
	return (__builtin_popcountll(~x));
}
