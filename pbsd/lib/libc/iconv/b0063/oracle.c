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
 * Batch b0063 oracle: the original HardenedBSD sources concatenated, with each
 * function renamed with a ref_ prefix.  Function bodies are unmodified.
 *
 *	lib/libc/iconv/iconv_canonicalize.c
 *	lib/libc/iconv/__iconv_get_list.c
 *	lib/libc/iconv/iconvlist.c
 *	lib/libc/iconv/iconv_set_relocation_prefix.c
 *
 * The batch sources include <sys/types.h>, <iconv.h> and "iconv-internal.h".
 * Those headers supply size_t, __iconv_bool and the __bsd_* prototypes; the
 * equivalents are provided below so that this file is self contained.
 */

#include <stddef.h>

/*
 * <iconv.h>: __iconv_bool is _Bool for C99 and later.
 */
typedef _Bool __iconv_bool;

/*
 * "iconv-internal.h": prototypes for the renamed citrus entry points.
 */
const char *__bsd_iconv_canonicalize(const char *);
int __bsd___iconv_get_list(char ***, size_t *, __iconv_bool);
void __bsd_iconvlist(int (*)(unsigned int, const char *const *, void *), void *);
void __bsd_iconv_set_relocation_prefix(const char *, const char *);

/* lib/libc/iconv/iconv_canonicalize.c */
const char *
ref_iconv_canonicalize(const char *a)
{
	return __bsd_iconv_canonicalize(a);
}

/* lib/libc/iconv/__iconv_get_list.c */
int
ref___iconv_get_list(char ***a, size_t *b, __iconv_bool c)
{
	return __bsd___iconv_get_list(a, b, c);
}

/* lib/libc/iconv/iconvlist.c */
void
ref_iconvlist(int (*a) (unsigned int, const char * const *, void *), void *b)
{
	return __bsd_iconvlist(a, b);
}

/* lib/libc/iconv/iconv_set_relocation_prefix.c */
void
ref_iconv_set_relocation_prefix(const char *a, const char *b)
{
	return __bsd_iconv_set_relocation_prefix(a, b);
}
