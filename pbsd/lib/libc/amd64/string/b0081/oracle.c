/*
 * oracle.c -- reference (specification) build for PBSD batch b0081.
 *
 * The original HardenedBSD sources
 *
 *	hbsd/src/lib/libc/amd64/string/strcpy.c
 *	hbsd/src/lib/libc/amd64/string/strnlen.c
 *	hbsd/src/lib/libc/amd64/string/strpbrk.c
 *	hbsd/src/lib/libc/amd64/string/strncpy.c
 *
 * concatenated verbatim, with each exported function renamed with a "ref_"
 * prefix.  No function body has been modified.
 */

#define _GNU_SOURCE 1

#include <stddef.h>
#include <string.h>
#include <sys/cdefs.h>

/*
 * The wrappers below delegate the actual work to the hand written amd64
 * assembly routines __stpcpy(), __stpncpy(), __memchr() and __strcspn(),
 * which are not part of this batch.  Bind those names to the equivalent
 * libc entry points, which implement the same contract, so that the
 * unmodified bodies below can be linked and executed.
 */
char *
ref_h_stpcpy(char *__restrict to, const char *__restrict from)
{
	return (stpcpy(to, from));
}

char *
ref_h_stpncpy(char *__restrict dst, const char *__restrict src, size_t len)
{
	return (stpncpy(dst, src, len));
}

char *
ref_h_memchr(const void *s, int c, size_t n)
{
	return ((char *)memchr(s, c, n));
}

size_t
ref_h_strcspn(const char *s, const char *charset)
{
	return (strcspn(s, charset));
}

#define __stpcpy	ref_h_stpcpy
#define __stpncpy	ref_h_stpncpy
#define __memchr	ref_h_memchr
#define __strcspn	ref_h_strcspn

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/strcpy.c				*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2011 George V. Neville-Neil. All rights reserved.
 *
 * The compilation of software known as FreeBSD is distributed under the
 * following terms:
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

char *__stpcpy(char * __restrict, const char * __restrict);

char *
ref_strcpy(char * __restrict to, const char * __restrict from)
{
	__stpcpy(to, from);
	return(to);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/strnlen.c				*/
/* ------------------------------------------------------------------ */

/*-
 * Copyright (c) 2023 The FreeBSD Foundation
 *
 * This software was developed by Robert Clausecker <fuz@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
 */

#include <string.h>

char *__memchr(const void *, int, size_t);

size_t
ref_strnlen(const char *s, size_t maxlen)
{
	const char *loc;

	loc = __memchr(s, '\0', maxlen);

	return (loc == NULL ? maxlen : (size_t)(loc - s));
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/strpbrk.c				*/
/* ------------------------------------------------------------------ */

/*-
 * Copyright (c) 2023 The FreeBSD Foundation
 *
 * This software was developed by Robert Clausecker <fuz@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
 */

#include <sys/cdefs.h>

#include <string.h>

size_t __strcspn(const char *, const char *);

char *
ref_strpbrk(const char *s, const char *charset)
{
	size_t loc;

	loc = __strcspn(s, charset);

	return (s[loc] == '\0' ? NULL : (char *)&s[loc]);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/strncpy.c				*/
/* ------------------------------------------------------------------ */

/*-
 * Copyright (c) 2023 The FreeBSD Foundation
 *
 * This software was developed by Robert Clausecker <fuz@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
 */

#include <sys/cdefs.h>
#include <string.h>

#undef strncpy	/* _FORTIFY_SOURCE */

char *__stpncpy(char *restrict, const char *restrict, size_t);

char *
ref_strncpy(char *restrict dst, const char *restrict src, size_t len)
{

	__stpncpy(dst, src, len);

	return (dst);
}
