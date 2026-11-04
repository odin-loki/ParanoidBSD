/*
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

module;

#include <limits.h>

export module pbsd.lib.libc.stdbit.b0026;

export namespace pbsd::lib_libc_stdbit::b0026 {

/* stdc_count_ones.c */

unsigned int
stdc_count_ones_uc(unsigned char x)
{
	return (__builtin_popcount(x));
}

unsigned int
stdc_count_ones_us(unsigned short x)
{
	return (__builtin_popcount(x));
}

unsigned int
stdc_count_ones_ui(unsigned int x)
{
	return (__builtin_popcount(x));
}

unsigned int
stdc_count_ones_ul(unsigned long x)
{
	return (__builtin_popcountl(x));
}

unsigned int
stdc_count_ones_ull(unsigned long long x)
{
	return (__builtin_popcountll(x));
}

/* stdc_count_zeros.c */

unsigned int
stdc_count_zeros_uc(unsigned char x)
{
	return (__builtin_popcount(x ^ UCHAR_MAX));
}

unsigned int
stdc_count_zeros_us(unsigned short x)
{
	return (__builtin_popcount(x ^ USHRT_MAX));
}

unsigned int
stdc_count_zeros_ui(unsigned int x)
{
	return (__builtin_popcount(~x));
}

unsigned int
stdc_count_zeros_ul(unsigned long x)
{
	return (__builtin_popcountl(~x));
}

unsigned int
stdc_count_zeros_ull(unsigned long long x)
{
	return (__builtin_popcountll(~x));
}

} /* namespace pbsd::lib_libc_stdbit::b0026 */
