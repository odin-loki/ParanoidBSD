/*
 * PBSD migration batch b0049 -- C++23 port of HardenedBSD
 * lib/libc/softfloat/{negtf2,unorddf2,unordsf2,fpgetmask}.c
 *
 * The softfloat comparison primitives float32_eq() and float64_eq() are not
 * part of this batch; as in the original C sources they are resolved at link
 * time against the softfloat runtime.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.softfloat.b0049;

extern "C" {
int float32_eq(unsigned int, unsigned int);
int float64_eq(unsigned long long, unsigned long long);
extern int float_exception_mask;
}

export namespace pbsd::lib_libc_softfloat::b0049 {

/* milieu.h / softfloat.h (bits64 configuration, FLOAT128 enabled) */
using flag = int;
using bits64 = std::uint64_t;
using float32 = unsigned int;
using float64 = unsigned long long;

struct float128 {
	bits64 low;
	bits64 high;
};

/*
 * In the bits64 configuration float64 already is a 64-bit integer, so
 * mangling between bit pattern and float64 is the identity.
 */
constexpr float64
FLOAT64_MANGLE(std::uint64_t a) noexcept
{

	return a;
}

/* $NetBSD: negtf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

float128 __negtf2(float128);

float128
__negtf2(float128 a)
{

	/* libgcc1.c says -a */
	a.high ^= FLOAT64_MANGLE(0x8000000000000000ULL);
	return a;
}

/* $NetBSD: unorddf2.c,v 1.1 2003/05/06 08:58:19 rearnsha Exp $ */

/*
 * Written by Richard Earnshaw, 2003.  This file is in the Public Domain.
 */

flag __unorddf2(float64, float64);

flag
__unorddf2(float64 a, float64 b)
{
	/*
	 * The comparison is unordered if either input is a NaN.
	 * Test for this by comparing each operand with itself.
	 * We must perform both comparisons to correctly check for
	 * signalling NaNs.
	 */
	return 1 ^ (float64_eq(a, a) & float64_eq(b, b));
}

/* $NetBSD: unordsf2.c,v 1.1 2003/05/06 08:58:20 rearnsha Exp $ */

/*
 * Written by Richard Earnshaw, 2003.  This file is in the Public Domain.
 */

flag __unordsf2(float32, float32);

flag
__unordsf2(float32 a, float32 b)
{
	/*
	 * The comparison is unordered if either input is a NaN.
	 * Test for this by comparing each operand with itself.
	 * We must perform both comparisons to correctly check for
	 * signalling NaNs.
	 */
	return 1 ^ (float32_eq(a, a) & float32_eq(b, b));
}

/* $NetBSD: fpgetmask.c,v 1.4 2008/04/28 20:23:00 martin Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Neil A. Carson and Mark Brinicombe
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

using fp_except = int;

fp_except
fpgetmask(void)
{
	return float_exception_mask;
}

} /* namespace pbsd::lib_libc_softfloat::b0049 */
