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
 * Reference oracle for batch b0062.  The sources below are
 *	lib/libc/iconv/iconv_close.c
 *	lib/libc/iconv/iconvctl.c
 *	lib/libc/iconv/__iconv_free_list.c
 *	lib/libc/iconv/iconv_open.c
 * concatenated with a ref_ prefix on each function name.  Function bodies are
 * unmodified.  <iconv.h> and "iconv-internal.h" are not available here, so the
 * iconv_t typedef and the __bsd_* prototypes they would supply are provided
 * below; nothing else is added.
 */

#include <sys/types.h>
#include <stddef.h>

typedef void *iconv_t;

extern int __bsd_iconv_close(iconv_t);
extern int __bsd_iconvctl(iconv_t, int, void *);
extern void __bsd___iconv_free_list(char **, size_t);
extern iconv_t __bsd_iconv_open(const char *, const char *);

/* lib/libc/iconv/iconv_close.c */
int
ref_iconv_close(iconv_t a)
{
	return __bsd_iconv_close(a);
}

/* lib/libc/iconv/iconvctl.c */
int
ref_iconvctl(iconv_t a, int b, void *c)
{
	return __bsd_iconvctl(a, b, c);
}

/* lib/libc/iconv/__iconv_free_list.c */
void
ref___iconv_free_list(char **a, size_t b)
{
	__bsd___iconv_free_list(a, b);
}

/* lib/libc/iconv/iconv_open.c */
iconv_t
ref_iconv_open(const char *a, const char *b)
{
	return __bsd_iconv_open(a, b);
}
