/* $NetBSD: fpsetsticky.c,v 1.3 2008/04/28 20:23:00 martin Exp $ */

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

/*
 * Reference oracle for PBSD batch b0339.  The function body below is copied
 * verbatim from hbsd/src/lib/libc/softfloat/fpsetsticky.c with only the
 * function name given a ref_ prefix.  The declarations the original picks up
 * from namespace.h, <ieeefp.h>, milieu.h and softfloat.h are supplied here
 * instead; the softfloat state keeps its original name so that the body is
 * untouched.  The __weak_alias() block is build glue with no run-time
 * behaviour and is not reproduced.
 */

/* milieu.h / processors template. */
typedef int int8;

/* <ieeefp.h> */
typedef int fp_except;

/* softfloat.h */
enum {
    float_flag_inexact   =  1,
    float_flag_underflow =  2,
    float_flag_overflow  =  4,
    float_flag_divbyzero =  8,
    float_flag_invalid   = 16
};

/* bits64/softfloat.c */
int8 float_exception_flags = 0;

fp_except
ref_fpsetsticky(fp_except except)
{
	fp_except old;

	old = float_exception_flags;
	float_exception_flags = except;
	return old;
}
