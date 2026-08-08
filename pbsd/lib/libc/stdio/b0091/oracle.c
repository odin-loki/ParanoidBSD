/*
 * b0091 oracle -- the specification.
 *
 * The original HardenedBSD sources
 *
 *	lib/libc/stdio/fsetpos.c
 *	lib/libc/stdio/getwchar.c
 *	lib/libc/stdio/putwchar.c
 *
 * concatenated, with every function renamed with a `ref_' prefix (call sites
 * of renamed functions are renamed with it too).  No function body has been
 * modified in any other way.
 *
 * Everything above the first copyright banner is host compatibility glue for
 * declarations FreeBSD's private libc headers supply and glibc does not:
 *
 *   hbsd_fpos_t	On FreeBSD fpos_t is __off_t, a scalar, which is why the
 *			original `(off_t)*pos' cast compiles there.  glibc's
 *			fpos_t is an opaque struct, so the FreeBSD scalar type
 *			is spelled out here instead.
 *   __get_locale()	FreeBSD's xlocale_private.h: the caller's current locale
 *   FIX_LOCALE()	FreeBSD's xlocale_private.h, verbatim.
 *   fgetwc_l()		FreeBSD's locale-explicit stdio primitives.  glibc has
 *   fputwc_l()		no _l stdio, so they are expressed with uselocale(),
 *			which is what makes the passed locale the one used for
 *			the conversion -- the same contract as FreeBSD's.
 *
 * port.cppm carries a character-for-character equivalent of this glue, so the
 * differential test compares the ported bodies and nothing else.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <sys/types.h>
#include <wchar.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

typedef off_t hbsd_fpos_t;

static locale_t
__get_locale(void)
{
	return (uselocale((locale_t)0));
}

#define FIX_LOCALE(loc) if (loc == NULL) loc = __get_locale()

static wint_t
fgetwc_l(FILE *fp, locale_t locale)
{
	locale_t old;
	wint_t r;
	int e;

	FIX_LOCALE(locale);
	old = uselocale(locale);
	r = fgetwc(fp);
	e = errno;
	uselocale(old);
	errno = e;
	return (r);
}

static wint_t
fputwc_l(wchar_t wc, FILE *fp, locale_t locale)
{
	locale_t old;
	wint_t r;
	int e;

	FIX_LOCALE(locale);
	old = uselocale(locale);
	r = fputwc(wc, fp);
	e = errno;
	uselocale(old);
	errno = e;
	return (r);
}

/* ------------------------------------------------------------------ */
/* lib/libc/stdio/fsetpos.c						*/
/* ------------------------------------------------------------------ */

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

/*
 * fsetpos: like fseek.
 */
int
ref_fsetpos(FILE *iop, const hbsd_fpos_t *pos)
{
	return (fseeko(iop, (off_t)*pos, SEEK_SET));
}

/* ------------------------------------------------------------------ */
/* lib/libc/stdio/getwchar.c						*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins.
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

#undef getwchar

/*
 * Synonym for fgetwc(stdin).
 */
wint_t
ref_getwchar(void)
{
	return (fgetwc(stdin));
}
wint_t
ref_getwchar_l(locale_t locale)
{
	return (fgetwc_l(stdin, locale));
}

/* ------------------------------------------------------------------ */
/* lib/libc/stdio/putwchar.c						*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins.
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

#undef putwchar

/*
 * Synonym for fputwc(wc, stdout).
 */
wint_t
ref_putwchar_l(wchar_t wc, locale_t locale)
{
	FIX_LOCALE(locale);
	return (fputwc_l(wc, stdout, locale));
}
wint_t
ref_putwchar(wchar_t wc)
{
	return ref_putwchar_l(wc, __get_locale());
}
