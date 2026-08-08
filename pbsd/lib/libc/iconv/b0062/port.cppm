/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2013 Peter Wemm
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
 * Batch b0062, ported from (all four carry the identical notice above):
 *	lib/libc/iconv/iconv_close.c
 *	lib/libc/iconv/iconvctl.c
 *	lib/libc/iconv/__iconv_free_list.c
 *	lib/libc/iconv/iconv_open.c
 */

module;

#include <sys/types.h>

export module pbsd.lib.libc.iconv.b0062;

/*
 * <iconv.h> and "iconv-internal.h" are not part of this batch; iconv_t is
 * void * and the __bsd_* entry points are declared here verbatim.
 */
extern "C" {
int __bsd_iconv_close(void *);
int __bsd_iconvctl(void *, int, void *);
void __bsd___iconv_free_list(char **, size_t);
void *__bsd_iconv_open(const char *, const char *);
}

export namespace pbsd::lib_libc_iconv::b0062 {

using iconv_t = void *;

int
iconv_close(iconv_t a)
{
	return __bsd_iconv_close(a);
}

int
iconvctl(iconv_t a, int b, void *c)
{
	return __bsd_iconvctl(a, b, c);
}

void
__iconv_free_list(char **a, size_t b)
{
	__bsd___iconv_free_list(a, b);
}

iconv_t
iconv_open(const char *a, const char *b)
{
	return __bsd_iconv_open(a, b);
}

} // namespace pbsd::lib_libc_iconv::b0062
