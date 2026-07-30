/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
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

/*
 * drem() wrapper for remainder().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

/*
 * Port of:
 *	lib/msun/src/s_llround.c	(instantiates s_lround.c)
 *	lib/msun/src/s_llroundf.c	(instantiates s_lround.c)
 *	lib/msun/src/s_llroundl.c	(instantiates s_lround.c)
 *	lib/msun/src/w_drem.c
 *
 * s_llround.c, s_llroundf.c and s_llroundl.c contain no code of their own:
 * each is a set of #defines followed by #include "s_lround.c".  The three
 * instantiations of the s_lround.c template are reproduced below verbatim,
 * once per (type, roundit, dtype, DTYPE_MIN, DTYPE_MAX, fn) tuple, with the
 * template's file-scope constants given per-instantiation names so that the
 * three copies can coexist in one translation unit.  Nothing else is changed.
 */

module;

#include <limits.h>
#include <fenv.h>
#include <math.h>

export module pbsd.lib.msun.src.b0060;

namespace pbsd::lib_msun_src::b0060::detail {

/*
 * If type has more precision than dtype, the endpoints dtype_(min|max) are
 * of the form xxx.5; they are "out of range" because lround() rounds away
 * from 0.  On the other hand, if type has less precision than dtype, then
 * all values that are out of range are integral, so we might as well assume
 * that everything is in range.  At compile time, INRANGE(x) should reduce to
 * two floating-point comparisons in the former case, or TRUE otherwise.
 */

/* s_llround.c: type = double, dtype = long long, fn = llround */
namespace llround_inst {

using type = double;
using dtype = long long;

[[maybe_unused]] static const type type_min = (type)LLONG_MIN;
static const type type_max = (type)LLONG_MAX;
static const type dtype_min = (type)LLONG_MIN - 0.5;
static const type dtype_max = (type)LLONG_MAX + 0.5;

static bool
inrange(type x)
{

	return (dtype_max - type_max != 0.5 ||
	    ((x) > dtype_min && (x) < dtype_max));
}

} /* namespace llround_inst */

/* s_llroundf.c: type = float, dtype = long long, fn = llroundf */
namespace llroundf_inst {

using type = float;
using dtype = long long;

[[maybe_unused]] static const type type_min = (type)LLONG_MIN;
static const type type_max = (type)LLONG_MAX;
static const type dtype_min = (type)LLONG_MIN - 0.5;
static const type dtype_max = (type)LLONG_MAX + 0.5;

static bool
inrange(type x)
{

	return (dtype_max - type_max != 0.5 ||
	    ((x) > dtype_min && (x) < dtype_max));
}

} /* namespace llroundf_inst */

/* s_llroundl.c: type = long double, dtype = long long, fn = llroundl */
namespace llroundl_inst {

using type = long double;
using dtype = long long;

[[maybe_unused]] static const type type_min = (type)LLONG_MIN;
static const type type_max = (type)LLONG_MAX;
static const type dtype_min = (type)LLONG_MIN - 0.5;
static const type dtype_max = (type)LLONG_MAX + 0.5;

static bool
inrange(type x)
{

	return (dtype_max - type_max != 0.5 ||
	    ((x) > dtype_min && (x) < dtype_max));
}

} /* namespace llroundl_inst */

} /* namespace pbsd::lib_msun_src::b0060::detail */

export namespace pbsd::lib_msun_src::b0060 {

long long
llround(double x)
{

	if (detail::llround_inst::inrange(x)) {
		x = ::round(x);
		return ((long long)x);
	} else {
		::feraiseexcept(FE_INVALID);
		return (LLONG_MAX);
	}
}

long long
llroundf(float x)
{

	if (detail::llroundf_inst::inrange(x)) {
		x = ::roundf(x);
		return ((long long)x);
	} else {
		::feraiseexcept(FE_INVALID);
		return (LLONG_MAX);
	}
}

long long
llroundl(long double x)
{

	if (detail::llroundl_inst::inrange(x)) {
		x = ::roundl(x);
		return ((long long)x);
	} else {
		::feraiseexcept(FE_INVALID);
		return (LLONG_MAX);
	}
}

double
drem(double x, double y)
{
	return ::remainder(x, y);
}

} /* namespace pbsd::lib_msun_src::b0060 */
