/*
 * b0100 oracle -- the specification.
 *
 * hbsd/src/lib/libc/stdio/wprintf.c, clrerr.c, and feof.c concatenated,
 * with every function renamed with a `ref_' prefix.  Function bodies are
 * UNMODIFIED.  Only defines/declarations that the unavailable FreeBSD/
 * HardenedBSD headers used to supply have been added.
 */

#include <stddef.h>
#include <limits.h>
#include <stdarg.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

struct _IO_FILE;
extern struct _IO_FILE *stdout;
int vfwprintf(struct _IO_FILE * __restrict, const wchar_t * __restrict,
    va_list);

struct __locale_struct;
typedef struct __locale_struct *locale_t;
#define LC_GLOBAL_LOCALE ((locale_t)0)

int
vfwprintf_l(struct _IO_FILE * __restrict fp, locale_t locale,
    const wchar_t * __restrict fmt, va_list ap)
{
	(void)locale;
	return vfwprintf(fp, fmt, ap);
}

/* ======================= wprintf.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins
 * All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
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

int
ref_wprintf(const wchar_t * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfwprintf(stdout, fmt, ap);
	va_end(ap);

	return (ret);
}
int
ref_wprintf_l(locale_t locale, const wchar_t * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfwprintf_l(stdout, locale, fmt, ap);
	va_end(ap);

	return (ret);
}

#ifndef _STDFILE_DECLARED
#define _STDFILE_DECLARED
struct __sFILE {
	unsigned char	*_p;
	int		_r;
	int		_w;
	short		_flags;
	short		_file;
};
typedef struct __sFILE FILE;
#endif

#define	__SEOF	0x0020
#define	__SERR	0x0040
#define	__sfeof(p)	(((p)->_flags & __SEOF) != 0)
#define	__sclearerr(p)	((void)((p)->_flags &= ~(__SERR|__SEOF)))

int	__isthreaded;

void
_flockfile(void *fp)
{
	(void)fp;
}

void
_funlockfile(void *fp)
{
	(void)fp;
}

#define	FLOCKFILE(fp)		if (__isthreaded) _flockfile(fp)
#define	FUNLOCKFILE(fp)		if (__isthreaded) _funlockfile(fp)

/* ======================= clrerr.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* #include "namespace.h"		-- unavailable, substituted above */
/* #include <stdio.h>			-- unavailable, substituted above */
/* #include "un-namespace.h"		-- unavailable, substituted above */
/* #include "libc_private.h"		-- unavailable, substituted above */

#undef clearerr
#undef clearerr_unlocked

void
ref_clearerr(FILE *fp)
{
	FLOCKFILE(fp);
	__sclearerr(fp);
	FUNLOCKFILE(fp);
}

void
ref_clearerr_unlocked(FILE *fp)
{

	__sclearerr(fp);
}

/* ======================= feof.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* #include "namespace.h"		-- unavailable, substituted above */
/* #include <stdio.h>			-- unavailable, substituted above */
/* #include "un-namespace.h"		-- unavailable, substituted above */
/* #include "libc_private.h"		-- unavailable, substituted above */

#undef feof
#undef feof_unlocked

int
ref_feof(FILE *fp)
{
	int	ret;

	FLOCKFILE(fp);
	ret= __sfeof(fp);
	FUNLOCKFILE(fp);
	return (ret);
}

int
ref_feof_unlocked(FILE *fp)
{

	return (__sfeof(fp));
}
