/*
 * Reference oracle for batch b0117s2.
 *
 * The original HardenedBSD sources are concatenated below with every function
 * renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.  Only the
 * macro definitions and external declarations that the host environment does
 * not provide have been added.
 */

#define _GNU_SOURCE

#include <limits.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef __RCSID
#define __RCSID(s)
#endif

static jmp_buf chk_fail_jmp;
static volatile int chk_fail_happened;

int
oracle_chk_fail_setjmp(void)
{
	chk_fail_happened = 0;
	return (setjmp(chk_fail_jmp));
}

int
oracle_chk_fail_happened(void)
{
	return (chk_fail_happened);
}

void
__chk_fail(void)
{
	chk_fail_happened = 1;
	longjmp(chk_fail_jmp, 1);
}

static inline int
__ssp_overlap(const void *leftp, const void *rightp, size_t sz)
{
	uintptr_t left = (uintptr_t)leftp;
	uintptr_t right = (uintptr_t)rightp;

	if (left <= right)
		return (SIZE_MAX - sz < left || right < left + sz);

	return (SIZE_MAX - sz < right || left < right + sz);
}

/*-
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2006 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
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
__RCSID("$NetBSD: strcpy_chk.c,v 1.8 2015/05/09 15:42:21 christos Exp $");

#undef memcpy

char *
ref___strcpy_chk(char * __restrict dst, const char * __restrict src, size_t slen)
{
	size_t len = strlen(src) + 1;

	if (len > slen)
		__chk_fail();

	if (__ssp_overlap(src, dst, len))
		__chk_fail();

	return (memcpy(dst, src, len));
}
