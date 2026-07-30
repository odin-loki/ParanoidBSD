/* $NetBSD: fpgetsticky.c,v 1.3 2008/04/28 20:23:00 martin Exp $ */
/* $NetBSD: fpgetround.c,v 1.3 2008/04/28 20:23:00 martin Exp $ */
/* $NetBSD: fpsetmask.c,v 1.4 2008/04/28 20:23:00 martin Exp $ */
/* $NetBSD: fpsetround.c,v 1.3 2008/04/28 20:23:00 martin Exp $ */

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

export module pbsd.lib.libc.softfloat.b0052;

export namespace pbsd::lib_libc_softfloat::b0052 {

/*
 * milieu.h / processors template: int8 is the fast signed integer type, which
 * for this port (as for the FreeBSD softfloat build) is a plain int.
 */
using int8 = int;

/*
 * <ieeefp.h>: fp_except and fp_rnd_t.  Both are int-width on the softfloat
 * targets, and the softfloat state they alias is declared int, so the whole
 * int range round-trips through these interfaces unchanged.
 */
using fp_except = int;
using fp_rnd_t = int;

/*
 * softfloat.h: the software rounding mode and the exception flag values.
 */
enum {
    float_round_nearest_even = 0,
    float_round_to_zero      = 1,
    float_round_down         = 2,
    float_round_up           = 3
};

enum {
    float_flag_inexact   =  1,
    float_flag_underflow =  2,
    float_flag_overflow  =  4,
    float_flag_divbyzero =  8,
    float_flag_invalid   = 16
};

/*
 * The softfloat rounding mode and exception state these accessors read and
 * write.  float_rounding_mode and float_exception_flags come from
 * bits64/softfloat.c, float_exception_mask from softfloat-specialize.
 */
int8 float_rounding_mode = float_round_nearest_even;
int8 float_exception_flags = 0;
int float_exception_mask = 0;

fp_except
fpgetsticky(void)
{
	return float_exception_flags;
}

fp_rnd_t
fpgetround(void)
{

	return float_rounding_mode;
}

fp_except
fpsetmask(fp_except mask)
{
	fp_except old;

	old = float_exception_mask;
	float_exception_mask = mask;
	return old;
}

fp_rnd_t
fpsetround(fp_rnd_t rnd_dir)
{
	fp_rnd_t old;

	old = float_rounding_mode;
	float_rounding_mode = rnd_dir;
	return old;
}

} /* namespace pbsd::lib_libc_softfloat::b0052 */
